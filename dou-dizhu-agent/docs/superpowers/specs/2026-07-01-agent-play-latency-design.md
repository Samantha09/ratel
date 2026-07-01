# Agent 出牌延迟优化设计:架构翻转为「直接出牌」

> 日期:2026-07-01
> 范围:`dou-dizhu-agent`(Python 出牌大脑)。bot-client / gateway / WS 合同不动。
> 目标:在**不降低出牌智力(仍用 M2.7)**的前提下,把单回合出牌延迟从 20–88s 大幅压低,并修复复杂回合的「截断→回退最笨规则牌」bug。

---

## 1. 问题与根因

链路:`gateway(C++) ──WS──> bot-client(TS) ──HTTP /play──> dou-dizhu-agent(Python) ──HTTP──> MiniMax M2.7`。单回合 20–88s,一局卡几分钟。

根因(实测确认,见 §2):

1. **M2.7 的 thinking 既关不掉也卡不长**。`thinking:{type:"disabled"}` 与 `thinking:{type:"enabled", budget_tokens:512}` 均被忽略,思考量 ~3500 token 不变(社区已知:这类推理模型的思考是内建行为)。
2. **现架构把全部合法候选塞进 prompt 让 LLM「选 index」**。这个「从列表里挑」的框架诱发模型逐项深度比较 → 思考爆 `max_tokens=4096` → **返回空 text → 静默回退到 `_default_fallback`**(出最小非炸弹)。
3. 领出回合候选数巨大(农民中位数 37、地主中位数 59,最多 462),正是这些回合触发截断。**即:最复杂的回合,机器人现在干等 ~88s,最后出的还是最笨的规则牌——慢且笨。**

## 2. 实测依据(2026-07-01,`MiniMax-M2.7-highspeed` / `anthropic` 端点)

### 2.1 thinking 控制(领出,31 候选,max_tokens 4096)

| 方案 | 耗时 | thinking | 答案 |
|---|---|---|---|
| baseline(不传 thinking) | 81s | ~3561 tok | 空(截断) |
| `disabled` | 72s | ~3611 tok | 空(截断) |
| `budget_tokens:512` | 79s | ~3679 tok | 空(截断) |

→ 关闭/卡短思考**均无效**;且 31 候选在 4096 下必然截断。

### 2.2 prompt 形式对比(领出,max_tokens 4096)

| 方案 | 耗时 | thinking | 答案 | 合法 |
|---|---|---|---|---|
| A 候选列表 31(现状) | 88s | ~3405 tok | 空(截断) | — |
| B 候选剪枝到 7 | 88s | ~3648 tok | 空(截断) | — |
| C **牌名直接给 + 直接出牌** | **11–15s** | **~430–565 tok** | `4 4 4` | ✓ |
| D 计数向量(bit)输入 | 72s | ~3084 tok | 空(截断) | 非法 |

→ 剪枝无效(框架没变);bit 向量更糟(解码负担);**牌名直接给 6 倍提速且合法**。

### 2.3 跟牌合法性(牌名直接给,max_tokens 4096)

| 场景 | 耗时 | thinking | 答案 | 合法性 |
|---|---|---|---|---|
| 管对子 `8 8` | 10.6s | ~424 tok | `9 9` | ✓ 在候选集内 |
| 管顺子 `6 7 8 9 10` | 30.1s | ~1150 tok | `7 8 9 10 J` | ✓ 在候选集内 |

→ 跟牌也能正常出合法答案;硬跟牌(管顺子)较慢(~30s)但仍远好于现状。

### 2.4 候选数分布(3000 手本地统计)

领出:农民 17 张中位数 37(p90 56,max 191);地主 20 张中位数 59(max 462),76% > 50。
跟牌:管对子中位数 3、管单张 5、管顺子 1(65% 只能过)、管三张 1(82% 只能过)。

## 3. 核心原则

> 任何让模型「多花力气解释输入」的东西(候选枚举比较、计数向量解码)都会引爆思考 → 截断。
> 让模型「直接看见、直接决定」的最小自然语言 prompt,思考最短、答得对。**少即是多。**

## 4. 设计:架构翻转

把「生成候选 → LLM 选 index」翻转为「给手牌+局面 → LLM 直接出牌 → 成员校验 → 不合法回退」。

```
POST /play (hand, last_play, role, counts, bottom, history)
  │
  ├─ candidates = generate_candidates(hand, last_play)    # 本地,快;仅用于校验+兜底
  ├─ raw = strategy.choose(hand, role, last_play, ...)     # LLM 直接出牌,不传候选列表
  ├─ cards = 解析 raw 中的牌 token
  └─ 校验(见 §6):
       合法 → return ("play", cards)      # 用 LLM 的牌
       空/非法 → return _default_fallback  # 规则兜底(永不出错牌、永不卡)
```

`generate_candidates` **不删**,只是从「喂给 LLM 的输入」降级为「合法性校验器 + 兜底数据源」。合法性强度与现状等价:采纳的牌必在合法候选集内。

