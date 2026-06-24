# ratel 网页游戏化 设计文档

- 日期：2026-06-24
- 作者：san（与 Claude 协作）
- 状态：已通过设计评审，待实现计划

## 1. 背景与目标

`ratel` 是一个 C++ 命令行斗地主（Dou Dizhu），三模块结构：

- `landlords-server`：服务端，含房间管理、发牌、抢地主、出牌校验、机器人 AI（`RobotDecisionMakers`）。
- `landlords-client`：命令行客户端（保留）。
- `landlords-common`：公共逻辑（牌型、序列化、protobuf 编解码、AI）。

依赖：muduo（异步 TCP）+ Boost（含 serialization）+ protobuf。约 12k 行 C++。

**目标：** 在不改动现有 C++ 服务端与游戏逻辑的前提下，增加一个网页前端，使斗地主可在浏览器中游玩。

**核心约束（决定架构）：** 现有服务端的网络协议为 **muduo 的 protobuf-over-TCP**，且 `Query`/`Answer` 内层的游戏数据用 **Boost binary archive** 序列化（`MapHelper`，含 6 类嵌套 map：`vector<Poker>`、`vector<ClientInfo>`、`vector<RoomInfo>` 等）。Boost binary 是 C++ 内部格式、非跨语言稳定规范；浏览器无法直连，外部语言逆向该二进制脆弱。因此网关用 **C++ 实现、复用 `landlords-common`**，让所有序列化在原生 C++ 侧完成。

## 2. 范围（V1）

**已确认：V1 = 人机对战（PVE）+ 简化房间。**

V1 包含：
- 设置昵称。
- 创建 PVE 房间（服务端自动补足机器人）。
- 按房间号加入房间（打通房间链路，验证多客户端路由）。
- 抢地主流程。
- 出牌流程（选牌、出牌、不出、合法性校验反馈）。
- 一局结束结算 + 再来一局。

V1 不包含（YAGNI，留待后续）：
- 完整大厅 / 房间列表浏览（`CODE_SHOW_ROOMS` / `CODE_GET_ROOMS` 暂不接 UI）。
- 观战（`CODE_GAME_WATCH`）。
- 踢人（`CODE_CLIENT_KICK`）、设置菜单（`CODE_SHOW_OPTIONS_SETTING`）。
- 账号 / 登录 / 持久化。
- 移动端深度适配（先保证桌面端可用）。
- PVP 真人匹配（链路已支持，留后续）。

## 3. 总体架构（三段链路）

```
浏览器(React)  ──WebSocket + JSON──>  landlords-gateway(C++/Asio)  ──TCP(muduo帧+protobuf)──>  现有 C++ 服务端
   (新增)                                  (新增, 复用 landlords-common)                       (不变)
```

- **现有 C++ 服务端：零改动。** 游戏逻辑、牌型校验、机器人 AI 全部保留并复用。
- **`landlords-gateway`（新增 C++ 程序）：** 唯一的协议翻译层。
  - 浏览器侧：Boost.Asio/Beast 提供 WebSocket，收发 JSON。
  - 服务端侧：裸 TCP，自行实现 muduo 的长度前缀 + adler32 校验帧，承载 protobuf `Query`/`Answer`。
  - 关键的 Boost-binary `MapHelper` 序列化仍在 C++ 侧完成（直接复用 `landlords-common` 的 `SerializeHelper` / `MapHelper` / `Poker` 等定义），无需逆向任何二进制格式。
- **每个浏览器会话 = 网关到服务端的一条 TCP 连接**，角色等同现有 CLI 客户端。

## 4. 组件

### 4.1 现有 C++ 服务端（不变）

不修改。继续监听 muduo TCP 端口，提供房间/对局/AI。

### 4.2 `landlords-gateway`（新增）

单进程、单 Asio `io_context`（可扩展为多线程，V1 单线程足够）。

子模块：

- `ws_server`：Beast WebSocket 服务，管理多个浏览器会话；每个会话持有一条到服务端的连接。
- `server_link`：封装到服务端的 TCP 连接 + muduo 帧编解码 + protobuf `Query`/`Answer` 收发。复用 `landlords-common` 的 `SerializeHelper`、`MapHelper`、protobuf 消息。
- `mapjson`：`MapHelper ↔ JSON` 双向转换（见 §6.3）。内嵌事件级 schema。
- `gateway.cc`：装配与生命周期。

