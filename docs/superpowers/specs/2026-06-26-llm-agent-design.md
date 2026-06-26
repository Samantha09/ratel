# LLM 斗地主 Agent 设计

- 日期：2026-06-26
- 分支：`feature/web-game`
- 状态：草案，待审阅
- 关联：`docs/superpowers/specs/2026-06-24-cpp-gateway-design.md`（C++ WebSocket Gateway 设计）

## 1. 背景与目标

当前 C++ gateway（`./gateway 127.0.0.1 8787`）在 PVE 模式中会创建两个内部机器人（`ClientRole::ROBOT`），由 `RobotEventFuncs.cc` / `RobotEventListener.cc` 驱动。该机器人逻辑存在当地主时卡死的问题，且出牌策略非常简陋（基本随机/固定 pass）。

本设计目标是**用 LLM Agent 替代原有 C++ 机器人**，让大模型通过 WebSocket 接入 gateway 并参与对局，从而：
- 解决机器人卡死问题；
- 提供更有趣、可解释的出牌/抢地主策略；
- 保持 gateway 改动最小，复用已调通的 WS+JSON 契约。

## 2. 范围

### 2.1 第一版目标

- 新增独立 TypeScript 项目 `llm-agent/`，内置 2 个 LLM robot。
- 每个 robot 以普通玩家身份连接真实 C++ gateway，完成一局 PVE 完整流程。
- 决策覆盖：抢地主（`landlordElect`）、出牌（`play`）、不出（`pass`）。
- LLM 只负责策略选择，牌型合法性由本地规则库兜底。
- 默认接入 OpenAI 兼容 API（环境变量配置），同时预留 Anthropic / Ollama 切换能力。

### 2.2 非目标（第一版不做）

- 多轮对话记忆、对局复盘、学习进化。
- LLM 自主理解完整斗地主规则并直接输出任意牌型（第一版由本地规则库枚举合法出牌）。
- 生产级并发、重连、断线恢复。
- 替换 mock server（`web/mock-server/`）中的机器人。

## 3. 现状分析

### 3.1 当前 C++ 机器人路径

- 人类创建 PVE 房间时，`EventFuncs.cc` 的 `CODE_ROOM_CREATE_PVE` 会生成两个 `ClientRole::ROBOT` 的 `ClientSide`，并让它们共享人类的 `TcpConnection`。
- 当轮到机器人时，`EventFuncs.cc` 在新线程中调用 `RobotEventListener::get`，最终进入 `RobotEventFuncs.cc`：
  - 抢地主：永远 `is_Y=false`（不抢）。
  - 出牌：从 `PokerHelper::validSells` 中随机选一组能压过上家的牌，否则 pass。
- 机器人出错时无 fallback，当地主后容易出现事件循环卡死，无法到达 `gameOver`。

### 3.2 现有可复用资产

- `web/mock-server/src/cardtype.ts` / `compare.ts` / `deck.ts`：已经用 TypeScript 实现了牌型识别、大小比较、发牌。
- `web/src/types.ts`：前端 ↔ gateway 的 JSON 事件类型。
- `web/e2e/diag-robot.ts`：已有一个驱动 gateway 的 Node 诊断脚本，可作为 agent 连接层的参考。

### 3.3 Gateway 需要的最小改动

为了让外部 agent 取代内部 robot，需要调整 PVE 房间创建逻辑。有两种可选子方案（见 §4.3），最终选择**子方案 A2**。

## 4. 方案决策

### 4.1 选定方案：外部 LLM Agent + Gateway 最小改造

新增 `llm-agent/` 项目，让 LLM robot 作为普通 WebSocket client 连入 gateway；gateway 维护一个**外部机器人注册池**，人类创建 PVE 房间时从池中取 2 个外部 robot 加入房间，不再创建内部 `ROBOT` ClientSide。

**架构原则：**
- Agent 只依赖公开 WS+JSON 契约，不依赖 C++ 内部结构。
- LLM 只做“从合法选项中选择”的策略决策，不做规则判定。
- Gateway 改动集中在 `CODE_ROOM_CREATE_PVE` 与机器人触发路径，游戏核心逻辑零改动。

### 4.2 框架选择：Vercel AI SDK

使用 `ai` + `@ai-sdk/openai`（或对应 provider 包）：
- 原生支持 `generateObject`，可强制返回固定 JSON schema。
- 支持 OpenAI、Anthropic、Ollama 等 provider，切换仅需改环境变量和 provider 包。
- 比 LangChain.js 轻量，适合单轮决策 Agent。

### 4.3 Gateway 改造子方案选择

