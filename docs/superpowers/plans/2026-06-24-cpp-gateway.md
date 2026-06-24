# C++ WebSocket Gateway Implementation Plan (Plan A)

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a C++ WebSocket+JSON gateway that reuses the existing 斗地主 game logic, so the web frontend can play a real PVE game against it (Plan B will adapt the frontend).

**Architecture:** Single-process改造 of `landlords-server`: replace the `muduo TCP + Protobuf + Boost binary` transport with `WebSocket(text) + JSON`, keeping `conn` as `muduo::net::TcpConnectionPtr` so all game-logic functions (`EventFuncs.cc` `CODE_*`, `ServerContains`, `PokerHelper`, `RobotEventListener`) stay unchanged. Two boundaries change only: ingress decode (WS frame → JSON → `MapHelper` → `serverEventListener`) and egress encode (`pushDataToClient` → `MapHelper` → JSON → WS frame).

**Tech Stack:** C++ (g++, muduo network lib vendored at `lib/muduo`), nlohmann/json (header-only), hand-written SHA-1/base64/WebSocket framing (no OpenSSL), Node `ws` client for e2e smoke.

**Spec:** `docs/superpowers/specs/2026-06-24-cpp-gateway-design.md`

## Global Constraints

- Compile flags (from existing `Makefile`): `g++ -g -I lib/muduo/include -I landlords-common -pthread -L lib/muduo/lib`
- Link (transition): `-lboost_serialization -lprotobuf -lz -lmuduo_base -lmuduo_net -lpthread`; the `gateway` target drops `-lprotobuf` once Protobuf layer is removed (Task 5).
- Gateway listens on `127.0.0.1:8787` (matches frontend default `ws://127.0.0.1:8787`).
- No new C++ test framework; pure-logic components ship an assert-based self-test `*_test.cc` compiled to a binary that exits 0 on success. Integration is verified by the Node smoke (Task 6).
- Game-logic functions (`landlords-server/event/EventFuncs.cc` `CODE_*`, `ServerEventListener`, `ServerContains`, `PokerHelper`, `RobotEventListener`) must NOT be edited except for the two egress helpers (`pushDataToClient`, `pushToClient`) in Task 5.
- JSON contract field names follow spec §6 (normalized names like `sellClientNickname`); tolerate C++ typos (`sellClinetNickname`) by reading both.

## File Structure

New C++ web-layer files (all under `landlords-common/web/`, included via `-I landlords-common`):
- `landlords-common/web/Sha1.h` — SHA-1 (RFC 3174), pure functions.
- `landlords-common/web/Base64.h` — base64 encode (RFC 4648).
- `landlords-common/web/WebSocket.h` — WS handshake + frame parse/build, pure functions on `std::string` (no muduo).
- `landlords-common/web/JsonMapHelper.h` — `MapHelper` ↔ JSON, keyed by event code.
- `landlords-common/web/WsCodec.h` — muduo integration: `onMessage(conn, buf, ts)` + `sendText(conn, json)`, uses `WebSocket.h`.
- `landlords-common/web/*_test.cc` — assert-based self-tests, one binary each.

Existing files modified:
- `landlords-server/server.cc` — swap `ProtobufCodec`/dispatcher for `WsCodec`; ingress path.
- `landlords-server/event/EventFuncs.cc` / `EventFuncs.h` — `pushDataToClient` codec type → `WsCodec*`, sends JSON+WS.
- `landlords-server/event/ServerEventListener.h` — `pushToClient` and `codec_` type → `WsCodec`; `SolveFunc` codec type.
- `Makefile` — add `gateway` target + test targets.

External vendored:
- `lib/json/json.hpp` — nlohmann/json single header.

E2E:
- `web/e2e/gateway-smoke.ts` — Node `ws` client driving one PVE game.

---

### Task 1: Environment ready + nlohmann/json vendored + baseline build green

**Files:**
- Install (apt): `g++`, `libboost-dev`, `libboost-serialization-dev`, `libprotobuf-dev`, `protobuf-compiler`, `zlib1g-dev`
- Create: `lib/json/json.hpp` (download nlohmann/json single header)
- Create: `lib/json/include_check.cc` (compile-only sanity check)

**Interfaces:**
- Produces: a buildable C++ toolchain; `#include "json.hpp"` resolves under `-I lib/json`.

- [ ] **Step 1: Install missing system packages**

Run:
```bash
sudo apt update && sudo apt install -y g++ libboost-dev libboost-serialization-dev libprotobuf-dev protobuf-compiler zlib1g-dev
```
Expected: completes; `g++ --version` prints a version.

- [ ] **Step 2: Verify baseline build of existing server/client compiles**

Run: `make -C /home/san/VsCodeProject/ratel/ratel clean && make -C /home/san/VsCodeProject/ratel/ratel`
Expected: produces `server` and `client` binaries, no errors. (Confirms muduo/boost/protobuf all link.)

- [ ] **Step 3: Vendor nlohmann/json single header**

Run:
```bash
mkdir -p /home/san/VsCodeProject/ratel/ratel/lib/json
curl -fsSL -o /home/san/VsCodeProject/ratel/ratel/lib/json/json.hpp \
  https://raw.githubusercontent.com/nlohmann/json/v3.11.3/single_include/nlohmann/json.hpp
```
Expected: `lib/json/json.hpp` exists (~25000 lines).

- [ ] **Step 4: Write compile-only sanity check**

Create `lib/json/include_check.cc`:
```cpp
#include "json.hpp"
int main() {
    nlohmann::json j;
    j["event"] = "idSet";
    j["data"]["clientId"] = 1;
    return j["data"]["clientId"].get<int>() == 1 ? 0 : 1;
}
```

- [ ] **Step 5: Compile and run the check**

Run:
```bash
g++ -std=c++14 -I /home/san/VsCodeProject/ratel/ratel/lib/json \
  /home/san/VsCodeProject/ratel/ratel/lib/json/include_check.cc -o /tmp/json_check && /tmp/json_check
echo "exit: $?"
```
Expected: `exit: 0`.

- [ ] **Step 6: Commit**

```bash
git -C /home/san/VsCodeProject/ratel/ratel add lib/json/json.hpp lib/json/include_check.cc
git -C /home/san/VsCodeProject/ratel/ratel commit -m "build(gateway): 补齐 C++ 工具链并引入 nlohmann/json"
```

