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
    assert((uint8_t(frame[1]) & 0x7F) == 17);            // payload length

    // Incomplete frame must return false (need more bytes)
    std::string stub = masked.substr(0, 3);
    assert(!ws_parse_frame(stub, opcode, payload, consumed));

    std::cout << "websocket_test OK\n";
    return 0;
}