| 子方案 | 说明 | 优劣 |
|---|---|---|
| A1 | PVE 创建房间后等待 2 个外部 agent 加入，再开始发牌 | 最符合“外部 robot”语义，但需要 agent 先连入再创建房间，启动顺序复杂 |
| **A2** | **保留人类创建 PVE 房间；gateway 维护外部机器人注册池，`createRoomPve` 从池中取 2 个已注册的 agent 加入房间，替代内部 ROBOT** | 人类流程完全不变；agent 启动顺序灵活；改动面小 |

**A2 具体流程：**
1. Agent 启动后连入 gateway，发送 `setNickname {nickname}`，昵称固定以 `robot_` 开头（如 `robot_1`、`robot_2`）。
2. Gateway 在 `CODE_CLIENT_NICKNAME_SET` 中识别 `robot_` 前缀，把该 client 加入 `ROBOT_POOL`，等待分配。
3. 人类玩家点击“开始人机对战”，发送 `createRoomPve`。
4. `CODE_ROOM_CREATE_PVE` 从 `ROBOT_POOL` 取出 2 个 robot，与人类组成房间并开始游戏。
5. 若池中机器人不足，gateway 返回错误提示（第一版不实现等待超时）。

> 注：用昵称前缀识别机器人是为了最小化契约改动；后续若需要更严谨的区分，可新增 `registerRobot` 事件。
| A3 | 人类不再点 PVE，而是开 PVP 房间，agent 作为两个普通玩家加入 | 无需改 gateway，但 UI 流程要改，且不符合现有“人机对战”入口 |

**选择 A2**：对人类流程完全不变，gateway 侧改动最小，agent 侧可用同一套 client 逻辑。

### 4.4 否决方案

- **在 C++ gateway 内直接调用 LLM HTTP API**：会阻塞 muduo 事件循环，且 C++ 拼 prompt / 解析 JSON / 错误回退都很繁琐。
- **LangChain.js 作为核心框架**：能力过剩，引入不必要的抽象和依赖。
- **纯 LLM 端到端出牌（不枚举合法选项）**：LLM 容易给出非法牌型，体验差；第一版先用规则库兜底。

## 5. 架构

```text
┌─────────────────────────────────────────────────────────────┐
│  浏览器（人类玩家）                                            │
│  web/ — React + Vite                                         │
└──────────────┬──────────────────────────────────────────────┘
               │ WebSocket + JSON
               ▼
┌─────────────────────────────────────────────────────────────┐
│  C++ Gateway (muduo + WsCodec + JsonMapHelper)               │
│  landlords-server/server.cc                                  │
│  · 人类：PLAYER role，正常收发事件                            │
│  · 机器人：外部 agent 注册到机器人池，被分配到 PVE 房间              │
└──────────────┬──────────────────────────────────────────────┘
               │ WebSocket + JSON (localhost)
               ▼
┌─────────────────────────────────────────────────────────────┐
│  llm-agent/ (Node + TypeScript + Vercel AI SDK)              │
│  · game-client.ts：连接 gateway，维护事件状态机               │
│  · agent.ts：单个 robot 的生命周期                            │
│  · decision.ts：调用 LLM API                                  │
│  · prompt.ts：生成 prompt                                     │
│  · rules.ts：复用牌型/比较规则，枚举合法选项                  │
└─────────────────────────────────────────────────────────────┘
```

## 6. Agent 状态机

每个 agent 实例维护以下状态：

```typescript
interface AgentState {
  clientId: number;
  nickname: string;
  phase: 'lobby' | 'bidding' | 'playing' | 'over';
  hand: Card[];              // 我的手牌
  seats: SeatInfo[];         // 座位信息
  turnClientId: number;      // 当前轮到谁
  lastPlay: LastPlay | null; // 上一轮出牌
  landlordId: number | null; // 地主 id
  myRole: 'LANDLORD' | 'PEASANT' | null;
}
```

事件处理：

| 收到事件 | 动作 |
|---|---|
| `idSet {clientId}` | 记录 id，发送 `setNickname {nickname}`（昵称前缀声明为 robot，gateway 将其注册到机器人池） |
| `showOptions` | 进入 lobby |
| `gameStarting {pokers, nextClientId}` | 记录手牌，若 `nextClientId === clientId` 则准备抢地主 |
| `landlordElect {nextClientId}` | 若轮到自己，调用 LLM 决策是否抢地主，发送 `landlordElect {grab}` |
| `landlordConfirm {landlordId, additionalPokers}` | 更新地主；若自己是地主，把 `additionalPokers` 加入手牌 |
| `showPokers {clientId, pokers}` | 更新 `lastPlay` |
| `playRedirect / playTurn {sellClientId, pokers}` | 若 `sellClientId === clientId`，调用 LLM 决策出牌，发送 `play` 或 `pass` |
| `playPass {nextClientId}` | 更新 `turnClientId` |
| `playError {code}` | 根据 code 重试或 fallback 到规则库 |
| `gameOver {winnerNickname, winnerType}` | 进入 over，可选重连 |

## 7. Prompt 设计

