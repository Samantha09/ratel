# C++ WebSocket Gateway 设计：前端接入真实斗地主后端

- 日期：2026-06-24
- 分支：`feature/web-game`
- 状态：草案，待审阅
- 关联：`docs/superpowers/plans/2026-06-24-mock-gateway.md`（Plan 1，mock gateway，已完成）、`docs/superpowers/plans/2026-06-24-frontend.md`（Plan 2，前端，已完成）

## 1. 背景与目标

Web 前端（`web/`，React + Vite + Tailwind）目前只连接 Plan 1 的 mock gateway（Node 实现，`web/mock-server/`）。本设计的目标是**让前端接入真实的 C++ 斗地主后端**（`landlords-server` + `landlords-common`），跑通一局真实对局。

真实后端的游戏逻辑（房间、发牌、抢地主、牌型判定、出牌比较、机器人 AI）全部用 C++ 实现且已可用；前端期望的接入形态是 WebSocket + JSON。两者之间存在传输协议与消息格式的鸿沟，需要一座 gateway 桥接。

## 2. 范围

### 2.1 第一版目标（本次实现）

**最小链路打通**：前端连上真实 C++ gateway，能完成 PVE 单局完整流程——设置昵称 → 创建人机房间 → 抢地主 → 出牌到结算。证明 `前端 ↔ WS+JSON ↔ C++ 游戏逻辑` 这条链路走通。

### 2.2 非目标（第一版不做）

- PVP（真人对战）、观战、房间列表浏览
- 并发承载、断线重连、异常恢复的生产级处理
- 出牌动画、计时器、音效等体验增强
- 保留原控制台客户端（`landlords-client`）可用——第一版放弃 TCP+Protobuf 协议

## 3. 现状分析

### 3.1 C++ 后端协议

- **传输层**：muduo 网络库的裸 TCP（`muduo::net::TcpServer`）。入口 `landlords-server/server.cc` 的 `QueryServer`，命令行传入 `ip port`。
- **消息编码**：Protobuf 外层（`query.proto` 的 `Query`/`Answer`）+ Boost binary 序列化内层（`MapHelper` 经 `boost::archive::binary_oarchive`）。编解码见 `landlords-common/protobuf/codec.cc`，序列化见 `landlords-common/helper/SerializeHelper.h`。
- **游戏逻辑入口**：`serverEventListener(conn, ServerEventCode, MapHelper)`（`landlords-server/event/ServerEventListener.h`）。
- **游戏逻辑出口**（单一集中点）：`pushDataToClient(codec, conn, ClientEventCode, MapHelper)`（`landlords-server/event/EventFuncs.cc`）→ `codec->send(conn, Answer{...})`。`ServerEventListener::pushToClient` 是同义的简化出口。
- **玩家对象**：`ClientSide` 持有 `conn`（`muduo::net::TcpConnectionPtr`）。机器人（`ClientRole::ROBOT`）通过 `std::thread` + `RobotEventListener::get` 驱动决策。
- **事件枚举**：`landlords-common/enums/ClientEventCode.h`（服务端→客户端，32 项）、`landlords-common/enums/ServerEventCode.h`（客户端→服务端，14 项）。
- **全局状态**：`ServerContains` 单例管理 clientId 分配、房间表、客户端表。

### 3.2 关键约束

`MapHelper` 含 6 种 `unordered_map`（`string→string`、`string→int`、`string→vector<Poker>`、`string→vector<PokerLevel>`、`string→vector<ClientInfo>`、`string→vector<RoomInfo>`），经 Boost binary archive 序列化。Boost 的 binary archive 是 **C++ 私有、版本相关、含 class tracking 的二进制格式**，在非 C++ 环境（Node 等）中可靠复现其反序列化不可行。这否决了"纯 Node 网关直接对接 C++ TCP 服务端"的方案。

### 3.3 工具链现状

- 缺失：`g++`/`c++`（仅有 `gcc`）、`libboost_serialization`、完整 `libprotobuf-dev`（仅有 lite/c 版）
- 就绪：`gcc`、`zlib`、`muduo`（随仓库 `lib/muduo/` 提供）、`apt`
- 缺失项均可通过 `apt install` 补齐

