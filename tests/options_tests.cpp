#include <boost/test/unit_test.hpp>

#include <server_clib/options.h>

#include <functional>

namespace server_clib {
struct options_tests_fixture
{
    using command_line_processor = std::function<int(int argc, char* argv[])>;

    template <typename... Args> int process_command_line(command_line_processor&& func, Args... args)
    {
        auto sz = sizeof...(args);
        ++sz;
        const char* argv[] = { "test", args..., nullptr };
        return func((int)sz, (char**)argv);
    }

    template <typename... Args> int process_empty_command_line(command_line_processor&& func)
    {
        const char* argv[] = { "test", nullptr };
        return func(1, (char**)argv);
    }
};

static const char* null_val = NULL;

BOOST_FIXTURE_TEST_SUITE(options_tests, options_tests_fixture)

BOOST_AUTO_TEST_CASE(show_cmd_and_help_check)
{
    auto processor = [&](int argc, char* argv[]) {
        BOOST_REQUIRE_EQUAL(argc, 2);
        BOOST_REQUIRE_EQUAL(argv[1], "--help");
        options_ctx_t ctx;
        options_init(&ctx, NULL, NULL);
        return options_parse(&ctx, argc, argv, NULL);
    };
    BOOST_CHECK_EQUAL(process_command_line(processor, "--help"), 0);
}

BOOST_AUTO_TEST_CASE(show_cmd_and_help_for_multiple_check)
{
    auto get_option_description = [](const char short_opt) -> const char* {
        switch (short_opt)
        {
        case 'a':
            return "About 'a' option";
        case 'c':
            return "About 'c' option";
        case 'd':
            return "About 'd' option";
        default:;
        }

        return NULL;
    };

    auto get_option_long_name = [](const char short_opt) -> const char* {
        switch (short_opt)
        {
        case 'a':
            return "a";
        case 'b':
            return "optb";
        case 'c':
            return "optc";
        case 'd':
            return "desc";
        case 'e':
            return "eeeee";
        case 'f':
            return "fffff";
        default:
            BOOST_REQUIRE(false); // there is no such option
        }

        return NULL;
    };

    auto processor = [&](int argc, char* argv[]) {
        options_ctx_t ctx;
        options_init(&ctx, "About this program", "ab:c!de!f");
        ctx.get_option_description = get_option_description;
        ctx.get_option_long_name = get_option_long_name;
        ctx.version_major = 1;
        ctx.version_major = 2;
        ctx.version_patch = 333;
        return options_parse(&ctx, argc, argv, NULL);
    };
    BOOST_CHECK_EQUAL(process_command_line(processor, "--help"), 0);
}

BOOST_AUTO_TEST_CASE(option_with_no_value_check)
{
    // clang-format off
    auto set_option = [](const char short_opt, const char*)
    {
        BOOST_REQUIRE_EQUAL(short_opt, 't');
        return TRUE;
    };
    auto processor = [&](int argc, char* argv[])
    {
        options_ctx_t ctx;
        options_init(&ctx, NULL, "t");
        return options_parse(&ctx, argc, argv, set_option);
    };
    // clang-format on
    BOOST_CHECK_EQUAL(process_command_line(processor, "-t", "file"), 2);
}

BOOST_AUTO_TEST_CASE(option_with_no_value_stealth_check)
{
    // clang-format off
    auto set_option = [](const char short_opt, const char*)
    {
        BOOST_REQUIRE_EQUAL(short_opt, 't');
        return TRUE;
    };
    auto processor = [&](int argc, char* argv[])
    {
        options_ctx_t ctx;
        options_init_stealth(&ctx, "t");
        return options_parse(&ctx, argc, argv, set_option);
    };
    // clang-format on
    BOOST_CHECK_EQUAL(process_command_line(processor, "-t", "file"), 2);
}

BOOST_AUTO_TEST_CASE(option_with_requied_value_check)
{
    // clang-format off
    auto set_option = [](const char short_opt, const char*val)
    {
        BOOST_REQUIRE_EQUAL(short_opt, 't');
        BOOST_REQUIRE_EQUAL(val, "file");
        return TRUE;
    };
    auto processor = [&](int argc, char* argv[])
    {
        options_ctx_t ctx;
        options_init_stealth(&ctx, "t!");
        return options_parse(&ctx, argc, argv, set_option);
    };
    // clang-format on
    BOOST_CHECK_EQUAL(process_command_line(processor, "-t", "file"), 3);
}

BOOST_AUTO_TEST_CASE(option_with_long_name_check)
{
    // clang-format off
    auto set_option = [](const char short_opt, const char*val)
    {
        BOOST_REQUIRE_EQUAL(short_opt, 't');
        BOOST_REQUIRE_EQUAL(val, "file");
        return TRUE;
    };
    auto get_option_long_name = [](const char short_opt) -> const char *
    {
        BOOST_REQUIRE_EQUAL(short_opt, 't');
        return "test";
    };
    auto processor = [&](int argc, char* argv[])
    {
        options_ctx_t ctx;
        options_init_stealth(&ctx, "t!");
        ctx.get_option_long_name = get_option_long_name;
        return options_parse(&ctx, argc, argv, set_option);
    };
    // clang-format on
    BOOST_CHECK_EQUAL(process_command_line(processor, "--test", "file"), 3);
}

BOOST_AUTO_TEST_CASE(option_with_optional_no_value_check)
{
    // clang-format off
    auto set_option = [](const char short_opt, const char*val)
    {
        BOOST_REQUIRE_EQUAL(short_opt, 't');
        BOOST_REQUIRE(!val);
        return TRUE;
    };
    auto get_option_long_name = [](const char short_opt) -> const char *
    {
        BOOST_REQUIRE_EQUAL(short_opt, 't');
        return "test";
    };
    auto processor = [&](int argc, char* argv[])
    {
        options_ctx_t ctx;
        options_init_stealth(&ctx, "t:");
        ctx.get_option_long_name = get_option_long_name;
        return options_parse(&ctx, argc, argv, set_option);
    };
    // clang-format on
    BOOST_CHECK_EQUAL(process_command_line(processor, "--test"), 2);
}

BOOST_AUTO_TEST_CASE(option_with_optional_with_value_check)
{
    // clang-format off
    auto set_option = [](const char short_opt, const char*val)
    {
        BOOST_REQUIRE_EQUAL(short_opt, 't');
        BOOST_REQUIRE_EQUAL(val, "file");
        return TRUE;
    };
    auto get_option_long_name = [](const char short_opt) -> const char *
    {
        BOOST_REQUIRE_EQUAL(short_opt, 't');
        return "test";
    };
    auto processor = [&](int argc, char* argv[])
    {
        options_ctx_t ctx;
        options_init_stealth(&ctx, "t:");
        ctx.get_option_long_name = get_option_long_name;
        return options_parse(&ctx, argc, argv, set_option);
    };
    // clang-format on
    BOOST_CHECK_EQUAL(process_command_line(processor, "--test=file"), 2);
}

BOOST_AUTO_TEST_CASE(option_multiple_check)
{
    // clang-format off
    auto set_option = [](const char short_opt, const char*val)
    {
        switch(short_opt)
        {
        case 'a':
            BOOST_CHECK_PREDICATE(std::equal_to<decltype(null_val)>(), (val)(null_val));
            break;
        case 'b':
            BOOST_REQUIRE_EQUAL(val, "value-for-b");
            break;
        case 'c':
            BOOST_REQUIRE_EQUAL(val, "value-for-c");
            break;
        case 'd':
            BOOST_CHECK_PREDICATE(std::equal_to<decltype(null_val)>(), (val)(null_val));
            break;
        default:
            BOOST_REQUIRE(false); //there is no such option
        }

        return TRUE; //no stop
    };
    auto processor = [&](int argc, char* argv[])
    {
        options_ctx_t ctx;
        options_init_stealth(&ctx, "ab:c!d");
        return options_parse(&ctx, argc, argv, set_option);
    };
    // clang-format on
    BOOST_CHECK_EQUAL(process_command_line(processor, "-a", "-b", "value-for-b", "-c", "value-for-c", "-d"), 7);
}

BOOST_AUTO_TEST_CASE(option_long_multiple_check)
{
    // clang-format off
    auto set_option = [](const char short_opt, const char*val)
    {
        switch(short_opt)
        {
        case 'a':
            BOOST_CHECK_PREDICATE(std::equal_to<decltype(null_val)>(), (val)(null_val));
            break;
        case 'b':
            BOOST_REQUIRE_EQUAL(val, "value-for-b");
            break;
        case 'c':
            BOOST_REQUIRE_EQUAL(val, "value-for-c");
            break;
        case 'd':
            BOOST_CHECK_PREDICATE(std::equal_to<decltype(null_val)>(), (val)(null_val));
            break;
        default:
            BOOST_REQUIRE(false); //there is no such option
        }

        return TRUE; //no stop
    };

    auto get_option_long_name = [](const char short_opt) -> const char *
    {
        switch(short_opt)
        {
        case 'a':
            return "a";
        case 'b':
            return "optb";
        case 'c':
            return "optc";
        case 'd':
            return "desc";
        default:
            BOOST_REQUIRE(false); //there is no such option
        }

        return NULL;
    };

    auto processor = [&](int argc, char* argv[])
    {
        options_ctx_t ctx;
        options_init_stealth(&ctx, "ab:c!d");
        ctx.get_option_long_name = get_option_long_name;
        return options_parse(&ctx, argc, argv, set_option);
    };
    // clang-format on
    BOOST_CHECK_EQUAL(process_command_line(processor, "--a", "--optb=value-for-b", "--optc", "value-for-c", "--desc"),
                      6);
}

BOOST_AUTO_TEST_CASE(option_single_required_option_check)
{
    auto processor = [&](int argc, char* argv[]) {
        std::string opt = options_parse_stealth_single_option(argc, argv, TRUE);
        BOOST_REQUIRE_EQUAL(opt, "path-to-file");
        return 0;
    };
    // clang-format on
    BOOST_CHECK_EQUAL(process_command_line(processor, "path-to-file"), 0);
}

BOOST_AUTO_TEST_CASE(option_no_single_required_option_check)
{
    auto processor = [&](int argc, char* argv[]) {
        const char* r = options_parse_stealth_single_option(argc, argv, TRUE);
        BOOST_CHECK_PREDICATE(std::equal_to<decltype(null_val)>(), (r)(null_val));
        return 0;
    };
    // clang-format on
    BOOST_CHECK_EQUAL(process_empty_command_line(processor), 0);
}

BOOST_AUTO_TEST_CASE(option_single_required_option_help_check)
{
    auto processor = [&](int argc, char* argv[]) {
        const char* r = options_parse_stealth_single_option(argc, argv, TRUE);
        BOOST_CHECK_PREDICATE(std::equal_to<decltype(null_val)>(), (r)(null_val));
        return 0;
    };
    // clang-format on
    BOOST_CHECK_EQUAL(process_command_line(processor, "-h"), 0);
}

BOOST_AUTO_TEST_CASE(option_single_optional_option_check)
{
    auto processor = [&](int argc, char* argv[]) {
        std::string opt = options_parse_stealth_single_option(argc, argv, FALSE);
        BOOST_REQUIRE_EQUAL(opt, "path-to-file");
        return 0;
    };
    // clang-format on
    BOOST_CHECK_EQUAL(process_command_line(processor, "path-to-file"), 0);
}

BOOST_AUTO_TEST_CASE(option_no_single_optional_option_check)
{
    auto processor = [&](int argc, char* argv[]) {
        const char* r = options_parse_stealth_single_option(argc, argv, FALSE);
        BOOST_CHECK_PREDICATE(std::equal_to<decltype(null_val)>(), (r)(null_val));
        return 0;
    };
    // clang-format on
    BOOST_CHECK_EQUAL(process_empty_command_line(processor), 0);
}

BOOST_AUTO_TEST_CASE(option_single_optional_option_help_check)
{
    auto processor = [&](int argc, char* argv[]) {
        const char* r = options_parse_stealth_single_option(argc, argv, FALSE);
        BOOST_CHECK_PREDICATE(std::equal_to<decltype(null_val)>(), (r)(null_val));
        return 0;
    };
    // clang-format on
    BOOST_CHECK_EQUAL(process_command_line(processor, "-h"), 0);
}

BOOST_AUTO_TEST_CASE(option_without_required_value_check)
{
    auto processor = [&](int argc, char* argv[]) {
        options_ctx_t ctx;
        options_init_stealth(&ctx, "at!b");
        return options_parse(&ctx, argc, argv, NULL);
    };
    // clang-format on
    BOOST_CHECK_EQUAL(process_command_line(processor, "-a", "-t"), -1);
    //(!) but -a whithout -t allow!
    BOOST_CHECK_EQUAL(process_command_line(processor, "-a"), 2);
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace server_clib
