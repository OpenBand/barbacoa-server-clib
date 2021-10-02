#include <boost/test/unit_test.hpp>

#include <server_clib/rubber.h>

namespace server_clib {
BOOST_AUTO_TEST_SUITE(rubber_tests)

BOOST_AUTO_TEST_CASE(rubber_init_check)
{
    static const size_t chunk_sz = 100;
    srv_c_rubber_ctx_t ctx;

    BOOST_REQUIRE(srv_c_rubber_init(&ctx, chunk_sz, false) > 0);
    BOOST_REQUIRE_EQUAL(ctx.chunk_sz, chunk_sz);
    BOOST_REQUIRE_GT(ctx.sz, 0);
    BOOST_REQUIRE_EQUAL(ctx.sz, ctx.rest);
    BOOST_REQUIRE_EQUAL(ctx.written, 0);

    BOOST_REQUIRE(srv_c_rubber_destroy(&ctx) > 0);
}

BOOST_AUTO_TEST_CASE(rubber_init_from_buff_check)
{
    static char info[100];
    static const size_t buff_sz = sizeof info;
    static const size_t chunk_sz = buff_sz / 10;
    srv_c_rubber_ctx_t ctx;

    BOOST_REQUIRE(srv_c_rubber_init_from_buff(&ctx, info, sizeof info, chunk_sz, false) > 0);

    BOOST_REQUIRE_EQUAL(ctx.chunk_sz, chunk_sz);
    BOOST_REQUIRE_EQUAL(ctx.sz, buff_sz);
    BOOST_REQUIRE_EQUAL(ctx.sz, ctx.rest);
    BOOST_REQUIRE_EQUAL(ctx.written, 0);

    BOOST_REQUIRE(srv_c_rubber_destroy(&ctx) > 0);
}

BOOST_AUTO_TEST_CASE(rubber_go_whithout_enlarge_check)
{
    static char info[100];
    static const size_t buff_sz = sizeof info;
    static const size_t chunk_sz = buff_sz / 10;
    srv_c_rubber_ctx_t ctx;

    BOOST_REQUIRE(srv_c_rubber_init_from_buff(&ctx, info, sizeof info, chunk_sz, false) > 0);

    int wrn = 0;
    BOOST_REQUIRE(srv_c_rubber_pos(&ctx, &wrn));
    wrn = 5;
    BOOST_REQUIRE(srv_c_rubber_pos(&ctx, &wrn));

    BOOST_REQUIRE_EQUAL(ctx.rest, ctx.sz - 5);
    BOOST_REQUIRE_EQUAL(ctx.written, 5);

    BOOST_REQUIRE(srv_c_rubber_destroy(&ctx) > 0);
}

BOOST_AUTO_TEST_CASE(rubber_go_whith_enlarge_check)
{
    static char info[100];
    static const size_t buff_sz = sizeof info;
    static const size_t chunk_sz = buff_sz / 10;
    srv_c_rubber_ctx_t ctx;

    BOOST_REQUIRE(srv_c_rubber_init_from_buff(&ctx, info, sizeof info, chunk_sz, false) > 0);

    BOOST_REQUIRE_EQUAL(ctx.sz, buff_sz);

    int wrn = 0;
    BOOST_REQUIRE(srv_c_rubber_pos(&ctx, &wrn));
    wrn = 5;
    BOOST_REQUIRE(srv_c_rubber_pos(&ctx, &wrn));

    BOOST_REQUIRE_EQUAL(ctx.rest, ctx.sz - 5);
    BOOST_REQUIRE_EQUAL(ctx.written, 5);

    auto written = ctx.rest - 1;
    wrn = written;
    BOOST_REQUIRE(srv_c_rubber_pos(&ctx, &wrn));

    BOOST_REQUIRE_EQUAL(ctx.rest, buff_sz);

    wrn = 1;
    BOOST_REQUIRE(srv_c_rubber_pos(&ctx, &wrn));

    BOOST_REQUIRE_EQUAL(ctx.rest, buff_sz - 1);
    BOOST_REQUIRE_EQUAL(ctx.written, 5 + written + 1);
    BOOST_REQUIRE_EQUAL(ctx.sz, buff_sz * 2);

    BOOST_REQUIRE(srv_c_rubber_destroy(&ctx) > 0);
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace server_clib
