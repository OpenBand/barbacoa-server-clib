#if 0

// https://linux.die.net/man/3/pthread_sigmask

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

/* Simple error handling functions */

#define handle_error_en(en, msg)                                                                                       \
    do                                                                                                                 \
    {                                                                                                                  \
        errno = en;                                                                                                    \
        perror(msg);                                                                                                   \
        exit(EXIT_FAILURE);                                                                                            \
    } while (0)

static void* sig_thread(void* arg)
{
    sigset_t* set = arg;
    int s, sig;

    for (;;)
    {
        s = sigwait(set, &sig);
        if (s != 0)
            handle_error_en(s, "sigwait");

        /* to show in IDE output:
         */
        fprintf(stderr, "Signal handling thread got signal %d\n", sig);
    }
}

int main(int argc, char* argv[])
{
    pthread_t thread;
    sigset_t set;
    int s;

    /* Block SIGQUIT and SIGUSR1; other threads created by main()
        will inherit a copy of the signal mask. */

    sigemptyset(&set);
    sigaddset(&set, SIGTERM);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGUSR1);
    s = pthread_sigmask(SIG_BLOCK, &set, NULL);
    if (s != 0)
        handle_error_en(s, "pthread_sigmask");

    s = pthread_create(&thread, NULL, &sig_thread, (void*)&set);
    if (s != 0)
        handle_error_en(s, "pthread_create");

    /* Main thread carries on to create other threads and/or do
        other work */

    pause(); /* Dummy pause so we can test program */
}

#else
#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>

#include <server_clib/server.h>
#include <server_clib/pause.h>

#include <stdnoreturn.h>

static noreturn void exit_handler(void)
{
    fprintf(stderr, "Process got exit\n");

    _exit(exit_code_ok);
}

static noreturn void signal_handler(int signo)
{
    fprintf(stderr, "Got signal %d\n", signo);

    if (SIGTERM == signo)
        exit(exit_code_ok); // allowed for synchronous signal that was used
}

static void* sig_thread(__attribute__((unused)) void* arg)
{
    for (;;)
    {
        server_mt_wait_sig_callback(signal_handler);
        fprintf(stderr, "Signal handling thread is whaiting for next signal\n");
    }
}

int main(void)
{
    pthread_t thread;

    int signals[] = { SIGTERM, SIGUSR1, SIGUSR2 };

    server_init_signals_should_register(signals, sizeof(signals) / sizeof(int));

    server_mt_init(exit_handler, signal_handler);

    SRV_C_CALL(pthread_create(&thread, NULL, &sig_thread, NULL) == 0);

    pause();
}
#endif