---

### Task 2: SHA-1 + base64 (for WS handshake)

**Files:**
- Create: `landlords-common/web/Sha1.h`
- Create: `landlords-common/web/Base64.h`
- Create: `landlords-common/web/sha1_base64_test.cc`

**Interfaces:**
- Produces: `std::string sha1_hex(const std::string& in)` (lowercase hex); `std::string base64_encode(const uint8_t* data, size_t len)`; `std::string base64_encode(const std::string& in)`.

- [ ] **Step 1: Write the failing self-test**

Create `landlords-common/web/sha1_base64_test.cc`:
```cpp
#include "Sha1.h"
#include "Base64.h"
#include <cassert>
#include <cstdint>
#include <iostream>

int main() {
    // SHA-1 known vectors (RFC 3174 / FIPS)
    assert(sha1_hex("") == "da39a3ee5e6b4b0d3255bfef95601890afd80709");
    assert(sha1_hex("abc") == "a9993e364706816aba3e25717850c26c9cd0d89d");
    assert(sha1_hex("The quick brown fox jumps over the lazy dog") ==
           "2fd4e1c67a2d28fced849ee1bb76e7391b93eb12");

    // base64 known vectors
    assert(base64_encode(std::string("")) == "");
    assert(base64_encode(std::string("f")) == "Zg==");
    assert(base64_encode(std::string("fo")) == "Zm8=");
    assert(base64_encode(std::string("foo")) == "Zm9v");
    assert(base64_encode(std::string("foobar")) == "Zm9vYmFy");

    // RFC 6455 handshake vector: key -> Sec-WebSocket-Accept
    //   base64(sha1(key + magic)) == "s3pPLMBiTxaQ9kYGzzhZRbK+xOo="
    // key is ASCII "dGhlIHNhbXBsZSBub25jZQ=="; sha1 over bytes, then base64 raw.
    // (Tested at the WebSocket layer in Task 3; here we only assert primitives.)

    std::cout << "sha1_base64_test OK\n";
    return 0;
}
```

- [ ] **Step 2: Run test to verify it fails**

Run:
```bash
g++ -std=c++14 -I landlords-common landlords-common/web/sha1_base64_test.cc -o /tmp/sb_test 2>&1 | head -5
```
Expected: FAIL — `Sha1.h: No such file or directory`.

- [ ] **Step 3: Implement Base64.h**

Create `landlords-common/web/Base64.h`:
```cpp
#ifndef LANDLORDS_COMMON_WEB_BASE64_H_
#define LANDLORDS_COMMON_WEB_BASE64_H_
#include <cstdint>
#include <string>

inline static const char* kB64Table =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

inline std::string base64_encode(const uint8_t* data, size_t len) {
    std::string out;
    out.reserve(((len + 2) / 3) * 4);
    for (size_t i = 0; i < len; i += 3) {
        uint32_t n = uint32_t(data[i]) << 16;
        if (i + 1 < len) n |= uint32_t(data[i + 1]) << 8;
        if (i + 2 < len) n |= uint32_t(data[i + 2]);
        out.push_back(kB64Table[(n >> 18) & 0x3F]);
        out.push_back(kB64Table[(n >> 12) & 0x3F]);
        out.push_back(i + 1 < len ? kB64Table[(n >> 6) & 0x3F] : '=');
        out.push_back(i + 2 < len ? kB64Table[n & 0x3F] : '=');
    }
    return out;
}

inline std::string base64_encode(const std::string& in) {
    return base64_encode(reinterpret_cast<const uint8_t*>(in.data()), in.size());
}
#endif
```

- [ ] **Step 4: Implement Sha1.h**

Create `landlords-common/web/Sha1.h`:
```cpp
#ifndef LANDLORDS_COMMON_WEB_SHA1_H_
#define LANDLORDS_COMMON_WEB_SHA1_H_
#include <cstdint>
#include <cstring>
#include <string>

inline uint32_t rotl(uint32_t x, int n) { return (x << n) | (x >> (32 - n)); }

inline std::string sha1_hex(const std::string& input) {
    uint32_t h0 = 0x67452301, h1 = 0xEFCDAB89, h2 = 0x98BADCFE,
             h3 = 0x10325476, h4 = 0xC3D2E1F0;
    uint64_t bitlen = uint64_t(input.size()) * 8;
    std::string msg = input;
    msg.push_back(char(0x80));
    while (msg.size() % 64 != 56) msg.push_back('\0');
    for (int i = 7; i >= 0; --i) msg.push_back(char((bitlen >> (i * 8)) & 0xFF));

    for (size_t chunk = 0; chunk < msg.size(); chunk += 64) {
        uint32_t w[80];
        for (int i = 0; i < 16; ++i) {
            w[i] = (uint8_t(msg[chunk + i * 4]) << 24) |
                   (uint8_t(msg[chunk + i * 4 + 1]) << 16) |
                   (uint8_t(msg[chunk + i * 4 + 2]) << 8) |
                   (uint8_t(msg[chunk + i * 4 + 3]));
        }
        for (int i = 16; i < 80; ++i)
            w[i] = rotl(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);

        uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;
        for (int i = 0; i < 80; ++i) {
            uint32_t f, k;
            if (i < 20)      { f = (b & c) | ((~b) & d); k = 0x5A827999; }
            else if (i < 40) { f = b ^ c ^ d;            k = 0x6ED9EBA1; }
            else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDC; }
            else             { f = b ^ c ^ d;            k = 0xCA62C1D6; }
            uint32_t t = rotl(a, 5) + f + e + k + w[i];
            e = d; d = c; c = rotl(b, 30); b = a; a = t;
        }
        h0 += a; h1 += b; h2 += c; h3 += d; h4 += e;
    }

    static const char* hex = "0123456789abcdef";
    std::string out;
    uint32_t hs[5] = {h0, h1, h2, h3, h4};
    for (int i = 0; i < 5; ++i) {
        out.push_back(hex[(hs[i] >> 28) & 0xF]);
        out.push_back(hex[(hs[i] >> 24) & 0xF]);
        out.push_back(hex[(hs[i] >> 20) & 0xF]);
        out.push_back(hex[(hs[i] >> 16) & 0xF]);
        out.push_back(hex[(hs[i] >> 12) & 0xF]);
        out.push_back(hex[(hs[i] >> 8) & 0xF]);
        out.push_back(hex[(hs[i] >> 4) & 0xF]);
        out.push_back(hex[hs[i] & 0xF]);
    }
    return out;
}
#endif
```