### 3.4 前端契约来源

前端现有事件契约（`web/src/types.ts`）来自 Plan 1 的 mock gateway（`web/mock-server/src/game.ts`）。mock 是用 Node 重新实现的简化游戏模型，其事件语义/字段/时序与真实 C++ 后端**存在多处不一致**（见 §6）。

## 4. 方案决策

### 4.1 选定方案：改造现有 server，单进程 WS+JSON，游戏逻辑零改动

将 `landlords-server` 的传输层从 `muduo TCP + Protobuf + Boost binary` 替换为 `WebSocket(文本) + JSON`：

- 保留 muduo 作为网络底座（EventLoop / TcpConnection / Buffer），`conn` 类型不变 → 所有游戏逻辑函数签名 `(codec/conn, ...)` 保持兼容，**业务代码（`EventFuncs.cc` 的 `CODE_*` 函数、`ServerContains`、`PokerHelper`、`RobotEventListener`）零改动**。
- 只改两个边界：入口解码（WS 帧 → JSON → 事件）、出口编码（`MapHelper` → JSON → WS 帧）。
- 移除 Protobuf 层（`codec.cc` / `query.proto` / `dispatcher`）与 Boost binary 运行时依赖。

### 4.2 契约策略：以 C++ 真实事件语义为准，重对齐前端契约

gateway 做薄翻译（仅做字段名规范化，如 `grab`↔`is_Y`），事件语义完全遵循 C++ 后端。前端 `types.ts` 与 `gameReducer.ts` 按真实事件语义重写（包括 `showPokers`=某人出的牌、出牌双事件 `SHOW_POKERS`+`PLAY_REDIRECT`、`landlordConfirm` 携带底牌等）。

### 4.3 否决方案

- **纯 Node 网关对接 C++ TCP 服务端**：Boost binary 跨语言不可行（§3.2）。
- **双进程：现有 server 不动 + 新写 C++ 翻译桥**：虽能保留控制台客户端，但桥需维护 WS↔TCP↔clientId 三重连接映射，两进程、多一跳延迟，复杂度高于直接改造。
- **重写网络层 + 重构 conn 抽象**（uWebSockets + session 接口）：改动面最大、风险最高，不契合最小链路目标。

## 5. 架构

```
浏览器 ──WebSocket(文本 JSON)──> 【改造后的 C++ server (单进程)】
                                       │
          ┌────────────────────────────┼───────────────────────────┐
          │                            │                           │
     WsCodec                    JsonMapHelper                游戏逻辑 (零改动)
  (muduo Buffer 上的             (MapHelper ↔ JSON,       EventFuncs / ServerContains
   WS 握手 + 文本帧编解码,        替换 Boost binary)       / PokerHelper / RobotEventListener
   替换 ProtobufCodec)
          │                            │                           │
          └──────────── 入口/出口两个边界 ─────────────────────────┘
```

- **入口**：`conn 收到字节 → WsCodec 解帧(文本) → JSON parse → {event, data} → JsonMapHelper 转 MapHelper → serverEventListener(conn, ServerEventCode, map)`
- **出口**：`pushDataToClient(conn, ClientEventCode, map) → JsonMapHelper 转 JSON → WsCodec 封文本帧 → conn->send()`

## 6. 事件契约（gateway ↔ 前端）

聚焦 PVE 一局流程涉及的事件。

### 6.1 前端 → gateway（→ `ServerEventCode` + `MapHelper` 字段规范化）

| 前端 event | → C++ code | MapHelper 字段 |
|---|---|---|
| `setNickname {nickname}` | `CODE_CLIENT_NICKNAME_SET` | `nickName` |
| `createRoomPve {}` | `CODE_ROOM_CREATE_PVE` | `clientId`, `choose=0`（难度，默认简单） |
| `landlordElect {grab:bool}` | `CODE_GAME_LANDLORD_ELECT` | `is_Y = "true"/"false"` |
| `play {pokers:[{level,type}...]}` | `CODE_GAME_POKER_PLAY` | `options = [levels]` |
| `pass {}` | `CODE_GAME_POKER_PLAY_PASS` | — |

