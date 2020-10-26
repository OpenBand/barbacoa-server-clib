#include <boost/test/unit_test.hpp>

#include <server_clib/config.h>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <fstream>
#include <sstream>
#include <iomanip> // std::put_time, std::get_time

namespace server_clib {

struct config_tests_fixture
{
    using path = boost::filesystem::path;

    config_tests_fixture()
    {
        auto local_dir = boost::filesystem::temp_directory_path() / "server-clib-tests";
        boost::filesystem::remove_all(local_dir);
        BOOST_REQUIRE(boost::filesystem::exists(local_dir) || boost::filesystem::create_directories(local_dir));
        _test_dir = local_dir;
    }
    ~config_tests_fixture()
    {
        if (boost::filesystem::exists(_test_dir))
            boost::filesystem::remove_all(_test_dir);
    }

    std::string create_config(const std::string& content)
    {
        auto config_path = _test_dir;
        config_path /= boost::filesystem::unique_path();
        std::ofstream out(config_path.generic_string());
        out << content;
        out.close();
        return config_path.generic_string();
    }

    path _test_dir;
};

BOOST_FIXTURE_TEST_SUITE(config_tests, config_tests_fixture)

BOOST_AUTO_TEST_CASE(config_json_string_string_check)
{
    std::string config = R"(
                         { "op1" : "option" }
                         )";
    auto config_path = create_config(config);

    auto config_callback = [](const char* key, const config_ctx_t* pctx) -> BOOL {
        std::string expected_key = "op1";
        BOOST_REQUIRE_EQUAL(expected_key, std::string{ key });
        char buff[MAX_INPUT];
        if (!config_get_string(pctx, buff, MAX_INPUT))
            return false;

        std::string expected_value = "option";
        BOOST_REQUIRE_EQUAL(expected_value, std::string{ buff });

        return true;
    };

    BOOST_REQUIRE(config_load(config_path.c_str(), config_callback));
}

BOOST_AUTO_TEST_CASE(config_jsmn_string_string_check)
{
    std::string config = R"(
                         "op1" : "option"
                         )";
    auto config_path = create_config(config);

    auto config_callback = [](const char* key, const config_ctx_t* pctx) -> BOOL {
        std::string expected_key = "op1";
        BOOST_REQUIRE_EQUAL(expected_key, std::string{ key });
        char buff[MAX_INPUT];
        if (!config_get_string(pctx, buff, MAX_INPUT))
            return false;

        std::string expected_value = "option";
        BOOST_REQUIRE_EQUAL(expected_value, std::string{ buff });

        return true;
    };

    BOOST_REQUIRE(config_load(config_path.c_str(), config_callback));
}

BOOST_AUTO_TEST_CASE(config_jsmn2_string_string_check)
{
    std::string config = R"(
                         op1 : "option"
                         )";
    auto config_path = create_config(config);

    auto config_callback = [](const char* key, const config_ctx_t* pctx) -> BOOL {
        std::string expected_key = "op1";
        BOOST_REQUIRE_EQUAL(expected_key, std::string{ key });
        char buff[MAX_INPUT];
        if (!config_get_string(pctx, buff, MAX_INPUT))
            return false;

        std::string expected_value = "option";
        BOOST_REQUIRE_EQUAL(expected_value, std::string{ buff });

        return true;
    };

    BOOST_REQUIRE(config_load(config_path.c_str(), config_callback));
}

BOOST_AUTO_TEST_CASE(config_ini_string_string_check)
{
    std::string config = R"(
                         # test option
                         #test option
#test option
#  test option
                         op1 = "option"
                         )";
    auto config_path = create_config(config);

    auto config_callback = [](const char* key, const config_ctx_t* pctx) -> BOOL {
        std::string expected_key = "op1";
        BOOST_REQUIRE_EQUAL(expected_key, std::string{ key });
        char buff[MAX_INPUT];
        if (!config_get_string(pctx, buff, MAX_INPUT))
            return false;

        std::string expected_value = "option";
        BOOST_REQUIRE_EQUAL(expected_value, std::string{ buff });

        return true;
    };

    BOOST_REQUIRE(config_load(config_path.c_str(), config_callback));
}

BOOST_AUTO_TEST_CASE(config_mixed_string_string_check)
{
    std::string config = R"(
                         op1 = "option1"
                         # first option
                         "op2" : "option2"
                         op3 = "ini-option"
                         )";
    auto config_path = create_config(config);

    auto config_callback = [](const char* key, const config_ctx_t* pctx) -> BOOL {
        if (!strncmp(key, "op1", MAX_INPUT))
        {
            char buff[MAX_INPUT];
            if (!config_get_string(pctx, buff, MAX_INPUT))
                return false;

            BOOST_REQUIRE_EQUAL(std::string{ "option1" }, std::string{ buff });
        }
        else if (!strncmp(key, "op2", MAX_INPUT))
        {
            char buff[MAX_INPUT];
            if (!config_get_string(pctx, buff, MAX_INPUT))
                return false;

            BOOST_REQUIRE_EQUAL(std::string{ "option2" }, std::string{ buff });
        }
        else if (!strncmp(key, "op3", MAX_INPUT))
        {
            char buff[MAX_INPUT];
            if (!config_get_string(pctx, buff, MAX_INPUT))
                return false;

            BOOST_REQUIRE_EQUAL(std::string{ "ini-option" }, std::string{ buff });
        }
        else
        {
            return false;
        }

        return true;
    };

    BOOST_REQUIRE(config_load(config_path.c_str(), config_callback));
}