- [ ] **Step 5: Run test to verify it passes**

Run:
```bash
g++ -std=c++14 -I landlords-common landlords-common/web/sha1_base64_test.cc -o /tmp/sb_test && /tmp/sb_test
echo "exit: $?"
```
Expected: prints `sha1_base64_test OK`, `exit: 0`.

- [ ] **Step 6: Commit**

```bash
git add landlords-common/web/Sha1.h landlords-common/web/Base64.h landlords-common/web/sha1_base64_test.cc
git commit -m "feat(gateway): SHA-1 与 base64 实现（WS 握手用）"
```

---

### Task 3: WebSocket handshake + frame parse/build (pure logic)

**Files:**
- Create: `landlords-common/web/WebSocket.h`
- Create: `landlords-common/web/websocket_test.cc`

**Interfaces:**
- Produces:
  - `std::string ws_handshake_accept(const std::string& key)` — RFC 6455 Sec-WebSocket-Accept.
  - `bool ws_parse_handshake_request(const std::string& req, std::string& out_key)` — extracts `Sec-WebSocket-Key`.
  - `bool ws_parse_frame(const std::string& buf, int& out_opcode, std::string& out_payload, size_t& out_consumed)` — parses ONE client→server frame (masked), returns false if incomplete. Handles 7/16/64-bit lengths.
  - `std::string ws_build_text_frame(const std::string& payload)` — server→server frame (unmasked, opcode 0x1).

- [ ] **Step 1: Write the failing self-test**

Create `landlords-common/web/websocket_test.cc`:
```cpp
#include "WebSocket.h"
#include <cassert>
#include <iostream>

int main() {
    // RFC 6455 handshake accept vector
    assert(ws_handshake_accept("dGhlIHNhbXBsZSBub25jZQ==") ==
           "s3pPLMBiTxaQ9kYGzzhZRbK+xOo=");

    // Parse a minimal handshake request, extract key
    std::string req =
        "GET / HTTP/1.1\r\n"
        "Host: 127.0.0.1:8787\r\n"
        "Upgrade: websocket\r\n"
        "Sec-WebSocket-Key: dGhlIHNhbXBsZSBub25jZQ==\r\n"
        "\r\n";
    std::string key;
    assert(ws_parse_handshake_request(req, key));
    assert(key == "dGhlIHNhbXBsZSBub25jZQ==");

    // Build a masked text frame "hi" from client side, then parse it back.
    // fin=1, opcode=1 -> 0x81; masked=1 -> 0x81; len=2 -> 0x82; mask=0x11223344
    std::string masked;
    masked.push_back(char(0x81));      // FIN + text
    masked.push_back(char(0x82));      // MASK + len 2
    masked.push_back(char(0x11));
    masked.push_back(char(0x22));
    masked.push_back(char(0x33));
    masked.push_back(char(0x44));
    uint8_t mask[4] = {0x11, 0x22, 0x33, 0x44};
    const char* data = "hi";
    for (int i = 0; i < 2; ++i)
        masked.push_back(char(uint8_t(data[i]) ^ mask[i % 4]));

    int opcode = 0;
    std::string payload;
    size_t consumed = 0;
    assert(ws_parse_frame(masked, opcode, payload, consumed));
    assert(opcode == 1);
    assert(payload == "hi");
    assert(consumed == masked.size());

    // Server-built text frame is unmasked; round-trip is server framing only,
    // so we assert its shape (FIN|text, no mask bit, correct len).
    std::string frame = ws_build_text_frame("{\"event\":\"idSet\"}");
    assert((uint8_t(frame[0]) & 0x0F) == 0x01);          // opcode text
    assert((uint8_t(frame[1]) & 0x80) == 0x00);          // not masked
    assert((uint8_t(frame[1]) & 0x7F) == 15);            // payload length

    // Incomplete frame must return false (need more bytes)
    std::string stub = masked.substr(0, 3);
    assert(!ws_parse_frame(stub, opcode, payload, consumed));

    std::cout << "websocket_test OK\n";
    return 0;
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `g++ -std=c++14 -I landlords-common landlords-common/web/websocket_test.cc -o /tmp/ws_test 2>&1 | head -5`
Expected: FAIL — `WebSocket.h: No such file or directory`.

- [ ] **Step 3: Implement WebSocket.h**

Create `landlords-common/web/WebSocket.h`:
```cpp
#ifndef LANDLORDS_COMMON_WEB_WEBSOCKET_H_
#define LANDLORDS_COMMON_WEB_WEBSOCKET_H_
#include "Sha1.h"
#include "Base64.h"
#include <cstdint>
#include <string>

static const std::string WS_MAGIC = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

inline std::string ws_handshake_accept(const std::string& key) {
    // sha1 over raw bytes of (key + magic), then base64 the 20 raw digest bytes.
    std::string digest_hex = sha1_hex(key + WS_MAGIC);
    std::string raw;
    raw.reserve(20);
    for (size_t i = 0; i < digest_hex.size(); i += 2) {
        auto hexval = [](char c) -> int {
            if (c >= '0' && c <= '9') return c - '0';
            if (c >= 'a' && c <= 'f') return c - 'a' + 10;
            return 0;
        };
        raw.push_back(char((hexval(digest_hex[i]) << 4) | hexval(digest_hex[i + 1])));
    }
    return base64_encode(raw);
}

inline bool ws_parse_handshake_request(const std::string& req, std::string& out_key) {
    std::string needle = "Sec-WebSocket-Key:";
    auto p = req.find(needle);
    if (p == std::string::npos) {
        needle = "Sec-WebSocket-Key: ";  // already with space? fallback handled below
        p = req.find("Sec-WebSocket-Key");
        if (p == std::string::npos) return false;
    }
    p = req.find(':', p) + 1;
    while (p < req.size() && (req[p] == ' ' || req[p] == '\t')) ++p;
    auto end = req.find("\r\n", p);
    if (end == std::string::npos) return false;
    out_key = req.substr(p, end - p);
    return !out_key.empty();
}