### 7.1 抢地主决策

```text
你是斗地主玩家 {nickname}。
当前手牌：{hand_desc}。
请决定是否抢地主。返回 JSON：
{
  "action": "grab" | "pass",
  "reason": "..."
}
```

### 7.2 出牌决策

```text
你是斗地主玩家 {nickname}，身份：{role}。
当前手牌：{hand_desc}。
场上最近出牌：{last_play_desc}（由 {last_player} 出）。
当前你必须出牌，且只能从以下合法选项中选一组（已通过规则校验）：
1. [3,3,3,4] — 三带一
2. [J,J,J,J] — 炸弹
3. [小王,大王] — 王炸
4. pass — 要不起

请返回 JSON：
{
  "action": "play" | "pass",
  "cards": [{"level":3,"type":0}, ...],
  "reason": "..."
}
```

`hand_desc` 用 `{level, type}` 数组或自然语言（如“3,3,4,5,5”），优先保留 `level/type` 以便程序解析。

### 7.3 结构化输出

使用 `ai` SDK 的 `generateObject`：

```typescript
const schema = z.object({
  action: z.enum(['play', 'pass', 'grab']),
  cards: z.array(z.object({ level: z.number(), type: z.number() })).optional(),
  reason: z.string(),
});
```

## 8. 合法性兜底

1. **选项枚举**：用 `rules.ts` 中的 `validSells(lastPlay, hand)` 列出所有能压过上家的合法牌型。
2. **LLM 选择**：把选项列表放进 prompt，LLM 只返回选项编号或 pass。
3. **结果校验**：收到 LLM 返回后，再次检查牌是否在手、牌型是否合法、是否能压住上家；不通过则 fallback。
4. **Fallback 策略**：
   - 有合法出牌时选最小能压过的牌（保守策略）。
   - 无合法出牌时 pass。
   - 若自己是首家（无 lastPlay），必须出牌，选最小单张。

## 9. 测试策略

### 9.1 单元测试

- `prompt.test.ts`：验证不同局面下 prompt 包含正确信息。
- `decision.test.ts`：mock LLM 返回，验证解析、校验、fallback。
- `rules.test.ts`：复用/迁移 `web/mock-server/test/` 中的牌型与比较测试。

### 9.2 集成测试

- 启动 `./gateway 127.0.0.1 8787`。
- 启动 2 个 agent（连接真实 LLM 或 mock）。
- 用 `web/e2e/diag-robot.ts` 或类似脚本模拟人类玩家。
- 断言一局能正常走到 `gameOver`，无 5 秒以上 stall。

### 9.3 LLM Mock 模式

通过环境变量 `LLM_PROVIDER=mock` 让 agent 使用固定策略（如“有牌就出最小的，没牌就 pass”），保证 CI 不依赖真实 API key。

## 10. 启动与配置

新增 `llm-agent/package.json` 脚本：

```json
{
  "scripts": {
    "dev": "tsx src/main.ts",
    "test": "vitest run",
    "smoke": "tsx scripts/smoke.ts"
  }
}
```

环境变量示例 `.env.example`：

```bash
GATEWAY_URL=ws://127.0.0.1:8787
LLM_PROVIDER=openai          # openai | anthropic | ollama | mock
OPENAI_API_KEY=sk-...
OPENAI_BASE_URL=https://api.openai.com/v1
OPENAI_MODEL=gpt-4o-mini
LLM_TEMPERATURE=0.3
AGENT_COUNT=2
AGENT_NICKNAME_PREFIX=robot
```

## 11. 任务拆分建议

1. 搭建 `llm-agent/` TypeScript 工程 + 依赖。
2. 实现 `game-client.ts` WebSocket 连接与事件分发。
3. 实现 `rules.ts`（复用 mock-server 规则）。
4. 实现 prompt 生成与 LLM 调用。
5. 实现 agent 状态机与决策循环。
6. 改造 gateway：`CODE_CLIENT_NICKNAME_SET` 识别 `robot_` 前缀并加入 `ROBOT_POOL`；`CODE_ROOM_CREATE_PVE` 从池中取 2 个 robot 替代内部 ROBOT。
7. 编写单元测试与集成 smoke 测试。
8. 联调端到端：人类 + 2 LLM agent 完成一局。

## 12. 风险与应对

| 风险 | 应对 |
|---|---|
| LLM API 延迟高，出牌有卡顿 | 先支持 mock provider 跑通流程；真实 LLM 可选较小模型（gpt-4o-mini） |
| LLM 返回非法牌型 | 合法性兜底 + fallback |
| Gateway 改动破坏人类流程 | 先保证人类 PVE 仍可用，再接入 agent |
| 成本问题 | 支持本地 Ollama，零 API 成本 |
| 多 agent 同时连接出现竞态 | 每个 agent 独立 clientId，gateway 正常排队 |
