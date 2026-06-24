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
