option ( SERVER_CLIB_BUILD_EXAMPLES "Build examples (ON OR OFF)" OFF)

if ( SERVER_CLIB_BUILD_EXAMPLES )

    add_executable( simple_app simple_app.c log_trace.h )

    add_dependencies( simple_app server_clib )
    target_link_libraries( simple_app
                           server_clib
                           ${PLATFORM_SPECIFIC_LIBS})

    add_executable( simple_daemon simple_daemon.c )

    add_dependencies( simple_daemon server_clib )
    target_link_libraries( simple_daemon
                           server_clib
                           ${PLATFORM_SPECIFIC_LIBS})

    add_executable( mt_app mt_app.c log_trace.h )

    find_package(Threads REQUIRED)

    add_dependencies( mt_app server_clib )
    target_link_libraries( mt_app
                           server_clib
                           Threads::Threads
                           ${PLATFORM_SPECIFIC_LIBS})
endif()
