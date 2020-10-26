#pragma once

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif
typedef void (*server_exit_callback_ft)(void);
typedef void (*server_sig_callback_ft)(int signo);

/* Set signals set to handle*/

void server_init_signals_should_register(int* psignos, int sz);
void server_init_default_signals_should_register(void);

size_t server_get_alter_stack_size(void);

/* For single thread process. It is used asynchronous signals handlers!
 * Be aware that for multithreaded server the mt_* analog should be used */

void server_init_daemon(server_exit_callback_ft exit_callback, server_sig_callback_ft sig_callback);

void server_init(server_exit_callback_ft exit_callback, server_sig_callback_ft sig_callback);

BOOL server_is_fail_signal(int signo);

/* For multithreaded (mt_*) servers. It is used synchronous signal technik */

// sig_callback assume hardware signals only
void server_mt_init_daemon(server_exit_callback_ft exit_callback, server_sig_callback_ft sig_callback);

// sig_callback assume hardware signals only
void server_mt_init(server_exit_callback_ft exit_callback, server_sig_callback_ft sig_callback);

// Call this method in special single thread!
// sig_callback assume only NOT hardware signals (SIGTERM, SIGINT, ...)
void server_mt_wait_sig_callback(server_sig_callback_ft sig_callback);

#ifdef __cplusplus
}
#endif