依赖：Boost（Asio + Beast，要求 Boost ≥ 1.66；项目已依赖 Boost）+ protobuf + 现有 `landlords-common`。可选 `nlohmann/json`（header-only）做 JSON，或手写极简 JSON（V1 字段简单）。

### 4.3 Web 前端（新增，`web/`）

技术栈：React 18 + Vite + TypeScript + Tailwind CSS。设计语言：采用 **Linear 的 `DESIGN.md`**（取自 voltagent/awesome-design-md）作为色彩 / 字体 / 间距 / 圆角 / 阴影 / 动效令牌来源；牌桌等游戏专属布局自行设计。

## 5. 现有网络协议（网关必须忠实复刻）

### 5.1 muduo 传输帧（`ProtobufCodec`）

来源：`landlords-common/protobuf/codec.h` 注释。所有 `int32` 为**大端（网络字节序）**。

```
struct ProtobufTransportFormat {
  int32_t  len;          // = nameLen(4) + sizeof(typeName) + sizeof(protobufData) + checkSum(4)
  int32_t  nameLen;      // = sizeof(typeName)
  char     typeName[nameLen];       // 例 "muduo.Answer" / "muduo.Query"
  char     protobufData[len - nameLen - 8];
  int32_t  checkSum;     // adler32 over ( nameLen 的 4 字节 BE + typeName + protobufData )
}
```

实现要点：adler32 需自行实现（约 10 行）或引入小库；网关侧负责按 `len` 分帧、校验 `checkSum`（校验失败可记日志并丢弃）。

### 5.2 protobuf 信封（`query.proto`）

```
message Query  { int64 id=1; string questioner=2; repeated string question=3; }  // 客户端→服务端
message Answer { int64 id=1; string questioner=2; string answerer=3; repeated bytes solution=4; } // 服务端→客户端
```

- `Query.id` = `ServerEventCode`；`Query.question[0]` = Boost-binary 序列化的 `MapHelper`。
- `Answer.id` = `ClientEventCode`；`Answer.solution[0]` = Boost-binary 序列化的 `MapHelper`。

> 注：proto 中 `question` 为 `repeated string`、`solution` 为 `repeated bytes`；现有代码均只用下标 0。网关沿用。

### 5.3 事件码

`ClientEventCode`（`Answer.id`，服务端→客户端，V1 用子集）：

| 值 | 名称 | V1 |
|---|---|---|
| 3 | CODE_CLIENT_CONNECT | ✅ 加入成功 |
| 9 | CODE_SHOW_POKERS | ✅ 我的手牌 |
| 10 | CODE_ROOM_CREATE_SUCCESS | ✅ |
| 11 | CODE_ROOM_JOIN_SUCCESS | ✅ |
| 12/13 | 房间加入失败（满/不存在） | ✅ |
| 15 | CODE_GAME_STARTING | ✅ |
| 16 | CODE_GAME_LANDLORD_ELECT | ✅ 抢地主 |
| 17 | CODE_GAME_LANDLORD_CONFIRM | ✅ 地主确认 |
| 19 | CODE_GAME_POKER_PLAY | ✅ 我的出牌回合 |
| 20 | CODE_GAME_POKER_PLAY_REDIRECT | ✅ 出牌广播 |
| 21 | CODE_GAME_POKER_PLAY_MISMATCH | ✅ 不匹配 |
| 22 | CODE_GAME_POKER_PLAY_LESS | ✅ 太小 |
| 23 | CODE_GAME_POKER_PLAY_PASS | ✅ 不出广播 |
| 24 | CODE_GAME_POKER_PLAY_CANT_PASS | ✅ 不允许不出 |
| 25 | CODE_GAME_POKER_PLAY_INVALID | ✅ 无效 |
| 26 | CODE_GAME_POKER_PLAY_ORDER_ERROR | ✅ 顺序错误 |
| 27 | CODE_GAME_OVER | ✅ |
| 31 | CODE_GAME_ID_SET | ✅ 会话 id |

`ServerEventCode`（`Query.id`，客户端→服务端，V1 用子集）：

| 值 | 名称 | V1 |
|---|---|---|
| 2 | CODE_CLIENT_NICKNAME_SET | ✅ |
| 5 | CODE_ROOM_CREATE_PVE | ✅ |
| 7 | CODE_ROOM_JOIN | ✅ |
| 9 | CODE_GAME_LANDLORD_ELECT | ✅ |
| 10 | CODE_GAME_POKER_PLAY | ✅ |
| 12 | CODE_GAME_POKER_PLAY_PASS | ✅ |
| 0 | CODE_CLIENT_EXIT | ✅ |