### 6.2 gateway → 前端（`ClientEventCode` → JSON，遵循 C++ 真实语义）

| C++ code | 前端 event | data |
|---|---|---|
| `CODE_GAME_ID_SET` | `idSet` | `{clientId}` |
| `CODE_GAME_STARTING` | `gameStarting` | `{pokers:[我的手牌], clientId, nextClientId, nextClientNickname, roomOwner, roomClientCount}` |
| `CODE_GAME_LANDLORD_ELECT` | `landlordElect` | `{nextClientId, nextClientNickname, preClientNickname}` |
| `CODE_GAME_LANDLORD_CONFIRM` | `landlordConfirm` | `{landlordId, landlordNickname, additionalPokers:[底牌3张]}` |
| `CODE_SHOW_POKERS` | `showPokers` | `{pokers:[某人**出的牌**], clientId, clientNickname, clientType}` |
| `CODE_GAME_POKER_PLAY_REDIRECT` | `playRedirect` | `{pokers:[我的完整手牌], clientInfos:[其他座位信息], lastSellPokers, lastSellClientId, sellClientId, sellClinetNickname}` |
| `CODE_GAME_POKER_PLAY_PASS` | `playPass` | `{clientId, clientNickname, nextClientId, nextClientNickname}` |
| `*_MISMATCH / _LESS / _INVALID / _ORDER_ERROR / _CANT_PASS` | `playError` | `{code: "mismatch"/"less"/"invalid"/"order"/"cantPass"}` |
| `CODE_GAME_OVER` | `gameOver` | `{winnerNickname, winnerType: "LANDLORD"/"PEASANT"}` |

### 6.3 PVE 一局真实事件时序（gateway 视角）

1. 浏览器建立 WS 连接 → `onConnection` → 发 `idSet {clientId}`
2. 前端 `setNickname` → 服务端记昵称（`CODE_SHOW_OPTIONS`，前端可忽略或用于驱动下一步）
3. 前端 `createRoomPve` → 服务端建房间 + 2 robot，构成 3 人环 → `gameStarting {pokers, nextClientId(先抢者)}`
4. 若先抢者为 robot，服务端内部线程驱动 robot 决策；轮到玩家则等 `landlordElect`
5. 玩家 `landlordElect {grab}`：grab=true → `landlordConfirm {landlordId, additionalPokers}`；grab=false → 转 `landlordElect {nextClientId}` 或全员不抢重新发牌
6. 地主确认后进入出牌：玩家 `play`/`pass` → 合法则广播 `showPokers {某人出的牌}` 给玩家，并给下一出牌者 `playRedirect {完整手牌, clientInfos, lastSell}`；下一者为 robot 则线程驱动
7. 某人手牌出完 → `gameOver {winnerNickname, winnerType}`

## 7. 组件设计

### 7.1 `WsCodec`（新增，`landlords-common` 下）

在 muduo 的 `Buffer` 上实现 RFC 6455 的最小子集：

- **握手**：解析 HTTP Upgrade 请求（`Upgrade: websocket`、`Sec-WebSocket-Key`），计算 `Sec-WebSocket-Accept = base64(sha1(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11"))`，回 `101 Switching Protocols`。HTTP 头解析按 `\r\n` 分行（仅握手用，不引入完整 HTTP 解析器）。
- **入帧**：解析 opcode（仅处理 text=0x1）、MASK、payload length（7/16/64 位）、解掩码；忽略/响应 close(0x8) 与 ping(0x9→pong)。
- **出帧**：服务端→客户端帧不掩码，封装为 text 帧。
- SHA-1 + base64 用小型实现（自包含，不引 OpenSSL）。

接口形态（与 `ProtobufCodec` 角色对应）：
- `void onMessage(conn, buffer, recvTime)`：muduo 消息回调，内部累积缓冲、完成握手/解帧后回调上层。
- `void sendText(conn, string_view json)`：封装 text 帧并 `conn->send()`。

