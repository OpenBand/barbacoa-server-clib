#pragma once

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*srv_c_app_exit_callback_ft)(void);
typedef void (*srv_c_app_sig_callback_ft)(int signo);
typedef void (*srv_c_app_fail_callback_ft)(void);

/* Set signals set to handle*/

void srv_c_app_init_signals_should_register(int* psignos, int sz);
void srv_c_app_init_default_signals_should_register(void);

/* srv_c_app_init_daemon, srv_c_app_init - for single thread application.
 * It is used asynchronous signals handlers!
 * Be aware that for multithreaded application the mt_* analog should be used */

void srv_c_app_init_daemon(srv_c_app_exit_callback_ft exit_callback,
                           srv_c_app_sig_callback_ft sig_callback,
                           srv_c_app_fail_callback_ft fail_callback);

void srv_c_app_init(srv_c_app_exit_callback_ft exit_callback,
                    srv_c_app_sig_callback_ft sig_callback,
                    srv_c_app_fail_callback_ft fail_callback,
                    BOOL lock_io);

/* srv_c_app_mt_init_daemon, srv_c_app_mt_init - for multithreaded application.
 * It is used synchronous signal technik */

// sig_callback assume hardware signals only
void srv_c_app_mt_init_daemon(srv_c_app_exit_callback_ft exit_callback,
                              srv_c_app_sig_callback_ft sig_callback,
                              srv_c_app_fail_callback_ft fail_callback);

// sig_callback assume hardware signals only
//
void srv_c_app_mt_init(srv_c_app_exit_callback_ft exit_callback,
                       srv_c_app_sig_callback_ft sig_callback,
                       srv_c_app_fail_callback_ft fail_callback,
                       BOOL lock_io);

// Call this method in special single thread!
// sig_callback assume only NOT hardware signals (SIGTERM, SIGINT, ...)
void srv_c_app_mt_wait_sig_callback(srv_c_app_sig_callback_ft sig_callback);

// Call 'abort' with default signal handler
void srv_c_app_abort();

// Lock signal receiving if it had not already locked
//(by 'srv_c_app_init', 'srv_c_app_mt_init' functions
void srv_c_app_lock_signal(int signo);

// invert 'srv_c_app_lock_signal'
void srv_c_app_unlock_signal(int signo);

/* Info */
size_t srv_c_app_get_alter_stack_size(void);

BOOL srv_c_is_fail_signal(int signo);

#ifdef __cplusplus
}
#endif
