#include <server_clib/server.h>
#include <server_clib/macro.h>

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <stdnoreturn.h>

#include "priv_macro.h"

#define MAXFD 64
#define ALTER_STACK_BUFF_SZ 4 * SIGSTKSZ

#if !defined(ALTER_STACK_DISABLED)
static char ALTER_STACK_BUFF[ALTER_STACK_BUFF_SZ];
#endif
static server_sig_callback_ft _sig_callback = NULL;
static bool _should_register_signals[NSIG] = { false };
static bool _registred_signals[NSIG] = { true };
static volatile sig_atomic_t _fail_flag = false; // to prevent infinite loop in corrupted callback

static server_sig_callback_ft register_async_signal_impl(int signo, server_sig_callback_ft func)
{
    struct sigaction act, oact;

    act.sa_handler = func;
    sigemptyset(&act.sa_mask);
    act.sa_flags = 0;
    if (signo == SIGALRM)
    {
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUPT;
#endif
    }
    else
    {
#ifdef SA_RESTART
        act.sa_flags |= SA_RESTART;
#endif
    }

#if !defined(ALTER_STACK_DISABLED)
    if (SIGSEGV == signo) // for memory corruption
    {
        stack_t sigstack;
        sigstack.ss_sp = ALTER_STACK_BUFF;
        sigstack.ss_size = sizeof(ALTER_STACK_BUFF);
        sigstack.ss_flags = 0;
        SRV_C_CALL(sigaltstack(&sigstack, NULL) == 0);
        act.sa_flags |= SA_ONSTACK;
    }
#endif

    if (sigaction(signo, &act, &oact) != 0)
        return (SIG_ERR);
    return (oact.sa_handler);
}

static void clear_signal_action(int signo)
{
    SRV_C_ASSERT_TEXT(signal(signo, SIG_IGN) != SIG_ERR, "Can't remove signal hadler");
}

static bool is_signal_registered(int signo)
{
    return signo > 0 && signo < NSIG && _registred_signals[signo];
}

static bool is_signal_should_register(int signo)
{
    return signo > 0 && signo < NSIG && _should_register_signals[signo];
}

static bool register_async_signal(int signo, server_sig_callback_ft func)
{
    if (is_signal_registered(signo))
        return false;

    server_sig_callback_ft ofunc;

    if ((ofunc = register_async_signal_impl(signo, func)) == SIG_ERR)
    {
        SRV_C_ERROR("Can't set signal handler");
    }
    else
    {
        _registred_signals[signo] = true;
    }
    return true;
}

static bool is_final_signal(int signo)
{
    switch (signo)
    {
    case SIGKILL:
    case SIGSTOP:
        return true;
    default:;
    }
    return false;
}

static bool register_blocked_signal(int signo)
{
    if (is_signal_registered(signo))
        return false;

    if (!is_final_signal(signo))
        clear_signal_action(signo);

    sigset_t block_set, prev_set;

    sigemptyset(&block_set);
    sigaddset(&block_set, signo);
    SRV_C_CALL(sigprocmask(SIG_BLOCK, &block_set, &prev_set) == 0);

    _registred_signals[signo] = true;
    return true;
}

static pid_t fork_impl()
{
    pid_t pid = fork();
    SRV_C_ASSERT_TEXT(-1 != pid, "Can't fork");
    return (pid);
}

bool server_is_fail_signal(int signo)
{
    switch (signo)
    {
    case SIGSEGV:
    case SIGBUS:

    case SIGILL:
    case SIGFPE:
    case SIGABRT: // to unify error processing
        return true;
    default:;
    }
    return false;
}

static void not_exit(void)
{
}

static void exit_sig_callback(int signo)
{
    if (!server_is_fail_signal(signo))
        _exit(exit_code_ok);
}

static void wrapped_sig_callback(int signo)
{
    bool hard = server_is_fail_signal(signo);
    if (hard)
    {
        if (_fail_flag)
            _exit(exit_code_error);
        else
        {
            _fail_flag = true;
        }
    }

    if (_sig_callback)
    {
        if (_sig_callback != exit_sig_callback || !hard)
        {
            if (hard)
                atexit(not_exit);

            _sig_callback(signo);
        }
    }

    if (hard)
        _exit(exit_code_error);
}

static void set_signals_callback(server_sig_callback_ft sig_callback, bool hard_only)
{
    _sig_callback = sig_callback;

    for (int sig = 1; sig < NSIG; ++sig)
    {
        if (!is_signal_should_register(sig))
            continue;

        if (!hard_only || server_is_fail_signal(sig))
            register_async_signal(sig, wrapped_sig_callback);
    }
}

