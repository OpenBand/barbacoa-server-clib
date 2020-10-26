#include <stdlib.h>

#include <server_clib/server.h>
#include <server_clib/pause.h>

#include <stdnoreturn.h>

static noreturn void simple_die(void)
{
    _exit(exit_code_ok);
}

static noreturn void exit_handler(void)
{
    simple_die();
}

static void simple_payload(void)
{
    // wait 30 seconds
    wpause(30 * 1000);
}

int main(void)
{
    server_init_default_signals_should_register();
    server_init_daemon(exit_handler, NULL);

    simple_payload();

    return 0;
}
