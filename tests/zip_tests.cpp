#include <boost/test/unit_test.hpp>

#include <server_clib/zip.h>
#include <server_clib/zip_stream.h>
#include <server_clib/macro.h>

#include <iostream>
#include <algorithm>
#include <fstream>

#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>

namespace server_clib {

#define PRINT_ZIP(method, input_sz, output_sz)                                                              \
    {                                                                                                       \
        std::cerr << method << " (T:" << __LINE__ << "): " << input_sz << " -> " << output_sz << std::endl; \
    }

static const char INPUT_ZIP_DATA[] = "startttt function1 function2 function3 function4 functio--8n5"
                                     "function6 yuiewpps function7 yeriowoo_ function8 function9"
                                     "function10 hertuuios unction function11 iroeoeoi function12"
                                     "testtttt function13 function14 function15 function16 function17"
                                     "function18 function19 function20 function21 function22 function23"
                                     "function24 function25 function26 function27 function28 function29"
                                     "abraaaaaaaaaaaaafunction function30 function31 function32"
                                     "startttt function1 function2 function3 function4 functio--8n5"
                                     "function6 yuiewpps function7 yeriowoo_ function8 function9"
                                     "function10 hertuuios unction function11 iroeoeoi function12"
                                     "testtttt function13 function14 function15 function16 function17"
                                     "function18 function19 function20 function21 function22 function23"
                                     "function24 function25 function26 function27 function28 function29"
                                     "abraaaaaaaaaaaaafunction function30 function31 function32"
                                     "startttt function1 function2 function3 function4 functio--8n5"
                                     "function6 yuiewpps function7 yeriowoo_ function8 function9"
                                     "function10 hertuuios unction function11 iroeoeoi function12"
                                     "testtttt function13 function14 function15 function16 function17"
                                     "function18 function19 function20 function21 function22 function23"
                                     "function24 function25 function26 function27 function28 function29"
                                     "abraaaaaaaaaaaaafunction function30 function31 function32"
                                     "startttt function1 function2 function3 function4 functio--8n5"
                                     "function6 yuiewpps function7 yeriowoo_ function8 function9"
                                     "function10 hertuuios unction function11 iroeoeoi function12"
                                     "testtttt function13 function14 function15 function16 function17"
                                     "function18 function19 function20 function21 function22 function23"
                                     "function24 function25 function26 function27 function28 function29"
                                     "abraaaaaaaaaaaaafunction function30 function31 function32 ***";

struct zip_files_tests
{
    using path = boost::filesystem::path;

    zip_files_tests()
    {
        auto local_dir = boost::filesystem::temp_directory_path() / "server-clib-tests";
        boost::filesystem::remove_all(local_dir);
        BOOST_REQUIRE(boost::filesystem::exists(local_dir) || boost::filesystem::create_directories(local_dir));
        _test_dir = local_dir;
    }
    ~zip_files_tests()
    {
        if (boost::filesystem::exists(_test_dir))
            boost::filesystem::remove_all(_test_dir);
    }

