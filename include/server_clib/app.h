#pragma once

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*app_exit_callback_ft)(void);
typedef void (*app_sig_callback_ft)(int signo);
typedef void (*app_fail_callback_ft)(void);

/* Set signals set to handle*/

void app_init_signals_should_register(int* psignos, int sz);
void app_init_default_signals_should_register(void);

size_t app_get_alter_stack_size(void);

BOOL app_has_fail_signal(int signo);

/* app_init_daemon, app_init - for single thread application.
 * It is used asynchronous signals handlers!
 * Be aware that for multithreaded application the mt_* analog should be used */

void app_init_daemon(app_exit_callback_ft exit_callback,
                     app_sig_callback_ft sig_callback,
                     app_fail_callback_ft fail_callback);

void app_init(app_exit_callback_ft exit_callback,
              app_sig_callback_ft sig_callback,
              app_fail_callback_ft fail_callback,
              BOOL lock_io);

/* app_mt_init_daemon, app_mt_init - for multithreaded application.
 * It is used synchronous signal technik */

// sig_callback assume hardware signals only
void app_mt_init_daemon(app_exit_callback_ft exit_callback,
                        app_sig_callback_ft sig_callback,
                        app_fail_callback_ft fail_callback);

// sig_callback assume hardware signals only
//
void app_mt_init(app_exit_callback_ft exit_callback,
                 app_sig_callback_ft sig_callback,
                 app_fail_callback_ft fail_callback,
                 BOOL lock_io);

// Call this method in special single thread!
// sig_callback assume only NOT hardware signals (SIGTERM, SIGINT, ...)
void app_mt_wait_sig_callback(app_sig_callback_ft sig_callback);

// Call 'abort' with default signal handler
void app_abort();

// Lock signal receiving if it had not already locked
//(by 'app_init', 'app_mt_init' functions
void lock_signal(int signo);

// invert 'lock_signal'
void unlock_signal(int signo);

#ifdef __cplusplus
}
#endif
