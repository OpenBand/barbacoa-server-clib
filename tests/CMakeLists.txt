option ( SERVER_CLIB_BUILD_TESTS "Build tests (ON OR OFF)" OFF)

if ( SERVER_CLIB_BUILD_TESTS )

    set(CMAKE_CXX_STANDARD 14)

    if ( NOT BOOST_VERSION_MIN )
        set(BOOST_VERSION_MIN "1.53")
    endif()

    set(BOOST_COMPONENTS)
    list(APPEND BOOST_COMPONENTS system
                                 thread
                                 filesystem
                                 date_time
                                 unit_test_framework)
    set( Boost_USE_STATIC_LIBS ON CACHE STRING "ON or OFF" )
    set( Boost_USE_MULTITHREADED ON CACHE STRING "ON or OFF" )

    find_package(Boost ${BOOST_VERSION_MIN} REQUIRED COMPONENTS ${BOOST_COMPONENTS})

    file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")
    file(GLOB_RECURSE HEADERS "${CMAKE_CURRENT_SOURCE_DIR}/*.h")

    add_executable(server_clib_tests ${SOURCES} ${HEADERS})

    add_dependencies( server_clib_tests
                      server_clib)
    target_include_directories( server_clib_tests
                           PRIVATE "${Boost_INCLUDE_DIR}"
                           PRIVATE "${Boost_INCLUDE_DIR}")
    target_link_libraries( server_clib_tests
                           server_clib
                           ${Boost_LIBRARIES}
                           ${PLATFORM_SPECIFIC_LIBS})
endif()
