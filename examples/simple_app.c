#include <stdlib.h>
#include <signal.h>

#include <server_clib/app.h>
#include <server_clib/pause.h>

#include "log_trace.h"

static void signal_handler(int signo)
{
    LOG_SIG_("Got signal ");
    LOG_SIG_NUMBER(signo);

    if (SIGTERM == signo)
    {
        _exit(exit_code_ok); // 'exit' not allowed for asynchronous signals that are used
    }
    else if (signo == SIGINT)
    {
        LOG_SIG("Only signal SIGTERM from current signal's list opts to quit this application");
    }
}

static void exit_handler(void)
{
    LOG("Got exit");
}

static void simple_payload(void)
{
    // wait 60 seconds
    int tick = 60;
    while (tick-- > 0)
    {
        LOG("Left %d", tick);
        // wait 1 second
        wpause(1000);
    }
    LOG("Stop application");
}

int main(void)
{
    int signals[] = { SIGINT, SIGTERM, SIGUSR1, SIGUSR2, SIGFPE };

    app_init_signals_should_register(signals, sizeof(signals) / sizeof(int));
    app_init(exit_handler, signal_handler, NULL, FALSE);

    simple_payload();

    return 0;
}
