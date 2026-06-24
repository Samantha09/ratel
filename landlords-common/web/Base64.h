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
