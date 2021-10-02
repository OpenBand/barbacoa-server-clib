#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#include <server_clib/app.h>
#include <server_clib/pause.h>

#include <stdnoreturn.h>

static volatile sig_atomic_t job_done = 0;
static char activity_file_path[PATH_MAX] = { 0 };

static void create_activity_file()
{
    char buff[PATH_MAX] = { 0 };

    FILE* f = fopen("/proc/self/comm", "rb");
    if (f == NULL)
        abort();
    size_t sz = fread(buff, sizeof(*buff), sizeof(buff) / sizeof(*buff), f);
    if (sz < 1)
        abort();
    fclose(f);

    char* p = strchr(buff, '\n');
    if (p == NULL)
    {
        p = buff;
        p += sz;
    }
    strcpy(p, ".XXXXXX");
    const char* TMP_DIR = "/tmp/";
    sz = strlen(TMP_DIR);
    p = buff;
    p += sz;
    strcpy(p, buff);
    strncpy(buff, TMP_DIR, sz);

    f = fdopen(mkstemp(buff), "wb");
    if (f == NULL)
        abort();

    strcpy(activity_file_path, buff);

    strcpy(buff, "Hi. I'm daemon Blablabla here\n");
    sz = fwrite(buff, sizeof(*buff), sizeof(buff) / sizeof(*buff), f);
    if (sz < 1)
        abort();

    fclose(f);

    job_done = 1;
}

static void exit_handler(void)
{
    if (job_done)
    {
        int r = unlink(activity_file_path);
        if (r != 0)
            abort();
    }
}

static void signal_handler(int signo)
{
    if (SIGTERM == signo)
    {
        exit_handler(); //_exit doesn't call this

        _exit(exit_code_ok); // 'exit' not allowed for asynchronous signals that are used
    }
}

static void simple_payload(void)
{
    create_activity_file();
    // wait 60 seconds
    srv_c_wpause(60 * 1000);
}

int main(void)
{
    srv_c_app_init_default_signals_should_register();
    srv_c_app_init_daemon(exit_handler, signal_handler, NULL);

    simple_payload();

    return 0;
}
