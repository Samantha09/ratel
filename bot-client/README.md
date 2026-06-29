# ratel LLM Agent

外部机器人,替代 C++ gateway 内部机器人。负责与 gateway 的 WebSocket 通信、
叫地主(启发式),**出牌决策默认委托给 Python [`dou-dizhu-agent`](../dou-dizhu-agent)
的 `POST /play`**(本进程做 WebSocket 壳 + 牌格式转换 + 失败兜底)。

## 架构总览

```
C++ gateway ──WS──> bot-client (TS 壳)  ──HTTP POST /play──> dou-dizhu-agent (Python)
                     连接/状态机/叫分/兜底                   牌型/候选/LLM 决策
```

- 出牌:`decidePlay` 把手牌/上家牌转成字符牌发给 Python `/play`,把返回的字符牌
  按 level 映射回真实 `{type,level}` 手牌并复核合法性;**调用失败/超时/非法一律
  回退本地规则牌**,机器人不会卡死。
- 叫地主:始终用本地启发式(Python 无叫分接口)。
- 留空 `PLAY_AGENT_URL` 则回退到本进程内置 LLM(MiniMax 等)出牌。

## 运行

```bash
cp bot-client/.env.example bot-client/.env.local   # 按需编辑

# 1) gateway(仓库根目录)
./gateway 127.0.0.1 8787

# 2) Python 出牌 agent(出牌大脑,默认 :8000)
cd dou-dizhu-agent && uv sync && uv run python -m dou_dizhu_agent
#   MiniMax Key 现在配在 Python 侧:export LLM_API_KEY=... (见其 README)

# 3) TS 壳(连接 gateway 的 N 个机器人)
cd bot-client && npm run dev
```

## 测试

```bash
# 单元测试（mock provider，不调用真实 API / 不连 Python）
npm test

# 端到端冒烟测试（mock provider）
npm run smoke
```

## 环境变量

| 变量 | 说明 | 默认值 |
|---|---|---|
| `GATEWAY_URL` | gateway WebSocket 地址 | `ws://127.0.0.1:8787` |
| `PLAY_AGENT_URL` | Python 出牌 agent 基地址;留空=用内置 LLM | `http://127.0.0.1:8000` |
| `LLM_PROVIDER` | 内置 LLM:`minimax` / `anthropic` / `openai` / `ollama` / `mock` | `minimax` |
| `MINIMAX_API_KEY` | MiniMax API Key(仅内置 LLM 用) | - |
| `MINIMAX_BASE_URL` | MiniMax OpenAI-compatible 端点 | `https://api.minimaxi.com/v1` |
| `MINIMAX_MODEL` | 通用模型名 | `MiniMax-M2.7` |
| `PLAY_MODEL` | 出牌专用模型(高速变体) | `MiniMax-M2.7-highspeed` |
| `PLAY_TIMEOUT_MS` | 出牌硬超时(毫秒),超时回退规则牌(对 play-agent 与内置 LLM 都生效) | `20000` |
| `LLM_TEMPERATURE` | 采样温度 | `0.3` |
| `AGENT_COUNT` | 机器人数量 | `2` |
| `AGENT_NICKNAME_PREFIX` | 机器人昵称前缀 | `robot` |

## 文档

- [出牌速度优化](docs/play-speed-optimization.md):机器人卡死/高延迟的根因分析与分层决策管线设计。

## 架构

- `src/rules.ts`：斗地主牌型识别、大小比较、合法出牌枚举。
- `src/play-agent.ts`：HTTP 桥接到 Python `dou-dizhu-agent`,含 `{type,level}`↔字符牌双向转换。
- `src/prompt.ts`：生成给内置 LLM 的 prompt。
- `src/decision.ts`：分层出牌决策(0个选项→pass / 唯一牌→直接出 / play-agent → 内置 LLM → 规则兜底)。
- `src/game-client.ts`：WebSocket 连接与事件收发。
- `src/agent.ts`：单个 robot 的状态机。
- `src/main.ts`：入口,按配置启动 N 个 agent。
- `scripts/smoke.ts`：端到端冒烟测试。