static void lock_signals()
{
    for (int sig = 1; sig < NSIG; ++sig)
    {
        if (!is_signal_should_register(sig))
            continue;

        if (!server_is_fail_signal(sig))
            register_blocked_signal(sig);
    }
}

static void daemon_init_impl(server_exit_callback_ft exit_callback)
{
    int i;
    pid_t pid;

    /* magic to avoid parent links */
    if ((pid = fork_impl()) != 0)
        exit(0); /* parent terminates */

    setsid(); /* become session leader */

    register_async_signal(SIGHUP, SIG_IGN);
    if ((pid = fork_impl()) != 0)
        exit(0); /* 1st child terminates */

    /* hook for exit */
    if (exit_callback)
        atexit(exit_callback);

    /* close all custom descriptors */
    for (i = 3; i < MAXFD; i++)
        close(i);

    /* close embedded descriptors */
    int devnull_fd = open("/dev/null", O_RDWR);
    dup2(devnull_fd, 0);
    dup2(devnull_fd, 1);
    dup2(devnull_fd, 2);
    close(devnull_fd);

    SRV_C_CALL(chdir("/") == 0); /* change working directory */

    umask(0); /* clear our file mode creation mask */
}

size_t server_get_alter_stack_size(void)
{
#if !defined(ALTER_STACK_DISABLED)
    return ALTER_STACK_BUFF_SZ;
#else
    return 0;
#endif
}

void server_init_signals_should_register(int* psignos, int sz)
{
    SRV_C_ASSERT_TEXT(psignos, "Invalid signals set");
    SRV_C_ASSERT_TEXT(sz > 0 && sz < NSIG, "Invalid signals set");

    memset(_should_register_signals, false, sizeof(_should_register_signals));
    memset(_registred_signals, true, sizeof(_registred_signals));

    for (int ci = 0; ci < sz; ++ci)
    {
        int signo = psignos[ci];
        SRV_C_ASSERT_TEXT(signo > 0 && signo < NSIG, "Invalid signals set");
        _should_register_signals[signo] = true;
        _registred_signals[signo] = false;
    }
}

void server_init_default_signals_should_register(void)
{
    int signals[] = { SIGTERM, SIGINT, SIGQUIT, SIGABRT, SIGSEGV, SIGFPE, SIGUSR1, SIGUSR2 };

    server_init_signals_should_register(signals, sizeof(signals) / sizeof(int));
}

void SRV_PRIV_C_WRAPPABLE_FUNC(server_init_daemon)(server_exit_callback_ft exit_callback,
                                                   server_sig_callback_ft sig_callback)
{
    daemon_init_impl(exit_callback);

    if (!sig_callback)
        sig_callback = exit_sig_callback;

    set_signals_callback(sig_callback, false);
}

void SRV_PRIV_C_WRAPPABLE_FUNC(server_init)(server_exit_callback_ft exit_callback, server_sig_callback_ft sig_callback)
{
    if (exit_callback)
        atexit(exit_callback);

    if (!sig_callback)
        sig_callback = exit_sig_callback;

    set_signals_callback(sig_callback, false);
}

void SRV_PRIV_C_WRAPPABLE_FUNC(server_mt_init_daemon)(server_exit_callback_ft exit_callback,
                                                      server_sig_callback_ft sig_callback)
{
    daemon_init_impl(exit_callback);

    lock_signals();

    set_signals_callback(sig_callback, true);
}

void SRV_PRIV_C_WRAPPABLE_FUNC(server_mt_init)(server_exit_callback_ft exit_callback,
                                               server_sig_callback_ft sig_callback)
{
    if (exit_callback)
        atexit(exit_callback);

    lock_signals();

    set_signals_callback(sig_callback, true);
}

void server_mt_wait_sig_callback(server_sig_callback_ft sig_callback)
{
    sigset_t wait_set;

    sigemptyset(&wait_set);

    for (int sig = 1; sig < NSIG; ++sig)
    {
        if (!is_signal_should_register(sig))
            continue;

        if (!server_is_fail_signal(sig))
            sigaddset(&wait_set, sig);
    }

    int inbound_sig;

    if (0 != sigwait(&wait_set, &inbound_sig))
    {
        SRV_C_ERROR("Can't wait any signal");
    }

    sig_callback(inbound_sig);
}