## 5. 改动范围(均在 `dou-dizhu-agent`)

**bot-client / gateway / WS 合同完全不动**:bot-client 只认 `/play` 的 `{action, cards:string[]}`,rank↔card 映射已具备。

### `strategy.py`
- 新 prompt(自然语言,见 §7):角色、手牌(牌名)、上家牌(或「你领出」)、对手剩余张数、剩余牌摘要。**去掉冗余的底牌**(其信息已被剩余牌摘要隐含)。
- `choose` 返回**牌字符串**(或 `"pass"`),不再返回 index;`_parse_choice` 退役。
- `MinimaxAnthropicStrategy` 保持 httpx 直连 `/anthropic`,只取 `type=="text"` 块(忽略 thinking 块),逻辑不变。

### `api.py` `/play`
- 仍调 `generate_candidates`(降级为校验器+兜底)。
- 把 `select(candidates, choice)` 替换为「解析 + 成员校验」(见 §6)。

### `selector.py`
- 保留 `_default_fallback` 作为唯一兜底路径。`select` / `select_from_request` 不再被 `/play` 调用,可删或保留待用。

## 6. 合法性校验规则

设 `cand_sets = { tuple(sorted(c)) for c in candidates if c }`,`last_play` 是否为空决定能否过牌:

```
if raw 解析为 "pass":
    return ("pass", []) if last_play 非空 else _default_fallback(candidates)   # 领出不能过
cards = [t for t in raw.split() if t in VALID_CARDS]
if cards and tuple(sorted(cards)) in cand_sets:
    return ("play", cards)            # 合法 → 采纳 LLM 决策
return _default_fallback(candidates)  # 空/非法 → 规则兜底
```

- 跟牌时「pass」始终合法;领出时「pass」→ 兜底出牌。
- LLM 输出的牌必须在候选集内才采纳;否则兜底。**最坏情况退回规则牌,永不出错牌、永不卡。**
- 解析容错:取空白分隔、属 `VALID_CARDS` 的 token(`10` 作为整体);乱序/带引号/多余文字不影响抽取;无法抽出有效牌 → 兜底。

## 7. Prompt(自然语言,最小化)

system:
```
You are a dou-dizhu player. Output ONLY the cards you play, space-separated
(e.g. "9 9" or "6 7 8 9 10"), or the word "pass". If you cannot or should not
beat the last play, output "pass". No explanation.
```

user(跟牌样例):
```
Role: peasant
My hand (13 cards): 9 9 10 J Q Q K K A 2 2 2 小王
Last play by opponent: 8 8   (beat with same pattern, or bomb/rocket, or pass)
Opponent card counts: {'robot_2': 6, '玩家': 9}
Remaining cards summary: <CardCounter.summary()>
Output ONLY the cards you play (or 'pass').
```

领出时 `Last play` 行改为 `You are first to play (lead).`。**不再列出候选清单,也不再列底牌。**

## 8. 参数

- `max_tokens`:维持 **4096**。实测直接出牌输出 token:领出 ~500、管对子 ~530、管顺子 ~1350。4096 给足余量,避免硬跟牌截断。(后续可按观测 p99 收紧到 ~3072。)
- `temperature` 0.3、`LLM_TIMEOUT` 90s 维持;不传 `thinking` 字段(传了也被忽略)。
- bot-client 侧 `PLAY_TIMEOUT_MS=100000` 维持(>Python 侧 90s)。

## 9. 残余风险与缓解

- **跟牌延迟方差大**:管顺子等硬决策 ~30s。可接受(远好于现状 88s+兜底),且校验保证最坏退规则牌。**后续叠加预取(prefetch)把这段延迟藏到对手思考时间里**——本场景另一对手也是 20–40s 机器人,预取命中率高。
- **LLM 偶发输出非法/空(含截断)**:成员校验 → 规则兜底,安全降级。
- **出牌质量**:样本出牌合理(领出 triple、管对子给最小能管 `9 9`、管顺子给最低顺子 `7 8 9 10 J`)。质量靠 M2.7 本身;不满意可微调温度/提示词。

## 10. 不在本次范围(YAGNI)

- **预取(prefetch)**:下一阶段,叠加在本成果上进一步压感知延迟;需顺手把 `api.py` 同步 `httpx.post` 改异步。
- **换非推理模型**:用户要求保智力,保留 M2.7;换模型作为可选 env 留后。
- **候选剪枝 / bit 向量**:实测无效或反效,明确不做。

## 11. 测试策略

- **单元**:§6 校验规则各路径(合法牌/非法牌/空/pass/领出 pass→兜底);`_default_fallback` 行为保持不变。
- **回归**:`test_strategy.py` / `test_api.py` 用 `MockStrategy` 改造为「返回牌字符串」;`/play` 端到端用 mock 验证采纳与兜底分支。
- **实证**:上线后抽样日志,确认 (a) 截断/回退率、(b) 各回合延迟分布、(c) 出牌是否被 gateway 判非法(`playError`)。
