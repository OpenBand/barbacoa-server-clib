#include <server_clib/app.h>
#include <server_clib/macro.h>

#include <signal.h>
#include <unistd.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <setjmp.h>

#include <stdnoreturn.h>

#include "priv_macro.h"

#define MAXFD 64
#define ALTER_STACK_BUFF_SZ 4 * SIGSTKSZ
#define EXIT_FAILURE_SIG_BASE_ 128

#if !defined(ALTER_STACK_DISABLED)
static char ALTER_STACK_BUFF[ALTER_STACK_BUFF_SZ];
#endif
static srv_c_app_sig_callback_ft _sig_callback = NULL;
static bool _should_register_signals[NSIG] = { false };
static bool _registred_signals[NSIG] = { false };
static volatile sig_atomic_t _fail_flag = false; // to prevent infinite loop in corrupted callback
static volatile sig_atomic_t _sig_initiated = false; // to protect support static data (that not sig_atomic_t)

static volatile sig_atomic_t _fail_can_jump = 0;
static sigjmp_buf _fail_jump_ctx;
static srv_c_app_fail_callback_ft _fail_callback = NULL;
static volatile sig_atomic_t _fail_core_creation_lock = 0;

static srv_c_app_sig_callback_ft register_signal_impl(int signo, srv_c_app_sig_callback_ft func)
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

