#include <boost/test/unit_test.hpp>

#include <server_clib/hex.h>
#include <server_clib/macro.h>

#include <cstring>

namespace server_clib {
BOOST_AUTO_TEST_SUITE(hex_tests)

BOOST_AUTO_TEST_CASE(positive_convertion_check)
{
    const char data[10] = "12345abra";

    char enc_buff[MAX_INPUT];
    auto r = srv_c_hex_stream_to_hex((const unsigned char*)data, sizeof(data), enc_buff, sizeof(enc_buff));

    BOOST_REQUIRE_EQUAL(r, sizeof(data));
    BOOST_REQUIRE_EQUAL(std::string(enc_buff), "31323334356162726100");

    char dec_buff[MAX_INPUT];
    r = srv_c_hex_stream_from_hex(enc_buff, strlen(enc_buff), (unsigned char*)dec_buff, sizeof(dec_buff));

    BOOST_REQUIRE_EQUAL(r, strlen(enc_buff));
    BOOST_REQUIRE_EQUAL(std::string(dec_buff), "12345abra");

    std::memset(dec_buff, 0, sizeof(dec_buff));

    r = srv_c_hex_stream_from_hex(enc_buff, strlen(enc_buff) - 3, (unsigned char*)dec_buff, sizeof(dec_buff));

    BOOST_REQUIRE_EQUAL(r, strlen(enc_buff) - 4);
    BOOST_REQUIRE_EQUAL(std::string(dec_buff), "12345abr");
}

BOOST_AUTO_TEST_CASE(sanitize_to_hex_convertion_check)
{
    char buff[MAX_INPUT];

    BOOST_REQUIRE_EQUAL(srv_c_hex_stream_to_hex(nullptr, 100, buff, sizeof(buff)), -1);
    BOOST_REQUIRE_EQUAL(srv_c_hex_stream_to_hex((const unsigned char*)buff, 0, buff, sizeof(buff)), -1);
    BOOST_REQUIRE_EQUAL(srv_c_hex_stream_to_hex((const unsigned char*)buff, sizeof(buff), nullptr, sizeof(buff)), -1);
    BOOST_REQUIRE_EQUAL(srv_c_hex_stream_to_hex((const unsigned char*)buff, sizeof(buff), buff, 0), -1);
    const int data = 12;
    BOOST_REQUIRE_EQUAL(srv_c_hex_stream_to_hex((const unsigned char*)&data, 1, buff, sizeof(buff)), 1);
    BOOST_REQUIRE_EQUAL(srv_c_hex_stream_to_hex((const unsigned char*)&data, 1, buff, 1), 0);

    const char multy_data[10] = "12345abra";
    BOOST_REQUIRE_EQUAL(
        srv_c_hex_stream_to_hex((const unsigned char*)multy_data, sizeof(multy_data), buff, sizeof(multy_data) * 2), 10);

    std::memset(buff, 0, sizeof(buff));
    BOOST_REQUIRE_EQUAL(
        srv_c_hex_stream_to_hex((const unsigned char*)multy_data, sizeof(multy_data), buff, sizeof(multy_data) * 2 - 3), 8);

    BOOST_REQUIRE_EQUAL(std::string(buff), "3132333435616272");
}

BOOST_AUTO_TEST_CASE(sanitize_from_hex_convertion_check)
{
    char buff[MAX_INPUT];

    BOOST_REQUIRE_EQUAL(srv_c_hex_stream_from_hex(nullptr, 100, (unsigned char*)buff, sizeof(buff)), -1);
    BOOST_REQUIRE_EQUAL(srv_c_hex_stream_from_hex(buff, 0, (unsigned char*)buff, sizeof(buff)), -1);
    BOOST_REQUIRE_EQUAL(srv_c_hex_stream_from_hex(buff, sizeof(buff), nullptr, sizeof(buff)), -1);
    BOOST_REQUIRE_EQUAL(srv_c_hex_stream_from_hex(buff, sizeof(buff), (unsigned char*)buff, 0), -1);
    const int data = 12;
    BOOST_REQUIRE_EQUAL(srv_c_hex_stream_from_hex((const char*)&data, 1, (unsigned char*)buff, sizeof(buff)), 0);
    BOOST_REQUIRE_EQUAL(srv_c_hex_stream_from_hex("0c", 2, (unsigned char*)buff, 1), 2);
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace server_clib