inline bool ws_parse_frame(const std::string& buf, int& out_opcode,
                           std::string& out_payload, size_t& out_consumed) {
    if (buf.size() < 2) return false;
    int opcode = uint8_t(buf[0]) & 0x0F;
    bool masked = (uint8_t(buf[1]) & 0x80) != 0;
    uint64_t len = uint8_t(buf[1]) & 0x7F;
    size_t idx = 2;
    if (len == 126) {
        if (buf.size() < 4) return false;
        len = (uint64_t(uint8_t(buf[2])) << 8) | uint8_t(buf[3]);
        idx = 4;
    } else if (len == 127) {
        if (buf.size() < 10) return false;
        len = 0;
        for (int i = 0; i < 8; ++i) len = (len << 8) | uint8_t(buf[2 + i]);
        idx = 10;
    }
    uint8_t mask[4] = {0, 0, 0, 0};
    if (masked) {
        if (buf.size() < idx + 4) return false;
        for (int i = 0; i < 4; ++i) mask[i] = uint8_t(buf[idx + i]);
        idx += 4;
    }
    if (buf.size() < idx + len) return false;
    out_payload.clear();
    out_payload.reserve(size_t(len));
    for (uint64_t i = 0; i < len; ++i)
        out_payload.push_back(char(uint8_t(buf[idx + i]) ^ mask[i % 4]));
    out_opcode = opcode;
    out_consumed = size_t(idx + len);
    return true;
}

inline std::string ws_build_text_frame(const std::string& payload) {
    std::string frame;
    frame.push_back(char(0x81));  // FIN + text
    size_t len = payload.size();
    if (len <= 125) {
        frame.push_back(char(len));
    } else if (len <= 65535) {
        frame.push_back(char(126));
        frame.push_back(char((len >> 8) & 0xFF));
        frame.push_back(char(len & 0xFF));
    } else {
        frame.push_back(char(127));
        for (int i = 7; i >= 0; --i) frame.push_back(char((len >> (i * 8)) & 0xFF));
    }
    frame.append(payload);
    return frame;
}
#endif
```

- [ ] **Step 4: Run test to verify it passes**

Run: `g++ -std=c++14 -I landlords-common landlords-common/web/websocket_test.cc -o /tmp/ws_test && /tmp/ws_test`
Expected: prints `websocket_test OK`, exit 0.

- [ ] **Step 5: Commit**

```bash
git add landlords-common/web/WebSocket.h landlords-common/web/websocket_test.cc
git commit -m "feat(gateway): WebSocket 握手与帧解析/封装（纯逻辑）"
```

---

### Task 4: JsonMapHelper (MapHelper ↔ JSON)

**Files:**
- Create: `landlords-common/web/JsonMapHelper.h`
- Create: `landlords-common/web/jsonmap_test.cc`

**Interfaces:**
- Consumes: `MapHelper` (`landlords-common/helper/SerializeHelper.h`), `Poker` (`entity/Poker.h`), enums `ClientEventCode`/`ServerEventCode`, nlohmann/json (`-I lib/json`).
- Produces:
  - `nlohmann::json to_event_json(ClientEventCode code, const MapHelper& m)` — returns `{"event": <name>, "data": {...}}` per spec §6.2.
  - `MapHelper from_event_json(const nlohmann::json& j)` — reads `event` + `data`, fills `MapHelper` per spec §6.1, returns it; caller maps event name → `ServerEventCode`.

> Mapping is keyed by event code (spec §7.2 rationale): enumerate fields per event rather than flatten all 6 maps. Poker → `{"level","type"}`. ClientInfo → `{"clientId","nickname","type","cardsLeft","position"}`.

- [ ] **Step 1: Write the failing self-test**

Create `landlords-common/web/jsonmap_test.cc`:
```cpp
#include "JsonMapHelper.h"
#include <cassert>
#include <iostream>
#include "json.hpp"

int main() {
    using nlohmann::json;

    // --- egress: GAME_ID_SET {clientId} ---
    MapHelper idm;
    idm.put("clientId", 7);
    json j = to_event_json(ClientEventCode::CODE_GAME_ID_SET, idm);
    assert(j["event"] == "idSet");
    assert(j["data"]["clientId"] == 7);

    // --- egress: SHOW_POKERS carries pokers (someone's played cards) ---
    MapHelper sp;
    std::vector<Poker> played = { Poker(PokerType::HEART, PokerLevel::LEVEL_3),
                                  Poker(PokerType::SPADE, PokerLevel::LEVEL_3) };
    sp.put("pokers", played);
    sp.put("clientId", 7);
    sp.put("clientNickname", "san");
    sp.put("clientType", 0);
    json js = to_event_json(ClientEventCode::CODE_SHOW_POKERS, sp);
    assert(js["event"] == "showPokers");
    assert(js["data"]["pokers"].size() == 2);
    assert(js["data"]["pokers"][0]["level"] == int(PokerLevel::LEVEL_3));
    assert(js["data"]["pokers"][0]["type"] == int(PokerType::HEART));

    // --- ingress: play {pokers:[{level,type}]} -> MapHelper.options=[levels] ---
    json in = { {"event", "play"},
                {"data", { {"pokers", { {"level", 3}, {"type", 1} }} }} };
    MapHelper m = from_event_json(in);
    // from_event_json keys options by level list; verify it read clientId-free input
    auto options = m.get("options", std::vector<PokerLevel>());
    assert(options.size() == 1 && options[0] == PokerLevel::LEVEL_3);

    // --- ingress: landlordElect {grab:true} -> MapHelper.is_Y == "true" ---
    json grab = { {"event", "landlordElect"}, {"data", { {"grab", true} }} };
    MapHelper mg = from_event_json(grab);
    assert(mg.get("is_Y", std::string("")) == "true");

    std::cout << "jsonmap_test OK\n";
    return 0;
}
```

- [ ] **Step 2: Run test to verify it fails**

Run: `g++ -std=c++14 -I landlords-common -I lib/json landlords-common/web/jsonmap_test.cc -o /tmp/jm_test 2>&1 | head -5`
Expected: FAIL — `JsonMapHelper.h: No such file or directory`.

- [ ] **Step 3: Implement JsonMapHelper.h**

Create `landlords-common/web/JsonMapHelper.h`:
```cpp
#ifndef LANDLORDS_COMMON_WEB_JSONMAPHELPER_H_
#define LANDLORDS_COMMON_WEB_JSONMAPHELPER_H_
#include "json.hpp"
#include "helper/SerializeHelper.h"
#include "entity/Poker.h"
#include "enums/ClientEventCode.h"
#include "enums/ServerEventCode.h"
#include <string>