（其余事件码在 V1 网关中以 `unknown` 透传，前端忽略。）

## 6. 浏览器 ↔ 网关 JSON 契约

### 6.1 卡牌数据模型

```
type PokerType = 0|1|2|3|4;   // BLANK=0, DIAMOND=1(♦), CLUB=2(♣), SPADE=3(♠), HEART=4(♥)
type PokerLevel = 0..14;      // 3=0,4=1,5=2,6=3,7=4,8=5,9=6,10=7,J=8,Q=9,K=10,A=11,2=12,SMALL_KING=13(S),BIG_KING=14(X)
interface Card { type: PokerType; level: PokerLevel; }
```

牌型 `SellType`（仅用于展示）：ILLEGAL=0 … FOUR_STRAIGHT_WITH_DOUBLE=17, VOID_SELL=18。

### 6.2 事件 JSON（V1）

统一外层：`{ event: string, data?: object }`。客户端→服务端（C→S）：

| event | data | 映射 ServerEventCode |
|---|---|---|
| `setNickname` | `{ nickname: string }` | CODE_CLIENT_NICKNAME_SET |
| `createRoomPve` | `{}` | CODE_ROOM_CREATE_PVE |
| `joinRoom` | `{ roomId: number }` | CODE_ROOM_JOIN |
| `landlordElect` | `{ grab: boolean }` | CODE_GAME_LANDLORD_ELECT |
| `play` | `{ pokers: Card[] }` | CODE_GAME_POKER_PLAY |
| `pass` | `{}` | CODE_GAME_POKER_PLAY_PASS |
| `exit` | `{}` | CODE_CLIENT_EXIT |

服务端→客户端（S→C）：

| event | data 关键字段 | 源 ClientEventCode |
|---|---|---|
| `idSet` | `{ clientId: number }` | CODE_GAME_ID_SET |
| `connected` | `{}` | CODE_CLIENT_CONNECT |
| `roomCreate` | `{ roomId, owner, clientCount, type }` | CODE_ROOM_CREATE_SUCCESS |
| `roomJoin` | `{ roomId, owner, clientCount, type }` | CODE_ROOM_JOIN_SUCCESS |
| `roomJoinFail` | `{ reason: "full" \| "inexist" }` | CODE_ROOM_JOIN_FAIL_* |
| `gameStarting` | `{}` | CODE_GAME_STARTING |
| `landlordElect` | `{ client: number, nickname: string }` | CODE_GAME_LANDLORD_ELECT |
| `landlordConfirm` | `{ landlord: number, nickname: string }` | CODE_GAME_LANDLORD_CONFIRM |
| `showPokers` | `{ pokers: Card[] }` | CODE_SHOW_POKERS |
| `playTurn` | `{}` | CODE_GAME_POKER_PLAY |
| `playRedirect` | `{ sellClient, sellNickname, sellPokers: Card[], sellType, nextClient }` | CODE_GAME_POKER_PLAY_REDIRECT |
| `playPass` | `{ client }` | CODE_GAME_POKER_PLAY_PASS |
| `playError` | `{ reason: "mismatch" \| "less" \| "invalid" \| "order" \| "cantPass" }` | _MISMATCH/_LESS/_INVALID/_ORDER_ERROR/_CANT_PASS |
| `gameOver` | `{ winner, landlord, nickname }` | CODE_GAME_OVER |
| `unknown` | `{ rawCode: number, fields?: object }` | 其余透传 |

> 各事件的精确 `MapHelper` 键与类型，在实现期对照 `landlords-client/event/eventFuncs.cc` 逐一对齐并补全到网关 schema 表；上表为 V1 契约基线。

### 6.3 `MapHelper ↔ JSON` 映射策略

`MapHelper` 内含 6 类强类型 map（`strMap`/`intMap`/`pokerVecMap`/`levelVecMap`/`clientInfos`/`roomInfos`），但键为泛型字符串、且不暴露遍历接口。

策略：网关内置**事件级 schema 表**——为每个事件声明它涉及哪些键、各键的类型（int/str/pokers/levels/clientInfos/roomInfos）。转换时：

- C++ → JSON：按 schema 用对应类型的 `get(key, T{})` 取值并写入 JSON。
- JSON → C++：按 schema 用对应类型的 `put(key, value)` 写入空 `MapHelper`。

事件与键集合有界，可从现有客户端监听代码完整推导。这是网关唯一的"知识密集"部分。

