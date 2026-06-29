# 前端原型对齐(prototype parity)— 设计文档

- 日期:2026-06-29
- 范围:`web/`(纯前端,不改动 C++ gateway)
- 参考原型:`index(1).html`(PlayStation 风格斗地主 demo)
- 方案:**A — 增量扩展现有架构**

## 1. 背景与现状

`index(1).html` 是一个**自包含**的斗地主 demo:自带本地游戏引擎(发牌、牌型判定、本地 AI、mock 房间)与一套"PlayStation"视觉设计。

现有 `web/` 是 React 18 + TS + Vite + Tailwind 的真实客户端,已通过 WebSocket 接入 gateway(`useGame` / `gameReducer` / `socket`)。

**关键事实:该原型的视觉与结构已被移植约 90%。** `styles/index.css` 顶部注释写明 *"Ported from the index(1) prototype"*;设计 token、class 名(`.topbar` / `.seat` / `.card` / `.mini` / `.bottom-strip` / `.room-card` …)、`GameView` / `CardTable` / `PlayingCard` 的 DOM 结构均与原型一致。git 中未提交的 `M web/...` 即本次移植的进行中改动。

本设计的目标:**补齐原型有、web 仍缺的差距**,使 web 在视觉与交互上与原型一致。

## 2. 范围与边界

| 能力 | 数据来源 | 说明 |
|---|---|---|
| 底牌揭示条(3 张底牌,所有人可见) | 真实 | `landlordConfirm.additionalPokers` 对房内所有 PLAYER 广播 |
| 三列出牌区(每座各显示最近一手) | 真实 | `showPokers.clientId` 区分出牌者 |
| TopBar 房间号 | 真实 | `state.roomId` |
| 倍数(炸弹/王炸 ×2) | 客户端计算 | gateway 出牌事件不带 sellType,本地评估器判定 |
| 底分(=3) | 常量 | gateway 合同无底分 |
| 局数 | 客户端计数 | "再来一局"时 +1 |
| 可用"提示"按钮(循环合法牌) | 客户端引擎 | 移植原型的 evaluate/beats/legalPlays |
| 玩家对战:房间列表 / 等待室 | **mock** | 对齐原型,落回 `createRoomPve`(AI 补位) |

**不做**(超出"客户端兜底"约定):
- 真实 PVP 房间创建/加入(需扩展 `JsonMapHelper.h` 的 ingress/egress 映射 + 重建 gateway)。
- 改变后端叫分语义(gateway 为 grab 布尔,原型 0/1/2/3 分仅作视觉装饰,保持现状)。
- 任何 C++ / gateway / agent 侧改动。

## 3. 状态层设计(`types.ts` + `state/gameReducer.ts`)

### 3.1 `GameState` 新增字段

```ts
bottomCards: Card[];                                       // 3 张底牌(所有人)
multiplier: number;                                        // 倍数,开局 1
baseScore: number;                                         // 常量 3
round: number;                                             // 局数,首局 1
playsBySeat: Record<number, { pokers: Card[]; passed: boolean }>; // 每座最近一手
lobbyScreen: 'nickname' | 'menu' | 'rooms' | 'waiting';    // 大厅子屏
```

`initialState` 补默认值:`bottomCards: []`、`multiplier: 1`、`baseScore: 3`、`round: 0`、`playsBySeat: {}`、`lobbyScreen: 'nickname'`。

### 3.2 事件处理调整

- `connected`:`lobbyScreen` 维持 `'nickname'`(除非 `nickname` 非空则 `'menu'`)。
- `gameStarting`:重置 `bottomCards=[]`、`multiplier=1`、`playsBySeat={}`;`round` 仅在"从 `over` 复盘重开"路径上 +1(见 §7);首局由 §7 的再来一局逻辑保证为 1。
- `landlordConfirm`:**所有人**存 `bottomCards = e.data.additionalPokers ?? []`;`multiplier=1`;地主仍把底牌加入 `hand`(现有逻辑保留)。
- `showPokers`:`playsBySeat[clientId] = { pokers, passed:false }`;用 `pokerEngine.evaluate` 判定,若为 BOMB/KING_BOMB 则 `multiplier *= 2` 并触发 toast("炸弹!倍数 ×2")。其余(hand/seats/lastSell)逻辑不变。
- `playPass`:`playsBySeat[clientId] = { pokers:[], passed:true }`。
- 新增 `Action`:`{ type:'gotoLobby'; screen: LobbyScreen }`、`{ type:'bumpRound' }`(供 §7 再来一局)。
- `reset`:不再回到 `initialState`;改为保留 `nickname` / `clientId`,置 `phase='connecting'`、清盘面字段,`round` 由调用方先 `bumpRound`。

### 3.3 `types.ts`
- `GameState` 已是 web 侧类型,直接扩展(无需改 `ServerEvent`,沿用既有 egress 字段)。
- 不新增 `ClientEvent`(玩家对战 mock 落回既有 `createRoomPve`)。

## 4. 大厅流程(`views/` + 新 overlay)

