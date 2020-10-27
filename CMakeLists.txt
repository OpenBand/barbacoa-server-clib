project( BarbacoaServerCLib )
cmake_minimum_required( VERSION 2.8.12 )

if (UNIX)

    set(SERVER_CLIB_SOURCES
        "${CMAKE_CURRENT_SOURCE_DIR}/src/server.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/priv_macro.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/pause.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/rubber.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/options.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/jsmn.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/config.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/blowfish.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/hex.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/zip.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/zip_stream.c"
        "${CMAKE_CURRENT_SOURCE_DIR}/src/rnd.c"
    )
    file(GLOB_RECURSE SERVER_CLIB_IMPL_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h")
    file(GLOB_RECURSE SERVER_CLIB_HEADERS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/server_clib/*.h")

    find_package(ZLIB REQUIRED)

    add_library( server_clib
                ${SERVER_CLIB_SOURCES}
                ${SERVER_CLIB_IMPL_HEADERS}
                ${SERVER_CLIB_HEADERS} )
    target_include_directories( server_clib
                            PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include/server_clib"
                            PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include" )
    target_link_libraries( server_clib
                        ${ZLIB_LIBRARIES})

    add_subdirectory( tests )
    add_subdirectory( examples )

else()

    add_library( server_clib INTERFACE )
    target_include_directories( server_clib
                 INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/include" )

endif()
