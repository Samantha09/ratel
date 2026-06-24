#ifndef LANDLORDS_COMMON_WEB_WSCODEC_H_
#define LANDLORDS_COMMON_WEB_WSCODEC_H_
#include "WebSocket.h"
#include "JsonMapHelper.h"
#include "muduo/net/Buffer.h"
#include "muduo/net/TcpConnection.h"
#include "muduo/base/Timestamp.h"
#include <functional>
#include <string>
#include <unordered_map>

using muduo::net::TcpConnectionPtr;
using muduo::net::Buffer;
using muduo::Timestamp;

// One of these lives per-connection, tracking handshake + leftover bytes.
struct WsConnState {
    bool handshakeDone = false;
    std::string leftover;
};

class WsCodec {
 public:
  // Called when a complete JSON text frame arrives.
  typedef std::function<void(const TcpConnectionPtr&,
                             const std::string& jsonText)> JsonMessageCallback;
  // Called when WebSocket handshake completes.
  typedef std::function<void(const TcpConnectionPtr&)> HandshakeCallback;

  WsCodec(JsonMessageCallback jsonCb, HandshakeCallback hsCb = nullptr)
      : jsonCb_(std::move(jsonCb)), hsCb_(std::move(hsCb)) {}

  // muduo message callback. Accumulates bytes, does handshake once, then
  // parses frames and invokes cb_ with each text frame's payload.
  void onMessage(const TcpConnectionPtr& conn,
                 Buffer* buf,
                 Timestamp) {
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
          if (hsCb_) hsCb_(conn);
          return;  // handshake done, next message will process frames
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
              jsonCb_(conn, payload);
          }
          st.leftover.erase(0, consumed);
      }
  }

  // Egress: turn (code, MapHelper) into a JSON text frame and send.
  void sendEvent(const TcpConnectionPtr& conn,
                 ClientEventCode code, const MapHelper& m) {
      std::string jsonText = to_event_json(code, m).dump();
      conn->send(ws_build_text_frame(jsonText));
  }

  void discard(const TcpConnectionPtr& conn) {
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
  JsonMessageCallback jsonCb_;
  HandshakeCallback hsCb_;
  std::unordered_map<const void*, WsConnState> states_;
};
#endif