BOOST_AUTO_TEST_CASE(config_mixed_simple_types_check)
{
    std::string config = R"(
                         op1  = "option1"
                         op2  = 123
                         op3  = true
                         op4  = -321
                         op5  = false
                         op6  = 0
                         op7  = 1
                         )";
    auto config_path = create_config(config);

    auto config_callback = [](const char* key, const config_ctx_t* pctx) -> BOOL {
        if (!strncmp(key, "op1", MAX_INPUT))
        {
            char buff[MAX_INPUT];
            if (!config_get_string(pctx, buff, MAX_INPUT))
                return false;

            BOOST_REQUIRE_EQUAL(std::string{ "option1" }, std::string{ buff });
        }
        else if (!strncmp(key, "op2", MAX_INPUT))
        {
            int val = -1;
            if (!config_get_int(pctx, &val))
                return false;

            BOOST_REQUIRE_EQUAL(val, 123);
        }
        else if (!strncmp(key, "op3", MAX_INPUT))
        {
            BOOL val = false;
            if (!config_get_bool(pctx, &val))
                return false;

            BOOST_REQUIRE_EQUAL(val, true);
        }
        else if (!strncmp(key, "op4", MAX_INPUT))
        {
            int val = 0;
            if (!config_get_int(pctx, &val))
                return false;

            BOOST_REQUIRE_EQUAL(val, -321);
        }
        else if (!strncmp(key, "op5", MAX_INPUT))
        {
            BOOL val = true;
            if (!config_get_bool(pctx, &val))
                return false;

            BOOST_REQUIRE_EQUAL(val, false);
        }
        else if (!strncmp(key, "op6", MAX_INPUT))
        {
            BOOL val = true;
            if (!config_get_bool(pctx, &val))
                return false;

            BOOST_REQUIRE_EQUAL(val, false);
        }
        else if (!strncmp(key, "op7", MAX_INPUT))
        {
            BOOL val = false;
            if (!config_get_bool(pctx, &val))
                return false;

            BOOST_REQUIRE_EQUAL(val, true);
        }
        else
        {
            return false;
        }

        return true;
    };

    BOOST_REQUIRE(config_load(config_path.c_str(), config_callback));
}

BOOST_AUTO_TEST_CASE(config_array_of_int_check)
{
    std::string config = R"(
                    op1 = [1, 2, 3, 4, 5]
                         )";
    auto config_path = create_config(config);

    auto config_callback = [](const char* key, const config_ctx_t* pctx) -> BOOL {
        std::string expected_key = "op1";
        BOOST_REQUIRE_EQUAL(expected_key, std::string{ key });
        int* parray = nullptr;
        size_t sz = 0;
        if (!config_get_int_array(pctx, &parray, &sz, 10))
            return false;

        BOOST_REQUIRE_EQUAL(sz, 5u);
        for (size_t ci = 0; ci < sz; ++ci)
        {
            BOOST_REQUIRE_EQUAL(parray[ci], ci + 1);
        }
        free(parray);

        return true;
    };

    BOOST_REQUIRE(config_load(config_path.c_str(), config_callback));
}

BOOST_AUTO_TEST_CASE(config_array_of_bool_check)
{
    std::string config = R"(
                    op1 = [1, false, true, false, 1, 0]
                         )";
    auto config_path = create_config(config);

    auto config_callback = [](const char* key, const config_ctx_t* pctx) -> BOOL {
        std::string expected_key = "op1";
        BOOST_REQUIRE_EQUAL(expected_key, std::string{ key });
        int* parray = nullptr;
        size_t sz = 0;
        if (!config_get_bool_array(pctx, &parray, &sz, 10))
            return false;

        BOOST_REQUIRE_EQUAL(sz, 6u);
        for (size_t ci = 0; ci < sz; ++ci)
        {
            BOOST_REQUIRE_EQUAL(static_cast<bool>(parray[ci]), ci % 2 == 0);
        }
        free(parray);

        return true;
    };

    BOOST_REQUIRE(config_load(config_path.c_str(), config_callback));
}