using nlohmann::json;

inline static json poker_to_json(const Poker& p) {
    return { {"level", p.getLevel()}, {"type", p.getType()} };
}
inline static json pokers_to_json(const std::vector<Poker>& v) {
    json arr = json::array();
    for (const auto& p : v) arr.push_back(poker_to_json(p));
    return arr;
}

// Egress: server -> frontend, per spec §6.2
inline json to_event_json(ClientEventCode code, const MapHelper& m) {
    json data = json::object();
    switch (code) {
        case ClientEventCode::CODE_GAME_ID_SET:
            data["clientId"] = m.get("clientId", 0);
            return { {"event", "idSet"}, {"data", data} };
        case ClientEventCode::CODE_GAME_STARTING:
            data["pokers"] = pokers_to_json(m.get("pokers", std::vector<Poker>()));
            data["clientId"] = m.get("clientId", 0);
            data["nextClientId"] = m.get("nextClientId", 0);
            data["nextClientNickname"] = m.get("nextClientNickname", std::string());
            data["roomOwner"] = m.get("roomOwner", std::string());
            data["roomClientCount"] = m.get("roomClientCount", 0);
            return { {"event", "gameStarting"}, {"data", data} };
        case ClientEventCode::CODE_GAME_LANDLORD_ELECT:
            data["nextClientId"] = m.get("nextClientId", 0);
            data["nextClientNickname"] = m.get("nextClientNickname", std::string());
            data["preClientNickname"] = m.get("preClientNickname", std::string());
            return { {"event", "landlordElect"}, {"data", data} };
        case ClientEventCode::CODE_GAME_LANDLORD_CONFIRM:
            data["landlordId"] = m.get("landlordId", 0);
            data["landlordNickname"] = m.get("landlordNickname", std::string());
            data["additionalPokers"] = pokers_to_json(m.get("additionalPokers", std::vector<Poker>()));
            return { {"event", "landlordConfirm"}, {"data", data} };
        case ClientEventCode::CODE_SHOW_POKERS:
            data["pokers"] = pokers_to_json(m.get("pokers", std::vector<Poker>()));
            data["clientId"] = m.get("clientId", 0);
            data["clientNickname"] = m.get("clientNickname", std::string());
            data["clientType"] = m.get("clientType", 0);
            return { {"event", "showPokers"}, {"data", data} };
        case ClientEventCode::CODE_GAME_POKER_PLAY_REDIRECT: {
            data["pokers"] = pokers_to_json(m.get("pokers", std::vector<Poker>()));
            data["lastSellPokers"] = pokers_to_json(m.get("lastSellPokers", std::vector<Poker>()));
            data["lastSellClientId"] = m.get("lastSellClientId", 0);
            data["sellClientId"] = m.get("sellClientId", 0);
            // tolerate C++ typo sellClinetNickname
            data["sellClientNickname"] = m.get("sellClinetNickname", std::string());
            json seats = json::array();
            for (const auto& ci : m.get("clientInfos", std::vector<ClientInfo>())) {
                seats.push_back({ {"clientId", ci.clientId},
                                  {"nickname", ci.clientNickname},
                                  {"type", int(ci.type)},
                                  {"cardsLeft", ci.surplus},
                                  {"position", ci.position} });
            }
            data["clientInfos"] = seats;
            return { {"event", "playRedirect"}, {"data", data} };
        }
        case ClientEventCode::CODE_GAME_POKER_PLAY_PASS:
            data["clientId"] = m.get("clientId", 0);
            data["clientNickname"] = m.get("clientNickname", std::string());
            data["nextClientId"] = m.get("nextClientId", 0);
            data["nextClientNickname"] = m.get("nextClientNickname", std::string());
            return { {"event", "playPass"}, {"data", data} };
        case ClientEventCode::CODE_GAME_POKER_PLAY_MISMATCH:
            return { {"event", "playError"}, {"data", { {"code", "mismatch"} }} };
        case ClientEventCode::CODE_GAME_POKER_PLAY_LESS:
            return { {"event", "playError"}, {"data", { {"code", "less"} }} };
        case ClientEventCode::CODE_GAME_POKER_PLAY_INVALID:
            return { {"event", "playError"}, {"data", { {"code", "invalid"} }} };
        case ClientEventCode::CODE_GAME_POKER_PLAY_ORDER_ERROR:
            return { {"event", "playError"}, {"data", { {"code", "order"} }} };
        case ClientEventCode::CODE_GAME_POKER_PLAY_CANT_PASS:
            return { {"event", "playError"}, {"data", { {"code", "cantPass"} }} };
        case ClientEventCode::CODE_GAME_OVER:
            data["winnerNickname"] = m.get("winnerNickname", std::string());
            data["winnerType"] = m.get("winnerType", std::string());
            return { {"event", "gameOver"}, {"data", data} };
        default:
            return { {"event", "unknown"}, {"data", data} };
    }
}

// Ingress: frontend -> server, per spec §6.1. Returns MapHelper; caller resolves
// event name -> ServerEventCode. Only normalizes the well-known client events.
inline MapHelper from_event_json(const json& j) {
    MapHelper m;
    std::string ev = j.value("event", "");
    const json& d = j.value("data", json::object());
    if (ev == "setNickname") {
        m.put("nickName", d.value("nickname", std::string()));
    } else if (ev == "createRoomPve") {
        m.put("choose", std::string("0"));  // difficulty default simple
    } else if (ev == "landlordElect") {
        m.put("is_Y", d.value("grab", false) ? "true" : "false");
    } else if (ev == "play") {
        std::vector<PokerLevel> options;
        if (d.contains("pokers") && d["pokers"].is_array()) {
            for (const auto& c : d["pokers"]) {
                options.push_back(PokerLevel(c.value("level", 0)));
            }
        }
        m.put("options", options);
    } else if (ev == "pass") {
        // no fields
    }
    return m;
}