### 7.2 `JsonMapHelper`（新增，`landlords-common` 下）

提供 `MapHelper ↔ JSON` 互转，替代 `SerializeHelper` 的 Boost binary 路径：

- `to_json(MapHelper, ClientEventCode) -> json`：按事件类型枚举需要序列化的字段，输出 §6.2 的 data 结构。涉及的结构映射：
  - `Poker` → `{level, type}`
  - `ClientInfo` → `{id(clientId), nickname(clientNickname), type, surplus(cardsLeft), position}`
  - `RoomInfo` → `{roomId, roomOwner, roomClientCount, roomType}`
- `from_json(json, ServerEventCode) -> MapHelper`：按 §6.1 的字段规范化解析（如 `play.pokers` → `options=[levels]`、`landlordElect.grab` → `is_Y`）。
- JSON 库：`nlohmann/json`（单 header，vendor 进 `lib/` 或 `landlords-common/`）。

> 说明：按事件类型枚举字段，而非把 `MapHelper` 的 6 个 map 全量扁平化——避免同名 key 冲突，也让契约显式可控。

### 7.3 `server.cc` 改造

- 用 `WsCodec` 替换 `ProtobufCodec` + `ProtobufDispatcher`。
- `onConnection`：保留"分配 clientId + 发 `idSet`"逻辑（`CODE_GAME_ID_SET`），改为走 WS+JSON 出口。
- `onMessage`（经 `WsCodec`）：收到文本帧 → JSON → `from_json` 得 `MapHelper` → `serverEventListener(conn, ServerEventCode(id), map)`。
- `main`：参数仍是 `ip port`（或固定端口如 8787 以对齐前端默认）。

### 7.4 `pushDataToClient` 出口改造

`landlords-server/event/EventFuncs.cc` 的 `pushDataToClient` 与 `ServerEventListener.h` 的 `pushToClient` 改为：

```
旧：SerializeHelper::SerializeToString<MapHelper>(map) → Answer → codec->send(conn, answer)
新：JsonMapHelper::to_json(map, code) → WsCodec::sendText(conn, json)
```

`codec` 参数语义变更（不再发 Protobuf）。为最小化改动，`codec` 指针可指向一个持有 `WsCodec`/`JsonMapHelper` 的轻量上下文对象，或直接改函数签名去掉 `codec`、改用全局/单例发送器——实现阶段定。游戏逻辑函数（`CODE_*`）的调用与控制流**不变**。

## 8. 数据流（双向示例）

**入口（玩家出牌）：**
```
浏览器 → {event:"play", data:{pokers:[{level:3,type:1},...]}}
  → WsCodec 解帧 → JSON → from_json(CODE_GAME_POKER_PLAY) → MapHelper.put("options",[3,...]).put("clientId",id)
  → serverEventListener(conn, CODE_GAME_POKER_PLAY, map)
  → ServerEventListener_CODE_GAME_POKER_PLAY(codec, conn, map) [游戏逻辑零改动]
```

**出口（广播某人出的牌）：**
```
游戏逻辑 pushDataToClient(conn, CODE_SHOW_POKERS, map)
  → JsonMapHelper::to_json(map, CODE_SHOW_POKERS) → {event:"showPokers", data:{pokers:[出的牌], clientId, clientNickname}}
  → WsCodec::sendText(conn, json) → 浏览器
```

## 9. 前端改动

- **`web/src/types.ts`**：`ServerEvent` / `ClientEvent` 联合类型按 §6 重写（字段名、新增 `additionalPokers`/`clientInfos`/`code` 等）。
- **`web/src/state/gameReducer.ts`**：state 形状与 reducer 适配真实语义——
  - `clientId` 使用连接时分配的真实值（不再假设 seat 0）；
  - `showPokers` 语义=某人出的牌 → 更新 `lastSell` / `seats.cardsLeft`；
  - `playRedirect` → `pokers`=我的完整手牌（覆盖 state.hand）、`clientInfos`→渲染座位；
  - `landlordConfirm` → 地主拿到 `additionalPokers` 底牌（更新手牌/牌数）；
  - `playError` 的 `code` 映射到提示文案；
  - `gameOver` 的 `winnerType` 判定胜负归属。