BOOST_AUTO_TEST_CASE(config_array_of_string_check)
{
    std::string config = R"(
                    op1 = ["value1", "value2", "value3", "value4", "value5", "long_value"]
                         )";
    auto config_path = create_config(config);

    auto config_callback = [](const char* key, const config_ctx_t* pctx) -> BOOL {
        std::string expected_key = "op1";
        BOOST_REQUIRE_EQUAL(expected_key, std::string{ key });
        char* parray = nullptr;
        size_t sz = 0;
        const size_t item_sz = 30;
        if (!config_get_string_array(pctx, &parray, &sz, item_sz, 10))
            return false;

        BOOST_REQUIRE_EQUAL(sz, 6u);
        char* pval = parray;
        for (size_t ci = 0; ci < sz - 1; ++ci)
        {
            std::stringstream ss;
            ss << "value";
            ss << ci + 1;
            BOOST_REQUIRE_EQUAL(std::string{ ss.str() }, std::string{ pval });
            pval += item_sz;
        }

        BOOST_REQUIRE_EQUAL(std::string{ "long_value" }, std::string{ pval });
        free(parray);

        return true;
    };

    BOOST_REQUIRE(config_load(config_path.c_str(), config_callback));
}

BOOST_AUTO_TEST_CASE(config_signle_line_config_check)
{
    std::string config
        = R"({ "op1": "option1", "op2": 123, "op3": true, "op4": [1, 2, 3, 4, 5, 6, 7], "op5": ["v1", "value2"] })";
    auto config_path = create_config(config);

    auto config_callback = [](const char* key, const config_ctx_t* pctx) -> BOOL {
        if (!strncmp(key, "op1", MAX_INPUT))
        {
            char buff[MAX_INPUT];
            if (!config_get_string(pctx, buff, MAX_INPUT))
                return false;

            BOOST_REQUIRE_EQUAL(std::string{ "option1" }, std::string{ buff });
        }
        else if (!strncmp(key, "op2", MAX_INPUT))
        {
            int val = -1;
            if (!config_get_int(pctx, &val))
                return false;

            BOOST_REQUIRE_EQUAL(val, 123);
        }
        else if (!strncmp(key, "op3", MAX_INPUT))
        {
            BOOL val = false;
            if (!config_get_bool(pctx, &val))
                return false;

            BOOST_REQUIRE_EQUAL(val, true);
        }
        else if (!strncmp(key, "op4", MAX_INPUT))
        {
            int* parray = nullptr;
            size_t sz = 0;
            if (!config_get_int_array(pctx, &parray, &sz, 5)) // test for max_sz limitation
                return false;

            BOOST_REQUIRE_EQUAL(sz, 5u);
            for (size_t ci = 0; ci < sz; ++ci)
            {
                BOOST_REQUIRE_EQUAL(parray[ci], ci + 1);
            }
            free(parray);
        }
        else if (!strncmp(key, "op5", MAX_INPUT))
        {
            char* parray = nullptr;
            size_t sz = 0;
            const size_t item_sz = 30;
            if (!config_get_string_array(pctx, &parray, &sz, item_sz, 10))
                return false;

            BOOST_REQUIRE_EQUAL(sz, 2u);
            char* pval = parray;
            BOOST_REQUIRE_EQUAL(std::string{ "v1" }, std::string{ pval });
            pval += item_sz;
            BOOST_REQUIRE_EQUAL(std::string{ "value2" }, std::string{ pval });
            free(parray);
        }
        else
        {
            return false;
        }

        return true;
    };

    BOOST_REQUIRE(config_load(config_path.c_str(), config_callback));
}

BOOST_AUTO_TEST_CASE(config_time_config_check)
{
    std::string config = R"(
                         op1 = "2018-12-06T10:28:20"
                         )";
    auto config_path = create_config(config);

    auto config_callback = [](const char* key, const config_ctx_t* pctx) -> BOOL {
        std::string expected_key = "op1";
        BOOST_REQUIRE_EQUAL(expected_key, std::string{ key });

        static const char* TIME_FORMAT = "%Y-%m-%dT%H:%M:%S";

        std::time_t unix_local_t;
        if (!config_get_local_time(pctx, TIME_FORMAT, &unix_local_t))
            return false;

        std::stringstream ss_local;

        ss_local << std::put_time(std::localtime(&unix_local_t), TIME_FORMAT);

        BOOST_REQUIRE_EQUAL(std::string{ "2018-12-06T10:28:20" }, ss_local.str());

        std::time_t unix_utc_t;
        if (!config_get_utc_time(pctx, TIME_FORMAT, &unix_utc_t))
            return false;

        std::stringstream ss_utc;

        ss_utc << std::put_time(std::gmtime(&unix_utc_t), TIME_FORMAT);

        BOOST_REQUIRE_EQUAL(std::string{ "2018-12-06T10:28:20" }, ss_utc.str());

        return true;
    };

    BOOST_REQUIRE(config_load(config_path.c_str(), config_callback));
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace server_clib