// Map a frontend event name to ServerEventCode (used by server.cc ingress).
inline ServerEventCode event_name_to_server_code(const std::string& ev) {
    if (ev == "setNickname")     return ServerEventCode::CODE_CLIENT_NICKNAME_SET;
    if (ev == "createRoomPve")   return ServerEventCode::CODE_ROOM_CREATE_PVE;
    if (ev == "landlordElect")   return ServerEventCode::CODE_GAME_LANDLORD_ELECT;
    if (ev == "play")            return ServerEventCode::CODE_GAME_POKER_PLAY;
    if (ev == "pass")            return ServerEventCode::CODE_GAME_POKER_PLAY_PASS;
    if (ev == "exit")            return ServerEventCode::CODE_CLIENT_EXIT;
    return ServerEventCode::CODE_CLIENT_EXIT;  // safe fallback
}
#endif
```

- [ ] **Step 4: Run test to verify it passes**

Run: `g++ -std=c++14 -I landlords-common -I lib/json landlords-common/web/jsonmap_test.cc -o /tmp/jm_test && /tmp/jm_test`
Expected: prints `jsonmap_test OK`, exit 0.

- [ ] **Step 5: Commit**

```bash
git add landlords-common/web/JsonMapHelper.h landlords-common/web/jsonmap_test.cc
git commit -m "feat(gateway): MapHelper↔JSON 转换（按事件枚举字段）"
```

---

### Task 5: WsCodec (muduo integration) + server.cc ingress + egress rewire + Makefile

**Files:**
- Create: `landlords-common/web/WsCodec.h`
- Modify: `landlords-server/server.cc` (replace `ProtobufCodec`/dispatcher; ingress path)
- Modify: `landlords-server/event/ServerEventListener.h` (`pushToClient`, `codec_`, `SolveFunc` → `WsCodec*`)
- Modify: `landlords-server/event/EventFuncs.h` + `landlords-server/event/EventFuncs.cc` (`pushDataToClient` → `WsCodec*`, JSON+WS send)
- Modify: `Makefile` (add `gateway` target + test targets; drop `-lprotobuf` from gateway link)

**Interfaces:**
- Consumes: `WebSocket.h`, `JsonMapHelper.h`, muduo `Buffer`/`TcpConnection`, the unchanged game-logic functions.
- Produces: a `gateway` binary listening on WS, speaking JSON; `server`/`client` may still build for transition.

> Design note (spec §7.4, §12): keep the `codec` parameter on game-logic functions but change its type from `ProtobufCodec*` to `WsCodec*`. `WsCodec` owns the JSON+WS send path, so `pushDataToClient(codec, conn, code, map)` becomes `codec->sendEvent(conn, code, map)`.

- [ ] **Step 1: Implement WsCodec.h (muduo integration)**

Create `landlords-common/web/WsCodec.h`:
```cpp
#ifndef LANDLORDS_COMMON_WEB_WSCODEC_H_
#define LANDLORDS_COMMON_WEB_WSCODEC_H_
#include "WebSocket.h"
#include "JsonMapHelper.h"
#include "muduo/net/Buffer.h"
#include "muduo/net/TcpConnection.h"
#include "muduo/net/Timestamp.h"
#include <functional>
#include <string>

// One of these lives per-connection, tracking handshake + leftover bytes.
struct WsConnState {
    bool handshakeDone = false;
    std::string leftover;
};

class WsCodec {
 public:
  // Called when a complete JSON text frame arrives.
  typedef std::function<void(const muduo::net::TcpConnectionPtr&,
                             const std::string& jsonText)> JsonMessageCallback;

  explicit WsCodec(JsonMessageCallback cb) : cb_(std::move(cb)) {}

  // muduo message callback. Accumulates bytes, does handshake once, then
  // parses frames and invokes cb_ with each text frame's payload.
  void onMessage(const muduo::net::TcpConnectionPtr& conn,
                 muduo::net::Buffer* buf,
                 muduo::net::Timestamp) {
      auto it = states_.find(conn.get());
      if (it == states_.end()) {
          it = states_.emplace(conn.get(), WsConnState{}).first;
      }
      WsConnState& st = it->second;
      st.leftover.append(buf->retrieveAllAsString());

      if (!st.handshakeDone) {
          auto end = st.leftover.find("\r\n\r\n");
          if (end == std::string::npos) return;  // wait for full headers
          std::string req = st.leftover.substr(0, end);
          std::string key;
          if (!ws_parse_handshake_request(req, key)) { conn->shutdown(); return; }
          std::string accept = ws_handshake_accept(key);
          std::string resp =
              "HTTP/1.1 101 Switching Protocols\r\n"
              "Upgrade: websocket\r\n"
              "Connection: Upgrade\r\n"
              "Sec-WebSocket-Accept: " + accept + "\r\n\r\n";
          conn->send(resp);
          st.leftover.erase(0, end + 4);
          st.handshakeDone = true;
      }

      // Drain complete frames.
      while (!st.leftover.empty()) {
          int opcode = 0;
          std::string payload;
          size_t consumed = 0;
          if (!ws_parse_frame(st.leftover, opcode, payload, consumed)) break;
          if (opcode == 0x8) { conn->shutdown(); break; }       // close
          if (opcode == 0x9) {                                  // ping -> pong
              conn->send(ws_build_pong(payload));
          } else if (opcode == 0x1) {                           // text
              cb_(conn, payload);
          }
          st.leftover.erase(0, consumed);
      }
  }

  // Egress: turn (code, MapHelper) into a JSON text frame and send.
  void sendEvent(const muduo::net::TcpConnectionPtr& conn,
                 ClientEventCode code, const MapHelper& m) {
      std::string jsonText = to_event_json(code, m).dump();
      conn->send(ws_build_text_frame(jsonText));
  }

  void discard(const muduo::net::TcpConnectionPtr& conn) {
      states_.erase(conn.get());
  }

 private:
  static std::string ws_build_pong(const std::string& payload) {
      std::string f;
      f.push_back(char(0x8A));  // FIN + pong
      f.push_back(char(payload.size() & 0x7F));
      f.append(payload);
      return f;
  }
  JsonMessageCallback cb_;
  std::unordered_map<const void*, WsConnState> states_;
};
#endif
```
Add `#include <unordered_map>` at top if not pulled by others.

- [ ] **Step 2: Rewire egress — `ServerEventListener.h`**