    std::string create_file_path()
    {
        auto new_path = _test_dir;
        new_path /= boost::filesystem::unique_path();
        return new_path.generic_string();
    }

private:
    path _test_dir;
};

BOOST_AUTO_TEST_SUITE(zip_tests)

BOOST_AUTO_TEST_CASE(zip_best_speed_check)
{
    char* pzip_data = nullptr;
    size_t zip_sz = 0;
    BOOST_REQUIRE(zip_pack_best_speed((const unsigned char*)INPUT_ZIP_DATA, sizeof(INPUT_ZIP_DATA),
                                      (unsigned char**)&pzip_data, &zip_sz, true));
    BOOST_REQUIRE_LT(zip_sz, sizeof(INPUT_ZIP_DATA)); // packed

    PRINT_ZIP("zip", sizeof(INPUT_ZIP_DATA), zip_sz)

    char* punzip_data = nullptr;
    size_t unzip_sz = sizeof(INPUT_ZIP_DATA) * 2;
    BOOST_REQUIRE(zip_unpack((const unsigned char*)pzip_data, zip_sz, (unsigned char**)&punzip_data, &unzip_sz, true));
    BOOST_REQUIRE_EQUAL(unzip_sz, sizeof(INPUT_ZIP_DATA));
    BOOST_REQUIRE_EQUAL(std::string { punzip_data }, std::string { INPUT_ZIP_DATA });

    free(pzip_data);
    free(punzip_data);
}

BOOST_AUTO_TEST_CASE(zip_best_size_check)
{
    char* pzip_data = nullptr;
    size_t zip_sz = 0;
    BOOST_REQUIRE(zip_pack_best_size((const unsigned char*)INPUT_ZIP_DATA, sizeof(INPUT_ZIP_DATA),
                                     (unsigned char**)&pzip_data, &zip_sz, true));
    BOOST_REQUIRE_LT(zip_sz, sizeof(INPUT_ZIP_DATA)); // packed

    PRINT_ZIP("zip", sizeof(INPUT_ZIP_DATA), zip_sz)

    char* punzip_data = nullptr;
    size_t unzip_sz = sizeof(INPUT_ZIP_DATA) * 2;
    BOOST_REQUIRE(zip_unpack((const unsigned char*)pzip_data, zip_sz, (unsigned char**)&punzip_data, &unzip_sz, true));
    BOOST_REQUIRE_EQUAL(unzip_sz, sizeof(INPUT_ZIP_DATA));
    BOOST_REQUIRE_EQUAL(std::string { punzip_data }, std::string { INPUT_ZIP_DATA });

    free(pzip_data);
    free(punzip_data);
}

BOOST_AUTO_TEST_CASE(zip_with_preallocated_buffer_check)
{
    constexpr auto SZ = sizeof(INPUT_ZIP_DATA) * 2;
    unsigned char* buffer_in = (unsigned char*)alloca(SZ);
    unsigned char* buffer_out = (unsigned char*)alloca(SZ / 2 + 10);

    size_t zip_sz = SZ;
    BOOST_REQUIRE(zip_pack_best_speed((const unsigned char*)INPUT_ZIP_DATA, sizeof(INPUT_ZIP_DATA),
                                      (unsigned char**)&buffer_in, &zip_sz, false));
    BOOST_REQUIRE_LT(zip_sz, sizeof(INPUT_ZIP_DATA)); // packed

    PRINT_ZIP("zip", sizeof(INPUT_ZIP_DATA), zip_sz)

    size_t unzip_sz = SZ / 2 + 10;
    BOOST_REQUIRE(zip_unpack(buffer_in, zip_sz, (unsigned char**)&buffer_out, &unzip_sz, false));
    BOOST_REQUIRE_EQUAL(unzip_sz, sizeof(INPUT_ZIP_DATA));

    auto result = std::string { (char*)buffer_out, unzip_sz - 1 };
    BOOST_REQUIRE_EQUAL(result, std::string { INPUT_ZIP_DATA });
}

BOOST_AUTO_TEST_CASE(data_stream_packing_single_input_check)
{
    const size_t CHUNK_SZ = 40;

    BOOST_REQUIRE_GT(sizeof(INPUT_ZIP_DATA), CHUNK_SZ);

    zip_stream_ctx_t ctx;
    zip_stream_pack_init(&ctx);

    std::vector<unsigned char> chunk;
    chunk.resize(CHUNK_SZ);

    std::vector<unsigned char> packed_data;
    long processed = zip_stream_start_pack_chunk(&ctx, (const unsigned char*)INPUT_ZIP_DATA, sizeof(INPUT_ZIP_DATA),
                                                 &chunk[0], chunk.size());
    BOOST_REQUIRE(processed > 0);
    std::copy_n(begin(chunk), processed, std::back_inserter(packed_data));
    while (processed == chunk.size())
    {
        processed = zip_stream_pack_chunk(&ctx, &chunk[0], chunk.size());
        BOOST_REQUIRE(processed > 0);
        std::copy_n(begin(chunk), processed, std::back_inserter(packed_data));
    }

    processed = zip_stream_finish_pack(&ctx, &chunk[0], chunk.size());
    BOOST_REQUIRE(processed >= 0);
    std::copy_n(begin(chunk), processed, std::back_inserter(packed_data));

    BOOST_REQUIRE(zip_stream_pack_destroy(&ctx));

    PRINT_ZIP("gzip", sizeof(INPUT_ZIP_DATA), packed_data.size())

    zip_stream_unpack_init(&ctx);
    chunk.resize(CHUNK_SZ, 0);

    std::vector<unsigned char> unpacked_data;

    processed = zip_stream_start_unpack_chuck(&ctx, &packed_data[0], packed_data.size(), &chunk[0], chunk.size());
    BOOST_REQUIRE(processed > 0);
    std::copy_n(begin(chunk), processed, std::back_inserter(unpacked_data));
    while (processed == chunk.size())
    {
        processed = zip_stream_unpack_chuck(&ctx, &chunk[0], chunk.size());
        BOOST_REQUIRE(processed > 0);
        std::copy_n(begin(chunk), processed, std::back_inserter(unpacked_data));
    }

    BOOST_REQUIRE(zip_stream_unpack_destroy(&ctx));

    BOOST_REQUIRE_EQUAL(std::string { INPUT_ZIP_DATA }, std::string { (char*)&unpacked_data[0] });
}

BOOST_AUTO_TEST_CASE(data_stream_packing_chunked_input_check)
{
    const size_t INPUT_CHUNK_SZ = sizeof(INPUT_ZIP_DATA) / 3;
    const size_t OUTPUT_CHUNK_SZ = 40;

    BOOST_REQUIRE_GT(INPUT_CHUNK_SZ, OUTPUT_CHUNK_SZ);

    std::vector<unsigned char> input_chunk;
    input_chunk.resize(INPUT_CHUNK_SZ);
    size_t rest_sz = sizeof(INPUT_ZIP_DATA);

    zip_stream_ctx_t ctx;
    zip_stream_pack_init(&ctx);

    std::vector<unsigned char> output_chunk;
    output_chunk.resize(OUTPUT_CHUNK_SZ);

    std::vector<unsigned char> packed_data;
    const unsigned char* input_pos = reinterpret_cast<const unsigned char*>(INPUT_ZIP_DATA);

    long processed = 0;
    while (rest_sz > 0)
    {
        auto actual_input_sz = std::min(rest_sz, input_chunk.size());
        memcpy(&input_chunk[0], input_pos, actual_input_sz);

        processed = zip_stream_start_pack_chunk(&ctx, &input_chunk[0], actual_input_sz, &output_chunk[0],
                                                output_chunk.size());
        BOOST_REQUIRE(processed > 0);
        std::copy_n(begin(output_chunk), processed, std::back_inserter(packed_data));
        while (processed == output_chunk.size())
        {
            processed = zip_stream_pack_chunk(&ctx, &output_chunk[0], output_chunk.size());
            BOOST_REQUIRE(processed > 0);
            std::copy_n(begin(output_chunk), processed, std::back_inserter(packed_data));
        }

        processed = zip_stream_finish_pack_chunk(&ctx, &output_chunk[0], output_chunk.size());
        BOOST_REQUIRE(processed >= 0);
        std::copy_n(begin(output_chunk), processed, std::back_inserter(packed_data));

        rest_sz -= actual_input_sz;
        input_pos += actual_input_sz;
    }

    processed = zip_stream_finish_pack(&ctx, &output_chunk[0], output_chunk.size());
    BOOST_REQUIRE(processed >= 0);
    std::copy_n(begin(output_chunk), processed, std::back_inserter(packed_data));

    BOOST_REQUIRE(zip_stream_pack_destroy(&ctx));

    PRINT_ZIP("gzip", sizeof(INPUT_ZIP_DATA), packed_data.size())

    zip_stream_unpack_init(&ctx);
    output_chunk.resize(OUTPUT_CHUNK_SZ, 0);

    std::vector<unsigned char> unpacked_data;

    processed = zip_stream_start_unpack_chuck(&ctx, &packed_data[0], packed_data.size(), &output_chunk[0],
                                              output_chunk.size());
    BOOST_REQUIRE(processed > 0);
    std::copy_n(begin(output_chunk), processed, std::back_inserter(unpacked_data));
    while (processed == output_chunk.size())
    {
        processed = zip_stream_unpack_chuck(&ctx, &output_chunk[0], output_chunk.size());
        BOOST_REQUIRE(processed > 0);
        std::copy_n(begin(output_chunk), processed, std::back_inserter(unpacked_data));
    }

    BOOST_REQUIRE(zip_stream_unpack_destroy(&ctx));

    BOOST_REQUIRE_EQUAL(std::string { INPUT_ZIP_DATA }, std::string { (char*)&unpacked_data[0] });
}

BOOST_AUTO_TEST_CASE(data_stream_packing_smaller_chunked_input_check)
{
    const size_t INPUT_CHUNK_SZ = 40;
    const size_t OUTPUT_CHUNK_SZ = 100;

    BOOST_REQUIRE_LT(INPUT_CHUNK_SZ, OUTPUT_CHUNK_SZ);

    std::vector<unsigned char> input_chunk;
    input_chunk.resize(INPUT_CHUNK_SZ);
    size_t rest_sz = sizeof(INPUT_ZIP_DATA);

    zip_stream_ctx_t ctx;
    zip_stream_pack_init(&ctx);

    std::vector<unsigned char> output_chunk;
    output_chunk.resize(OUTPUT_CHUNK_SZ);

    std::vector<unsigned char> packed_data;
    const unsigned char* input_pos = reinterpret_cast<const unsigned char*>(INPUT_ZIP_DATA);

    long processed = 0;
    while (rest_sz > 0)
    {
        auto actual_input_sz = std::min(rest_sz, input_chunk.size());
        memcpy(&input_chunk[0], input_pos, actual_input_sz);

        processed = zip_stream_start_pack_chunk(&ctx, &input_chunk[0], actual_input_sz, &output_chunk[0],
                                                output_chunk.size());
        BOOST_REQUIRE(processed > 0);
        std::copy_n(begin(output_chunk), processed, std::back_inserter(packed_data));
        while (processed == output_chunk.size())
        {
            processed = zip_stream_pack_chunk(&ctx, &output_chunk[0], output_chunk.size());
            BOOST_REQUIRE(processed > 0);
            std::copy_n(begin(output_chunk), processed, std::back_inserter(packed_data));
        }

        processed = zip_stream_finish_pack_chunk(&ctx, &output_chunk[0], output_chunk.size());
        BOOST_REQUIRE(processed >= 0);
        std::copy_n(begin(output_chunk), processed, std::back_inserter(packed_data));

        rest_sz -= actual_input_sz;
        input_pos += actual_input_sz;
    }

    processed = zip_stream_finish_pack(&ctx, &output_chunk[0], output_chunk.size());
    BOOST_REQUIRE(processed >= 0);
    std::copy_n(begin(output_chunk), processed, std::back_inserter(packed_data));

    BOOST_REQUIRE(zip_stream_pack_destroy(&ctx));

    PRINT_ZIP("gzip", sizeof(INPUT_ZIP_DATA), packed_data.size())

    zip_stream_unpack_init(&ctx);
    output_chunk.resize(OUTPUT_CHUNK_SZ, 0);

    std::vector<unsigned char> unpacked_data;

    processed = zip_stream_start_unpack_chuck(&ctx, &packed_data[0], packed_data.size(), &output_chunk[0],
                                              output_chunk.size());
    BOOST_REQUIRE(processed > 0);
    std::copy_n(begin(output_chunk), processed, std::back_inserter(unpacked_data));
    while (processed == output_chunk.size())
    {
        processed = zip_stream_unpack_chuck(&ctx, &output_chunk[0], output_chunk.size());
        BOOST_REQUIRE(processed > 0);
        std::copy_n(begin(output_chunk), processed, std::back_inserter(unpacked_data));
    }

    BOOST_REQUIRE(zip_stream_unpack_destroy(&ctx));

    BOOST_REQUIRE_EQUAL(std::string { INPUT_ZIP_DATA }, std::string { (char*)&unpacked_data[0] });
}

BOOST_AUTO_TEST_CASE(data_stream_diff_buffers_size_check)
{
    const size_t PACK_IN_CHUNK_SZ = 1024;
    const size_t PACK_OUT_CHUNK_SZ = 512;
    const size_t UNPACK_IN_CHUNK_SZ = 256;
    const size_t UNPACK_OUT_CHUNK_SZ = 124;

    BOOST_REQUIRE_GT(sizeof(INPUT_ZIP_DATA), PACK_IN_CHUNK_SZ);

    std::vector<unsigned char> input_chunk;
    std::vector<unsigned char> output_chunk;

    input_chunk.resize(PACK_IN_CHUNK_SZ);
    output_chunk.resize(PACK_OUT_CHUNK_SZ);

    zip_stream_ctx_t ctx;
    zip_stream_pack_init(&ctx);

    size_t rest_sz = sizeof(INPUT_ZIP_DATA);
    const unsigned char* input_pos = reinterpret_cast<const unsigned char*>(INPUT_ZIP_DATA);

    std::vector<unsigned char> packed_data;
    long processed = 0;

    while (rest_sz > 0)
    {
        auto actual_input_sz = std::min(rest_sz, input_chunk.size());
        memcpy(&input_chunk[0], input_pos, actual_input_sz);

        processed = zip_stream_start_pack_chunk(&ctx, &input_chunk[0], actual_input_sz, &output_chunk[0],
                                                output_chunk.size());
        BOOST_REQUIRE(processed > 0);
        std::copy_n(begin(output_chunk), processed, std::back_inserter(packed_data));
        while (processed == output_chunk.size())
        {
            processed = zip_stream_pack_chunk(&ctx, &output_chunk[0], output_chunk.size());
            BOOST_REQUIRE(processed > 0);
            std::copy_n(begin(output_chunk), processed, std::back_inserter(packed_data));
        }

        processed = zip_stream_finish_pack_chunk(&ctx, &output_chunk[0], output_chunk.size());
        BOOST_REQUIRE(processed >= 0);
        std::copy_n(begin(output_chunk), processed, std::back_inserter(packed_data));

        rest_sz -= actual_input_sz;
        input_pos += actual_input_sz;
    }

    processed = zip_stream_finish_pack(&ctx, &output_chunk[0], output_chunk.size());
    BOOST_REQUIRE(processed >= 0);
    std::copy_n(begin(output_chunk), processed, std::back_inserter(packed_data));

    BOOST_REQUIRE(zip_stream_pack_destroy(&ctx));

    PRINT_ZIP("gzip", sizeof(INPUT_ZIP_DATA), packed_data.size())

    rest_sz = packed_data.size();
    input_chunk.resize(UNPACK_IN_CHUNK_SZ, 0);
    output_chunk.resize(UNPACK_OUT_CHUNK_SZ, 0);

    zip_stream_unpack_init(&ctx);

    std::vector<unsigned char> unpacked_data;

    input_pos = reinterpret_cast<const unsigned char*>(&packed_data[0]);

    processed = 0;
    while (rest_sz > 0)
    {
        auto actual_input_sz = std::min(rest_sz, input_chunk.size());
        memcpy(&input_chunk[0], input_pos, actual_input_sz);

        processed = zip_stream_start_unpack_chuck(&ctx, &input_chunk[0], actual_input_sz, &output_chunk[0],
                                                  output_chunk.size());
        BOOST_REQUIRE(processed > 0);
        std::copy_n(begin(output_chunk), processed, std::back_inserter(unpacked_data));
        while (processed == output_chunk.size())
        {
            processed = zip_stream_unpack_chuck(&ctx, &output_chunk[0], output_chunk.size());
            BOOST_REQUIRE(processed >= 0);
            std::copy_n(begin(output_chunk), processed, std::back_inserter(unpacked_data));
        }

        rest_sz -= actual_input_sz;
        input_pos += actual_input_sz;
    }

    BOOST_REQUIRE(zip_stream_unpack_destroy(&ctx));

    BOOST_REQUIRE_EQUAL(std::string { INPUT_ZIP_DATA }, std::string { (char*)&unpacked_data[0] });
}

BOOST_FIXTURE_TEST_CASE(create_gzip_file_check, zip_files_tests)
{
    constexpr size_t CHUNK_SZ = 1024;

    auto input_path = create_file_path();
    {
        std::ofstream out(input_path);
        out << INPUT_ZIP_DATA;
        out.close();
    }

    auto output_path = create_file_path();
    {
        std::ofstream out(output_path);

        FILE* f_input = fopen(input_path.c_str(), "r");
        BOOST_REQUIRE(f_input);

        zip_stream_ctx_t ctx;
        zip_stream_pack_init(&ctx);

        long processed = 0;
        unsigned char input_buf[CHUNK_SZ];
        unsigned char output_buf[CHUNK_SZ];
        size_t result = 0;
        while ((result = fread(input_buf, 1, sizeof input_buf, f_input)) > 0)
        {
            processed = zip_stream_start_pack_chunk(&ctx, input_buf, result, output_buf, sizeof(output_buf));
            BOOST_REQUIRE(processed > 0);
            out.write((char*)output_buf, processed);
            while (processed == sizeof(output_buf))
            {
                processed = zip_stream_pack_chunk(&ctx, output_buf, sizeof(output_buf));
                BOOST_REQUIRE(processed > 0);
                out.write((char*)output_buf, processed);
            }

            processed = zip_stream_finish_pack_chunk(&ctx, output_buf, sizeof(output_buf));
            BOOST_REQUIRE(processed >= 0);
            out.write((char*)output_buf, processed);
        }

        processed = zip_stream_finish_pack(&ctx, output_buf, sizeof(output_buf));
        BOOST_REQUIRE(processed >= 0);
        out.write((char*)output_buf, processed);

        out.close();
        BOOST_REQUIRE(feof(f_input));
    }

    PRINT_ZIP("gzip-file", boost::filesystem::file_size(input_path), boost::filesystem::file_size(output_path))

    std::vector<unsigned char> unpacked_data;
    {
        FILE* f_input = fopen(output_path.c_str(), "r");
        BOOST_REQUIRE(f_input);

        zip_stream_ctx_t ctx;
        zip_stream_unpack_init(&ctx);

        long processed = 0;
        unsigned char input_buf[CHUNK_SZ];
        std::vector<unsigned char> output_chunk;
        output_chunk.resize(CHUNK_SZ);
        size_t result = 0;
        while ((result = fread(input_buf, 1, sizeof input_buf, f_input)) > 0)
        {
            output_chunk.resize(CHUNK_SZ, '*'); // add noise for test only
            processed = zip_stream_start_unpack_chuck(&ctx, input_buf, result, &output_chunk[0], output_chunk.size());
            BOOST_REQUIRE(processed > 0);
            std::copy_n(begin(output_chunk), processed, std::back_inserter(unpacked_data));
            while (processed == output_chunk.size())
            {
                output_chunk.resize(CHUNK_SZ, '*');
                processed = zip_stream_unpack_chuck(&ctx, &output_chunk[0], output_chunk.size());
                BOOST_REQUIRE(processed >= 0);
                std::copy_n(begin(output_chunk), processed, std::back_inserter(unpacked_data));
            }
        }
        BOOST_REQUIRE(feof(f_input));

        BOOST_REQUIRE(zip_stream_unpack_destroy(&ctx));
    }

    //TODO: Fix on Ubuntu 20.04 !!!
    BOOST_CHECK_EQUAL(std::string { INPUT_ZIP_DATA }, std::string { (char*)&unpacked_data[0] });
}

BOOST_AUTO_TEST_SUITE_END()
} // namespace server_clib
