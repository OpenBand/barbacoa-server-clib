#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/syscall.h>

#include <server_clib/app.h>
#include <server_clib/pause.h>
#include <server_clib/macro.h>

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
    LOG("Got exit in thread %lu", (unsigned long)syscall(SYS_gettid));

    _exit(exit_code_ok);
}

static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

static void* sig_thread(__attribute__((unused)) void* arg)
{
    LOG_THREAD(mtx, "Signal handling thread %lu has started",
               (unsigned long)syscall(SYS_gettid));
    for (;;)
    {
        app_mt_wait_sig_callback(signal_handler);
        LOG_THREAD(mtx, "Signal handling thread %lu is waiting for next signal",
                   (unsigned long)syscall(SYS_gettid));
    }
}

static void* job_thread(__attribute__((unused)) void* arg)
{
    // wait 60 seconds
    int tick = 60;
    while (tick-- > 0)
    {
        LOG_THREAD(mtx, "Job %lu left %d", (unsigned long)syscall(SYS_gettid), tick);
        // wait 1 second
        wpause(1000);
    }
    LOG_THREAD(mtx, "Job %lu stops application", (unsigned long)syscall(SYS_gettid));
    exit(exit_code_ok);
}

int main(void)
{
    pthread_t thread0, thread1, thread2, thread3;

    int signals[] = { SIGINT, SIGTERM, SIGUSR1, SIGUSR2, SIGFPE };

    app_init_signals_should_register(signals, sizeof(signals) / sizeof(int));

    app_mt_init(exit_handler, signal_handler, NULL, TRUE);

    SRV_C_CALL(pthread_create(&thread0, NULL, &sig_thread, NULL) == 0);
    SRV_C_CALL(pthread_create(&thread1, NULL, &job_thread, NULL) == 0);
    SRV_C_CALL(pthread_create(&thread2, NULL, &job_thread, NULL) == 0);
    wpause(500);
    SRV_C_CALL(pthread_create(&thread3, NULL, &job_thread, NULL) == 0);

    SRV_C_CALL(pthread_join(thread0, NULL) == 0);
    SRV_C_CALL(pthread_join(thread1, NULL) == 0);
    SRV_C_CALL(pthread_join(thread2, NULL) == 0);
    SRV_C_CALL(pthread_join(thread3, NULL) == 0);

    return 0;
}