In `landlords-server/event/ServerEventListener.h`:
- Replace `#include "protobuf/codec.h"` / `dispatcher.h` / `query.pb.h` with `#include "web/WsCodec.h"`.
- Change `SolveFunc` typedef's first param `ProtobufCodec*` → `WsCodec*`.
- Change member `ProtobufCodec codec_;` → `WsCodec codec_;` (construct as `codec_(std::bind(&ServerEventListener::onJson, this, _1, _2))` — see below) and remove `ProtobufDispatcher dispatcher_;`.
- Replace `pushToClient` body:
```cpp
  void pushToClient(const muduo::net::TcpConnectionPtr &conn,
             ClientEventCode code, const char * /*data*/ ) {
      codec_.sendEvent(conn, code, MapHelper());
  }
```
- Add a private `onJson` stub that is the ingress dispatcher target (server.cc will own real dispatch; this default forwards to the listener's operator()).

> Concretely, `ServerEventListener::operator()` currently calls `(*answerFunc)(&codec_, conn, mapHelper)` — keep that call site, only `codec_`'s type changed.

- [ ] **Step 3: Rewire egress — `EventFuncs.cc` / `EventFuncs.h`**

In `EventFuncs.h` change every `ServerEventListener_CODE_*` and `pushDataToClient` declaration's first param `ProtobufCodec*` → `WsCodec*`.
In `EventFuncs.cc` change `pushDataToClient` body from:
```cpp
std::string result = SerializeHelper::SerializeToString<MapHelper>(mapHelper);
muduo::Answer answer;
answer.set_answerer("san"); answer.set_questioner("san");
answer.set_id(int(code)); answer.add_solution(result);
codec->send(conn, answer);
```
to:
```cpp
codec->sendEvent(conn, code, mapHelper);
```
Remove now-unused `#include "protobuf/query.pb.h"` if it was only for `Answer`. Update the `robot_elect_landlord` forward decl's `ProtobufCodec*` → `WsCodec*` too. Update `#include "protobuf/codec.h"` → `#include "web/WsCodec.h"` where the type is used.

- [ ] **Step 4: Rewire ingress — `server.cc`**

Replace the `ProtobufCodec`/`ProtobufDispatcher` usage in `QueryServer`:
- Member: `WsCodec codec_;` constructed with a callback that parses JSON and dispatches:
```cpp
: server_(loop, listenAddr, "QueryServer"),
  codec_(std::bind(&QueryServer::onWsMessage, this, _1, _2)),
  serverEventListener()
{
    server_.setConnectionCallback(std::bind(&QueryServer::onConnection, this, _1));
    server_.setMessageCallback(std::bind(&WsCodec::onMessage, &codec_, _1, _2, _3));
}
```
- Add `onWsMessage`:
```cpp
void onWsMessage(const muduo::net::TcpConnectionPtr& conn, const std::string& text) {
    nlohmann::json j;
    try { j = nlohmann::json::parse(text); }
    catch (...) { LOG_WARN << "bad json frame"; return; }
    ServerEventCode code = event_name_to_server_code(j.value("event", ""));
    MapHelper data = from_event_json(j);
    serverEventListener(conn, code, data);
}
```
- `onConnection`: keep clientId allocation; the existing `pushToClient(conn, CODE_GAME_ID_SET, ...)` call now goes out as JSON+WS via the rewired `pushToClient`. Add `codec_.discard(conn)` on disconnect.
- Add `#include "web/WsCodec.h"`, `#include "web/JsonMapHelper.h"`, `#include "json.hpp"`. Remove Protobuf includes.
- `main`: keep `ip port` args (run as `./gateway 127.0.0.1 8787`).

- [ ] **Step 5: Update Makefile**

Add a `gateway` target that compiles `server.cc` (now WS-based) WITHOUT the protobuf sources, and links without `-lprotobuf`/`-lboost_serialization` once confirmed unneeded. Minimal diff:
```make
GATEWAY_SRC = landlords-server/server.cc \
              landlords-server/event/EventFuncs.cc \
              landlords-server/event/ServerEventListener.cc \
              landlords-server/event/ServerContains.cc \
              landlords-server/robot/RobotEventListener.cc \
              landlords-server/robot/RobotEventFuncs.cc \
              landlords-common/helper/PokerHelper.cc \
              landlords-common/entity/PokerSell.cc \
              landlords-common/robot/RobotDecisionMakers.cc

GATEWAY_CXXFLAGS = -g -std=c++14 -I lib/muduo/include -I landlords-common -I lib/json -pthread -L lib/muduo/lib
GATEWAY_LDFLAGS = -L lib/muduo/lib -lmuduo_net -lmuduo_base -lpthread -lz

gateway: $(GATEWAY_SRC)
	g++ $(GATEWAY_CXXFLAGS) -o gateway $(GATEWAY_SRC) $(GATEWAY_LDFLAGS)

test: sha1_base64_test websocket_test jsonmap_test
sha1_base64_test: ; g++ $(GATEWAY_CXXFLAGS) landlords-common/web/sha1_base64_test.cc -o /tmp/$@ && /tmp/$@
websocket_test:   ; g++ $(GATEWAY_CXXFLAGS) landlords-common/web/websocket_test.cc   -o /tmp/$@ && /tmp/$@
jsonmap_test:     ; g++ $(GATEWAY_CXXFLAGS) landlords-common/web/jsonmap_test.cc       -o /tmp/$@ && /tmp/$@
```
(Keep the existing `server`/`client`/`libravel.a` rules intact for transition.)

- [ ] **Step 6: Build gateway and run self-tests**

Run: `make -C /home/san/VsCodeProject/ratel/ratel test gateway 2>&1 | tail -20`
Expected: three test binaries print OK; `gateway` binary produced; no errors.

- [ ] **Step 7: Smoke-start the gateway**

Run:
```bash
/home/san/VsCodeProject/ratel/ratel/gateway 127.0.0.1 8787 &
sleep 0.5
echo "gateway pid: $!"
```
Expected: process stays up (no crash). Kill it after Task 6 (`kill %1`).

- [ ] **Step 8: Commit**

```bash
git add landlords-common/web/WsCodec.h landlords-server/server.cc \
        landlords-server/event/ServerEventListener.h landlords-server/event/EventFuncs.cc \
        landlords-server/event/EventFuncs.h Makefile
git commit -m "feat(gateway): WsCodec 集成 muduo 并改造 server 入口/出口为 WS+JSON"
```

---

### Task 6: Node e2e smoke — drive one PVE game through the gateway

**Files:**
- Create: `web/e2e/gateway-smoke.ts`
- Modify: `web/package.json` (add `"smoke": "tsx e2e/gateway-smoke.ts"` script; `tsx` already a dev tool via mock-server)

**Interfaces:**
- Consumes: a running `gateway` binary on `ws://127.0.0.1:8787`.
- Produces: pass/fail exit asserting the event sequence for one full PVE game.

- [ ] **Step 1: Write the smoke client**

Create `web/e2e/gateway-smoke.ts`:
```ts
import WebSocket from 'ws';

const URL = process.env.GATEWAY_URL ?? 'ws://127.0.0.1:8787';

type Msg = { event: string; data: Record<string, unknown> };

const seen: string[] = [];
let ws: WebSocket;
let resolveDone: () => void;
const done = new Promise<void>((r) => (resolveDone = r));

function send(event: string, data: Record<string, unknown> = {}) {
  ws.send(JSON.stringify({ event, data }));
}

function start() {
  ws = new WebSocket(URL);
  ws.on('message', (raw) => {
    let msg: Msg;
    try { msg = JSON.parse(raw.toString()); } catch { return; }
    seen.push(msg.event);
    console.log('<-', msg.event, JSON.stringify(msg.data).slice(0, 120));

    if (msg.event === 'idSet') { send('setNickname', { nickname: 'san' }); return; }
    // After nickname the server pushes showOptions; request PVE room:
    if (msg.event === 'showOptions' || msg.event === 'nicknameSet') {
      send('createRoomPve', {}); return;
    }
    if (msg.event === 'landlordElect') { send('landlordElect', { grab: true }); return; }
    if (msg.event === 'playRedirect' || msg.event === 'playTurn') {
      // Play the lowest single card we hold, else pass.
      const hand = (msg.data.pokers as { level: number; type: number }[]) ?? [];
      if (hand.length) send('play', { pokers: [{ level: hand[0].level, type: hand[0].type }] });
      else send('pass');
      return;
    }
    if (msg.event === 'gameOver') { resolveDone(); }
  });
  ws.on('error', (e) => { console.error('ws error', e); process.exit(1); });
}

start();
const timer = setTimeout(() => { console.error('TIMEOUT'); process.exit(1); }, 20000);
(async () => {
  await done;
  clearTimeout(timer);
  ws.close();
  const want = ['idSet', 'gameStarting', 'landlordConfirm', 'gameOver'];
  const ok = want.every((w) => seen.includes(w));
  console.log('\nseen:', seen.join(', '));
  console.log(ok ? 'SMOKE PASS' : 'SMOKE FAIL');
  process.exit(ok ? 0 : 1);
})();
```

- [ ] **Step 2: Add the script to package.json**

In `web/package.json` scripts add:
```json
"smoke": "tsx e2e/gateway-smoke.ts"
```
Ensure `ws` and `tsx` are deps/devDeps (mock-server uses them; add to `web` devDependencies if missing: `npm i -D ws tsx @types/ws` in `web/`).

- [ ] **Step 3: Run the smoke against the live gateway**

In one shell start the gateway: `./gateway 127.0.0.1 8787` (repo root).
Then run:
```bash
cd /home/san/VsCodeProject/ratel/ratel/web && npm run smoke 2>&1 | tail -30
```
Expected: prints `<-` event lines, ending with `SMOKE PASS` and exit 0. The sequence must include `idSet`, `gameStarting`, `landlordConfirm`, and `gameOver`.

- [ ] **Step 4: If a field/event mismatch appears, fix in JsonMapHelper.h and re-run**

Iterate on `to_event_json` / `from_event_json` until the smoke passes. Do NOT edit game-logic functions. Re-run `make test gateway` then the smoke.

- [ ] **Step 5: Commit**

```bash
git add web/e2e/gateway-smoke.ts web/package.json web/package-lock.json
git commit -m "test(gateway): Node e2e 冒烟脚本驱动一局 PVE"
```

---

## Self-Review

**1. Spec coverage** (spec §1–§13):
- §4.1 改造方案 → Tasks 5 (ingress/egress) ✓
- §5 架构 WsCodec/JsonMapHelper → Tasks 3/4/5 ✓
- §6 契约表 → Task 4 (JsonMapHelper) + Task 6 (smoke asserts) ✓
- §7.1 WsCodec 握手+帧 → Task 3 (pure) + Task 5 (muduo) ✓
- §7.2 JsonMapHelper → Task 4 ✓
- §7.3 server.cc 改造 → Task 5 ✓
- §7.4 pushDataToClient 改造 → Task 5 ✓
- §10 依赖与构建 → Task 1 + Task 5 (Makefile) ✓
- §11 测试策略 (assert 自测 + Node 冒烟) → Tasks 2/3/4/6 ✓
- §13 验收 1–2 → Tasks 5/6 ✓ (验收 3–4 前端属于 Plan B)

**2. Placeholder scan:** No TBD/TODO/"handle edge cases". Task 5 §7.4 codec-type migration is spelled out (keep param, change type to `WsCodec*`). All code blocks contain real code.

**3. Type consistency:** `sendEvent(conn, code, map)` used in `WsCodec` (Task 5 Step 1) matches the egress rewrite in Steps 2–3. `to_event_json`/`from_event_json`/`event_name_to_server_code` names consistent across Tasks 4–6. `ws_build_text_frame`/`ws_parse_frame`/`ws_handshake_accept` consistent across Tasks 3–5.

**Open follow-up (Plan B, frontend):** §9 frontend changes (types.ts, gameReducer, CardTable via clientInfos, useGame/LobbyView flow), §13 验收 3–4 (frontend test/build green + manual one-game).

## Execution Handoff

Plan complete and saved to `docs/superpowers/plans/2026-06-24-cpp-gateway.md`. Two execution options:

1. **Subagent-Driven (recommended)** — I dispatch a fresh subagent per task, review between tasks, fast iteration.
2. **Inline Execution** — Execute tasks in this session using executing-plans, batch execution with checkpoints.

Which approach?