## 7. 前端架构

### 7.1 状态机 / 画面

`连接中 → 设昵称 → 大厅(简化) → 房间/对局中 → 结算`。由 WS 事件驱动转移。

### 7.2 核心组件

- `LobbyView`：创建 PVE 房间、按号加入房间。
- `CardTable`：3 个座位（我 + 2 机器人），显示昵称、剩余牌数、轮次指示、地主标识。
- `Hand`：可选中的手牌（点击切换选中态，按 level 排序展示）。
- `ActionBar`：出牌 / 不出；抢地主时切换为 抢 / 不抢。
- `BiddingOverlay` / `ResultOverlay`。
- `Toast`：出牌错误提示。

### 7.3 状态管理

`useReducer` 管一份游戏状态：`{ phase, clientId, nickname, roomId, hand: Card[], lastSell, turnClient, landlord, seats }`。WS 消息 → action → reducer。YAGNI：不引入 Redux/Zustand。

### 7.4 DESIGN.md 应用方式

将 Linear `DESIGN.md` 的设计令牌（色彩、字体阶梯、间距尺度、圆角、阴影、动效）转写为：

- `tailwind.config` 的 theme 扩展（CSS 变量 + 自定义颜色 / 字号 / 间距）。
- 一组基础组件（Button / Card / Badge / Modal / Toast），游戏组件在其上构建。

## 8. 错误处理与降级

- 网关↔服务端断连 / 浏览器↔网关 WS 断连：前端置 `连接断开` 态，提供"重连"按钮；重连后重新走 设昵称→（如服务端仍持有则可续，否则）新建流程。V1 不保证对局中断续传。
- 出牌非法：服务端已返回明确错误码（mismatch/less/invalid/order/cantPass），前端 toast 提示并**保留**当前手牌选中，便于修改后重出。
- 网关收到未知事件：以 `unknown` 透传原始码，前端忽略，避免阻塞已知流程。
- 网关对未知 JSON 事件字段：忽略多余键，不报错。

## 9. 测试策略

- **网关：**
  - 单元：muduo 帧编解码往返（`Query/Answer ↔ bytes`，含 adler32 校验）。
  - 单元：`MapHelper ↔ JSON` 往返（针对每个 V1 事件 schema）。
  - 集成：用 mock 服务端跑端到端"设昵称→创房→抢地主→出牌→结束"。
- **前端：**
  - reducer 纯函数测试（给定事件序列 → 预期状态）。
  - 关键组件（Hand 选牌、ActionBar 状态）快照/交互测试。
- **手工验收：** 启动 服务端 + 网关，浏览器打一局完整 PVE 并验证错误反馈。

## 10. 目录结构（新增）

```
ratel/
├── landlords-server/        (不变)
├── landlords-client/        (不变, 保留 CLI)
├── landlords-common/        (不变)
├── landlords-gateway/       (新增 C++ 网关)
│   ├── Makefile
│   ├── gateway.cc
│   ├── ws_server.{h,cc}
│   ├── server_link.{h,cc}
│   ├── mapjson.{h,cc}
│   └── muduo_frame.{h,cc}     // 帧编解码 + adler32
├── web/                     (新增前端)
│   ├── DESIGN.md              // Linear 风格, 取自 awesome-design-md
│   ├── package.json
│   ├── vite.config.ts
│   ├── tailwind.config.ts
│   ├── tsconfig.json
│   └── src/
│       ├── main.tsx
│       ├── App.tsx
│       ├── state/{reducer,types}.ts
│       ├── net/{socket}.ts        // WS 客户端
│       ├── components/{LobbyView,CardTable,Hand,ActionBar,...}.tsx
│       └── styles/index.css
└── docs/superpowers/specs/2026-06-24-web-game-design.md  (本文件)
```

## 11. 风险与待验证

- **Boost 版本 / Beast 可用性：** 网关依赖 Boost ≥ 1.66 的 Beast。实现前需确认本机 Boost 版本；若过旧，备选用 `websocketpp` 或纯手写 WebSocket 帧层。
- **`MapHelper` schema 对齐：** 需逐事件对照 `eventFuncs.cc` 抽取键与类型，是网关正确性的关键。
- **muduo 帧与现有服务端字节级一致：** adler32 计算范围与字段顺序必须与 `ProtobufCodec` 完全一致；实现期以真机往返测试兜底。
- **多线程安全（若启用）：** V1 单线程；若后续多线程，Asio strand 保护每会话状态。