- **`web/src/views/GameView.tsx` / `CardTable.tsx`**：座位渲染改用 `clientInfos`（id/nickname/cardsLeft/position/是否地主）。
- **`web/src/game/Hand.tsx` / `ActionBar.tsx`**：基本不动（依赖的 state 字段名保持）。
- **`web/src/views/LobbyView.tsx`**：流程改为"输入昵称 → setNickname → createRoomPve"。
- **`web/src/hooks/useGame.ts`**：`play` 把选中牌序列化为 `{level,type}`；连接默认端口对齐 gateway。

## 10. 依赖与构建

- **apt 补齐**：`g++`、`libboost-dev`（头文件）；过渡期保留 `libboost-serialization-dev`、`libprotobuf-dev`（直到 Protobuf 层移除）。
- **引入**：`nlohmann/json`（vendor 单 header）、WS 握手用 SHA-1+base64 小型实现。
- **构建**：沿用 Make。新增 `gateway` 目标（或改造 `server` 目标），链接去掉 `-lprotobuf`，新增 JSON/WS 源。`landlords-common/protobuf/codec.cc`、`query.pb.cc` 在 gateway 构建中不再编译。

## 11. 测试策略

- **C++ 端**：第一版不引入 C++ 单测框架。用 Node `ws` 客户端脚本（`web/` 下新增 e2e 冒烟脚本）驱动：连接 → `setNickname` → `createRoomPve` → `landlordElect` → `play`/`pass` 到 `gameOver`，断言关键事件到达与字段正确。
- **前端**：`gameReducer.test.ts` 按新契约重写；现有组件测试（`Hand`/`ActionBar`/`LobbyView`/`PlayingCard`/`App`）适配新 state 形状。
- **端到端验证**：前端 dev server + 真实 C++ gateway，人工完成一局。

## 12. 风险与开放问题

- **muduo 上实现 WebSocket**：muduo 无原生 WS 支持。需在 `Buffer` 上手写握手+帧（约 200~300 行）。风险：握手/帧边界 bug。缓解：e2e 冒烟脚本覆盖；先用最小文本帧。
- **`codec` 参数的签名迁移**：游戏逻辑函数签名携带 `ProtobufCodec* codec`，改造发送路径需决定是保留 `codec` 指针（指向新上下文）还是改签名。倾向保留指针、内部重定向，以零改动业务调用。
- **clientId 语义**：真实 clientId 为递增正整数（玩家）/负数（robot）。前端"是不是我回合"判断须用真实 `clientId`，不能再用 seat 索引。
- **机器人时序**：robot 由 `std::thread` 驱动并在服务端线程内 `join`，可能阻塞 muduo 事件循环。现有行为如此，第一版沿用，观察是否影响 WS 响应延迟。
- **字段名笔误**：C++ 代码中存在 `sellClinetNickname`（应为 sellClient）、`getLoadlordPokers`/`getLandlordPokers` 不一致等历史笔误。gateway 契约统一用规范名（`sellClientNickname`），内部做容错读取。
- **牌型 `sellType` 缺失**：真实后端 `SHOW_POKERS` 只给原始 `pokers`，不提供牌型枚举；而 mock 的 `playRedirect.sellType` 曾被前端 reducer 使用。第一版处理：前端若需展示牌型，从 `web/mock-server/src/cardtype.ts` 移植轻量检测到 `web/src`；否则不展示。

## 13. 验收标准（最小链路打通）

1. C++ gateway 编译通过（`make gateway`），监听 WS 端口。
2. Node e2e 冒烟脚本能与 gateway 完成一局 PVE，收到完整的 `idSet → gameStarting → landlordElect → landlordConfirm → showPokers/playRedirect/playPass → gameOver` 序列，字段符合 §6.2。
3. 前端连上真实 gateway（非 mock），能输入昵称、开局、抢地主、出牌，UI 正确反映真实后端状态，直到一局结算。
4. 前端测试套件（`npm test`）在新契约下全绿；`npm run build` 通过。
