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
