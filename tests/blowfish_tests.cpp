#include <boost/test/unit_test.hpp>

#include <server_clib/blowfish.h>
#include <iostream>
#include <algorithm>

namespace server_clib {

BOOST_AUTO_TEST_SUITE(blowfish_tests)

BOOST_AUTO_TEST_CASE(encryption_base_author_check)
{
    uint32_t L = 1, R = 2;
    blowfish_ctx_t ctx;

    blowfish_init(&ctx, (uint8_t*)"TESTKEY", 7);
    blowfish_encrypt_chunk(&ctx, &L, &R);
    BOOST_REQUIRE(L == 0xDF333FD2L);
    BOOST_REQUIRE(R == 0x30A71BB4L);

    blowfish_decrypt_chunk(&ctx, &L, &R);
    BOOST_REQUIRE(L == 1);
    BOOST_REQUIRE(R == 2);
}

BOOST_AUTO_TEST_CASE(chunk_encryption_check)
{
    const uint32_t initial_L = 12;
    const uint32_t initial_R = 31;

    uint32_t L = initial_L, R = initial_R;
    blowfish_ctx_t ctx;

    char key[] = "password";
    char wrong_key[] = "PASSWORD";

    BOOST_REQUIRE(blowfish_init(&ctx, (uint8_t*)key, sizeof(key)));
    BOOST_REQUIRE(blowfish_encrypt_chunk(&ctx, &L, &R));
    BOOST_REQUIRE(blowfish_destroy(&ctx));
    BOOST_REQUIRE_EQUAL(ctx.P[0], 0);
    BOOST_REQUIRE_EQUAL(ctx.S[0][0], 0);

    uint32_t enc_L = L, enc_R = R;

    BOOST_REQUIRE(blowfish_init(&ctx, (uint8_t*)wrong_key, sizeof(wrong_key)));
    BOOST_REQUIRE(blowfish_decrypt_chunk(&ctx, &L, &R));

    BOOST_REQUIRE_NE(L, initial_L);
    BOOST_REQUIRE_NE(R, initial_R);

    L = enc_L;
    R = enc_R;

    BOOST_REQUIRE(blowfish_init(&ctx, (uint8_t*)key, sizeof(key)));
    BOOST_REQUIRE(blowfish_decrypt_chunk(&ctx, &L, &R));
    BOOST_REQUIRE(blowfish_destroy(&ctx));
    BOOST_REQUIRE_EQUAL(ctx.P[0], 0);
    BOOST_REQUIRE_EQUAL(ctx.S[0][0], 0);

    BOOST_REQUIRE_EQUAL(L, initial_L);
    BOOST_REQUIRE_EQUAL(R, initial_R);
}

BOOST_AUTO_TEST_CASE(data_encryption_check)
{
    const char input_data[] = "function1 function2 function3"
                              "function4 function5 function6";
    char key[] = "password";
    blowfish_ctx_t ctx;

    auto enc_sz = blowfish_get_stream_output_length(sizeof(input_data));
    BOOST_REQUIRE_GE(enc_sz, sizeof(input_data));
    BOOST_REQUIRE_EQUAL(enc_sz % blowfish_get_min_chunk_length(), 0);

    unsigned char* enc_data = (unsigned char*)alloca(enc_sz);
    BOOST_REQUIRE(blowfish_init(&ctx, (uint8_t*)key, sizeof(key)));
    BOOST_REQUIRE_EQUAL(
        blowfish_stream_encrypt(&ctx, (const unsigned char*)input_data, sizeof(input_data), enc_data, enc_sz), enc_sz);
    BOOST_REQUIRE(blowfish_destroy(&ctx));

    BOOST_REQUIRE(blowfish_init(&ctx, (uint8_t*)key, sizeof(key)));
    unsigned char* dec_data = (unsigned char*)alloca(enc_sz);
    BOOST_REQUIRE_EQUAL(blowfish_stream_decrypt(&ctx, enc_data, enc_sz, dec_data, enc_sz), enc_sz);
    BOOST_REQUIRE(blowfish_destroy(&ctx));

    BOOST_REQUIRE_EQUAL(std::string{ input_data }, std::string{ (char*)dec_data });
}

BOOST_AUTO_TEST_CASE(data_stream_encryption_check)
{
    const char input_data[] = "function1 function2 function3"
                              "function4 function5 function6";
    char key[] = "password";
    blowfish_ctx_t ctx;
    size_t chuck_sz = sizeof(input_data) / 3;

    BOOST_REQUIRE_GE(chuck_sz, blowfish_get_min_chunk_length());

    BOOST_REQUIRE(blowfish_init(&ctx, (uint8_t*)key, sizeof(key)));

    size_t rest_sz = sizeof(input_data);
    std::vector<unsigned char> encoded;
    encoded.reserve(blowfish_get_stream_output_length(rest_sz));
    std::vector<unsigned char> chunk;
    chunk.resize(chuck_sz, 0);
    const unsigned char* p_in_pos = reinterpret_cast<const unsigned char*>(input_data);
    while (rest_sz > 0)
    {
        auto chunk_actial_in = std::min(chuck_sz, rest_sz);
        auto chunk_actial_out = std::max(chunk_actial_in, static_cast<size_t>(blowfish_get_min_chunk_length()));
        auto r = blowfish_stream_encrypt(&ctx, p_in_pos, chunk_actial_in, &chunk[0], chunk_actial_out);
        BOOST_REQUIRE_GT(r, 0);
        p_in_pos += r;
        std::copy_n(begin(chunk), r, std::back_inserter(encoded));
        rest_sz -= (r <= rest_sz) ? r : rest_sz;
    }
    BOOST_REQUIRE(blowfish_destroy(&ctx));

    {
        BOOST_REQUIRE(blowfish_init(&ctx, (uint8_t*)key, sizeof(key)));
        unsigned char* dec_data = (unsigned char*)alloca(encoded.size());
        BOOST_REQUIRE_EQUAL(blowfish_stream_decrypt(&ctx, &encoded[0], encoded.size(), dec_data, encoded.size()),
                            encoded.size());
        BOOST_REQUIRE(blowfish_destroy(&ctx));

        BOOST_REQUIRE_EQUAL(std::string{ input_data }, std::string{ (char*)dec_data });
    }

    BOOST_REQUIRE(blowfish_init(&ctx, (uint8_t*)key, sizeof(key)));
    std::vector<unsigned char> decoded;
    decoded.reserve(encoded.size());
    rest_sz = encoded.size();
    size_t ci = 0;
    while (rest_sz > 0)
    {
        auto chunk_actial_in = std::min(chuck_sz, rest_sz);
        auto chunk_actial_out = std::max(chunk_actial_in, static_cast<size_t>(blowfish_get_min_chunk_length()));
        auto r = blowfish_stream_decrypt(&ctx, &encoded[ci], chunk_actial_in, &chunk[0], chunk_actial_out);
        BOOST_REQUIRE_GT(r, 0);
        ci += r;
        std::copy_n(begin(chunk), r, std::back_inserter(decoded));
        rest_sz -= (r <= rest_sz) ? r : rest_sz;
    }

    BOOST_REQUIRE_EQUAL(std::string{ input_data }, std::string{ (char*)&decoded[0] });
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace server_clib
