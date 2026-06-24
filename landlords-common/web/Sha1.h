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