`nickname → 主菜单(人机/玩家)`:
- 选**人机** → `createRoomPve` → `gameStarting`。
- 选**玩家** → 房间管理(mock 房间列表 + 4 位房号加入 + 创建)→ 等待室(座位填充动画,约 2s)→ 落回 `createRoomPve`。

新增组件(复用既有 `.panel` / `.room-card` / `.seat-slot` CSS):
- `views/MainMenu.tsx`:`overlay > panel`,两枚按钮(🤖 人机对战 / 👥 玩家对战)。
- `views/RoomManager.tsx`:mock 房间列表 + 房号输入 + 创建/返回。mock 数据生成不含随机时间源,初始固定几条。
- `views/WaitingRoom.tsx`:座位填充动画,完成回调 `onReady`。
- `views/LobbyView.tsx`:保留为昵称输入屏。

`App.tsx` 路由:`phase==='connecting'|'lobby'` 时按 `state.lobbyScreen` 分发到对应屏;`GameView` 路径不变。

## 5. 牌桌增强

- 新增 `game/BottomStrip.tsx`:`bottomCards.length>0` 时,在牌桌顶部居中渲染 3 张底牌(复用 `PlayingCard` 的 `variant="mini"`,外层 `.bottom-strip`)。
- `game/CardTable.tsx`:`play-zone` 三列改用 `playsBySeat`——左/右列取对应对手座位的最近一手(`pokers` 或"不出"气泡),中列取自己。沿用 `.played` / `.mini-row` / `.pass-bubble` / `.lead-tag`。
- `views/GameView.tsx` 的 `TopBar`:底分(`baseScore`)/ 倍数(`multiplier`)/ 局数(`round`)/ 房间(`roomId`)全部接 state,不再 hardcode。

## 6. 新增 `game/pokerEngine.ts`

从原型移植评估器,适配本项目 `Card { type: 0..4; level: 0..14 }`。

**Ordinal 映射**(消除原型 value 跳号,便于顺子判定):
```ts
// level: 0..12 = 3,4,...,J,Q,K,A,2 ; 13 = 小王 ; 14 = 大王
ordinal(level) = level <= 12 ? level + 3 : (level === 13 ? 16 : 17)
// => 3=3 ... A=14, 2=15, 小王=16, 大王=17
```

导出:
- `evaluate(cards): { type: SellTypeValue; ordinal: number; len: number } | null`
  覆盖:单、对、三、三带一、三带二、顺子(≥5,ordinal 连续且 ≤14)、连对(≥3 对)、飞机(纯/带单/带对)、炸弹、火箭(双王)。
- `beats(a, b | null)`:火箭 > 炸弹 > 同型同长更大。
- `legalPlays(hand, last | null)`:返回所有能压 `last` 的合法组合(领出时给一组选项),按弱到强排序,供"提示"循环。
- `isBomb(e)` / `isKingBomb(e)`:供倍数检测。

一处实现,同时服务 §3 的**倍数检测**与 §8 的**提示按钮**。

## 7. 结果页与"再来一局"修复

- `game/ResultOverlay.tsx`:揭示 3 张底牌(`.bottom-cards`)+ 计分公式 `底分 × 倍数 = X 分`。
- **修复现存短板**:当前"再来一局"(`reset`)会把用户打回昵称输入页。改为:
  - `onAgain` 先 `bumpRound`(round+1),再 `reset`(保留 nickname)。
  - `useGame` 在重连后检测"复盘意图",自动补发 `setNickname` + `createRoomPve`,跳过大厅直接进入下一局。

## 8. 提示按钮(`game/ActionBar.tsx` + `GameView`)
- `ActionBar.onHint` 改为真实循环:`GameView` 维护 `hintList = legalPlays(hand, lastSell)` 与索引,每次点击高亮下一组合法牌(写入 `state.selected`)。
- 移除现有"提示功能暂未开放"的装饰 toast。

## 9. 测试策略(沿用 vitest + RTL)
- `pokerEngine.test.ts`:各牌型 `evaluate`、`beats` 压牌、`legalPlays` 循环与领出。
- `gameReducer.test.ts`:`landlordConfirm` 后 `bottomCards` 全员可见且地主手牌含底牌;`showPokers` 炸弹触发 `multiplier×2`;`playsBySeat` 更新;`bumpRound`/`reset` 保留 nickname 且 round 递增。
- 组件关键用例:`TopBar` 数值随 state 变化;`BottomStrip` 显隐;三列 `play-zone` 渲染对应座位的牌/不出气泡。

## 10. 风险与回退
- **风险**:pokerEngine 牌型判定与 gateway/agent 侧存在分歧时,"提示"给出的牌仍可能被后端判 `playError`。**缓解**:提示只是辅助选择,最终以 `playError` 为准;评估器单测覆盖常见牌型。
- **风险**:`reset` 重连自动重发可能因连接时序失败。**缓解**:保留 lobby 兜底,若 `gameStarting` 未到仍可手动从大厅进入。
- **回退**:各改动按模块(pokerEngine / reducer / 牌桌 / 大厅 / 结果)独立,可分块提交/回退;视觉层只消费既有 CSS class,不动 `index.css` 设计 token。
