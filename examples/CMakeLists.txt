option ( SERVER_CLIB_BUILD_EXAMPLES "Build examples (ON OR OFF)" ON)

if ( SERVER_CLIB_BUILD_EXAMPLES )

    add_executable( simple_daemon simple_daemon.c )

    add_dependencies( simple_daemon server_clib )
    target_link_libraries( simple_daemon
                           server_clib
                           ${PLATFORM_SPECIFIC_LIBS})

    add_executable( mt_server mt_server.c )

    find_package(Threads REQUIRED)

    add_dependencies( mt_server server_clib )
    target_link_libraries( mt_server
                           server_clib
                           Threads::Threads
                           ${PLATFORM_SPECIFIC_LIBS})
endif()