static void lock_signal_impl(int signo)
{
    sigset_t block_set, prev_set;

    sigemptyset(&block_set);
    sigaddset(&block_set, signo);
    SRV_C_ASSERT_FMT(0 == sigprocmask(SIG_BLOCK, &block_set, &prev_set), "Can't lock signal %d", signo);
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

static bool register_signal(int signo, srv_c_app_sig_callback_ft func)
{
    if (is_signal_registered(signo))
        return false;

    srv_c_app_sig_callback_ft ofunc;

    if ((ofunc = register_signal_impl(signo, func)) == SIG_ERR)
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
    //'volatile' removes optimization magic -O3 (that unfortunately corrupt switch logic for GCC)
    volatile bool result = false;
    switch (signo)
    {
    case SIGKILL:
    case SIGSTOP:
        result = true;
        break;
    default:;
    }
    return result;
}

bool srv_c_is_fail_signal(int signo)
{
    //'volatile' removes optimization magic -O3 (that unfortunately corrupt switch logic for GCC)
    volatile bool result = false;
    switch (signo)
    {
    case SIGSEGV:
    case SIGBUS:
    case SIGILL:
    case SIGFPE:
    case SIGABRT:
    case SIGXCPU:
    case SIGXFSZ:
        result = true;
        break;
    default:;
    }
    return result;
}

static bool register_blocked_signal(int signo)
{
    if (is_signal_registered(signo))
        return false;

    if (!is_final_signal(signo))
        clear_signal_action(signo);

    lock_signal_impl(signo);

    _registred_signals[signo] = true;
    return true;
}

static pid_t fork_impl()
{
    pid_t pid = fork();
    SRV_C_ASSERT_TEXT(-1 != pid, "Can't fork");
    return (pid);
}

static void default_sig_callback(int signo)
{
    if (!srv_c_is_fail_signal(signo))
    {
        // Causes normal program termination to occur without
        // completely cleaning the resources.
        // Use custom callback to fix it
        _exit(exit_code_ok);
    }
    else
    {
        srv_c_app_abort();
    }
}

static void wrapped_sig_callback(int signo)
{
    if (_sig_initiated)
    {
        if (srv_c_is_fail_signal(signo))
        {
            if (_fail_flag)
            {
                //prevent repeated fail signal in fail callback
                if (SIGABRT != signo)
                    srv_c_app_abort();
                return;
            }
            else
            {
                _fail_flag = true;
            }
        }

        if (_sig_callback)
            _sig_callback(signo);

        if (_fail_flag)
        {
            if (_fail_can_jump)
            {
                pid_t pid = fork();
                if (pid > 0)
                {
                    _fail_core_creation_lock = 1;

                    //jump only in parent process
                    siglongjmp(_fail_jump_ctx, 1);
                } //else abort for child or fork error
            }

            srv_c_app_abort();
        }
    }
}

static void set_signals_callback(srv_c_app_sig_callback_ft sig_callback,
                                 bool fails_only,
                                 bool lock_io)
{
    if (!sig_callback)
        sig_callback = default_sig_callback;

    if (lock_io)
    {
        // for silent applications daemonized by demon mode or external orchestrator
        // OS emit SIGPIPE signal if application try to write to STDOUT, STRERR
        srv_c_app_lock_signal(SIGPIPE);
    }

    _sig_callback = sig_callback;

    for (int sig = 1; sig < NSIG; ++sig)
    {
        if (!is_signal_should_register(sig))
            continue;

        if (!fails_only || srv_c_is_fail_signal(sig))
            register_signal(sig, wrapped_sig_callback);
    }
}

static void lock_signals()
{
    for (int sig = 1; sig < NSIG; ++sig)
    {
        if (!is_signal_should_register(sig))
            continue;

        if (!srv_c_is_fail_signal(sig))
            register_blocked_signal(sig);
    }
}

static void daemon_init_impl(srv_c_app_exit_callback_ft exit_callback)
{
    int i;
    pid_t pid;

    /* magic to avoid parent links */
    if ((pid = fork_impl()) != 0)
        exit(0); /* parent terminates */

    setsid(); /* become session leader */

    register_signal(SIGHUP, SIG_IGN);
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

static void fail_callback_init_impl(srv_c_app_fail_callback_ft fail_callback)
{
    if (fail_callback)
    {
        _fail_callback = fail_callback;

        //set jump point if special callback was set
        if (0 == sigsetjmp(_fail_jump_ctx, 0))
        {
            _fail_can_jump = 1;
        }
        else
        {
            _fail_callback();

            srv_c_app_abort();
        }
    }
}

static void app_init_impl(srv_c_app_sig_callback_ft sig_callback,
                          srv_c_app_fail_callback_ft fail_callback,
                          bool lock_io)
{
    _sig_initiated = false;

    set_signals_callback(sig_callback, false, lock_io);

    fail_callback_init_impl(fail_callback);

    _sig_initiated = true;
}

static void app_mt_init_impl(srv_c_app_sig_callback_ft sig_callback,
                             srv_c_app_fail_callback_ft fail_callback,
                             bool lock_io)
{
    _sig_initiated = false;

    //to determine exact thread for signal processing
    //(using srv_c_app_mt_wait_sig_callback) we should lock all
    //processing signals for every thread
    lock_signals();

    //register only fail signals here because
    //others will use 'srv_c_app_mt_wait_sig_callback' technique
    //for multithreaded sake and was locked before
    set_signals_callback(sig_callback, true, lock_io);

    fail_callback_init_impl(fail_callback);

    _sig_initiated = true;
}

size_t srv_c_app_get_alter_stack_size(void)
{
#if !defined(ALTER_STACK_DISABLED)
    return ALTER_STACK_BUFF_SZ;
#else
    return 0;
#endif
}

void srv_c_app_init_signals_should_register(int* psignos, int sz)
{
    SRV_C_ASSERT_TEXT(psignos, "Invalid signals set");
    SRV_C_ASSERT_TEXT(sz > 0 && sz < NSIG, "Invalid signals set");

    _sig_initiated = false;

    MEMSET(_should_register_signals, false, sizeof(_should_register_signals));
    MEMSET(_registred_signals, false, sizeof(_registred_signals));

    for (int ci = 0; ci < sz; ++ci)
    {
        int signo = psignos[ci];
        SRV_C_ASSERT_TEXT(signo > 0 && signo < NSIG, "Invalid signals set");
        _should_register_signals[signo] = true;
    }
}

void srv_c_app_init_default_signals_should_register(void)
{
    int signals[] = { SIGTERM,
                      SIGINT,
                      SIGQUIT,
                      SIGABRT,
                      SIGSEGV,
                      SIGFPE,
                      SIGXCPU,
                      SIGXFSZ,
                      SIGUSR1,
                      SIGUSR2 };

    srv_c_app_init_signals_should_register(signals, sizeof(signals) / sizeof(int));
}

void SRV_PRIV_C_WRAPPABLE_FUNC(srv_c_app_init_daemon)(srv_c_app_exit_callback_ft exit_callback,
                                                      srv_c_app_sig_callback_ft sig_callback,
                                                      srv_c_app_fail_callback_ft fail_callback)
{
    daemon_init_impl(exit_callback);

    app_init_impl(sig_callback, fail_callback, true);
}

void SRV_PRIV_C_WRAPPABLE_FUNC(srv_c_app_init)(srv_c_app_exit_callback_ft exit_callback,
                                               srv_c_app_sig_callback_ft sig_callback,
                                               srv_c_app_fail_callback_ft fail_callback,
                                               bool lock_io)
{
    if (exit_callback)
        atexit(exit_callback);

    app_init_impl(sig_callback, fail_callback, lock_io);
}

void SRV_PRIV_C_WRAPPABLE_FUNC(srv_c_app_mt_init_daemon)(srv_c_app_exit_callback_ft exit_callback,
                                                         srv_c_app_sig_callback_ft sig_callback,
                                                         srv_c_app_fail_callback_ft fail_callback)
{
    daemon_init_impl(exit_callback);

    app_mt_init_impl(sig_callback, fail_callback, true);
}

void SRV_PRIV_C_WRAPPABLE_FUNC(srv_c_app_mt_init)(srv_c_app_exit_callback_ft exit_callback,
                                                  srv_c_app_sig_callback_ft sig_callback,
                                                  srv_c_app_fail_callback_ft fail_callback,
                                                  bool lock_io)
{
    if (exit_callback)
        atexit(exit_callback);

    app_mt_init_impl(sig_callback, fail_callback, lock_io);
}

void srv_c_app_mt_wait_sig_callback(srv_c_app_sig_callback_ft sig_callback)
{
    sigset_t wait_set;

    sigemptyset(&wait_set);

    for (int sig = 1; sig < NSIG; ++sig)
    {
        if (!is_signal_should_register(sig))
            continue;

        if (!srv_c_is_fail_signal(sig))
            sigaddset(&wait_set, sig);
    }

    int inbound_sig;

    if (0 != sigwait(&wait_set, &inbound_sig))
    {
        SRV_C_ERROR("Can't wait any signal");
    }

    sig_callback(inbound_sig);
}

void srv_c_app_abort()
{
    if (!_fail_core_creation_lock)
    {
        if (is_signal_registered(SIGABRT))
        {
            //only for to not receive SIGABRT
            //in our callback
            signal(SIGABRT, SIG_DFL);
        }
        abort();
    }
    else
    {
        _exit(EXIT_FAILURE_SIG_BASE_ + SIGABRT);
    }
}

void srv_c_app_lock_signal(int signo)
{
    if (!is_signal_registered(signo))
        lock_signal_impl(signo);
}

void srv_c_app_unlock_signal(int signo)
{
    if (!is_signal_registered(signo))
    {
        sigset_t pending_set;
        sigpending(&pending_set);
        if (sigismember(&pending_set, signo))
        {
            int inbound_sig;
            sigwait(&pending_set, &inbound_sig); //pop pendign signal
        }

        sigset_t block_set, prev_set;

        sigemptyset(&block_set);
        sigaddset(&block_set, signo);
        sigprocmask(SIG_UNBLOCK, &block_set, &prev_set);
    }
}
