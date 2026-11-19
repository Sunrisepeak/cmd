/* glibcmd.h first on every platform: the portable declarations come
 * from it even when this guard compiles the file away entirely, and
 * the translation unit then stays non-empty in ISO C90 (System V cc
 * rejects empty preprocessed files). */
#include "glibcmd.h"
#if !defined(_WIN32) && !defined(_WIN64) && !defined(_MSC_VER)
/*
 * lexec.c - process execution
 *
 * License: GNU GPLv3
 */


#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

/* -------------------------------------------------------------------------
 * Internal helpers
 * ---------------------------------------------------------------------- */

static void fill_exit_info(int wstatus, libcmd_exit_info_t *info)
{
    if (info == NULL)
        return;
    info->exited   = 0;
    info->exit_code = 0;
    info->signaled = 0;
    info->signal   = 0;

    if (WIFEXITED(wstatus)) {
        info->exited    = 1;
        info->exit_code = WEXITSTATUS(wstatus);
    } else if (WIFSIGNALED(wstatus)) {
        info->signaled = 1;
        info->signal   = WTERMSIG(wstatus);
        info->exit_code = 128 + WTERMSIG(wstatus);
    }
}

static void child_setup_fds(int stdin_fd, int stdout_fd, int stderr_fd)
{
    if (stdin_fd >= 0 && stdin_fd != STDIN_FILENO) {
        dup2(stdin_fd, STDIN_FILENO);
        close(stdin_fd);
    }
    if (stdout_fd >= 0 && stdout_fd != STDOUT_FILENO) {
        dup2(stdout_fd, STDOUT_FILENO);
        close(stdout_fd);
    }
    if (stderr_fd >= 0 && stderr_fd != STDERR_FILENO) {
        dup2(stderr_fd, STDERR_FILENO);
        close(stderr_fd);
    }
}

static void child_redirect_to_null(int target)
{
    int flags = (target == STDIN_FILENO) ? O_RDONLY : O_WRONLY;
    int fd = open("/dev/null", flags);
    if (fd < 0)
        return;
    dup2(fd, target);
    close(fd);
}

#if defined(CMD_LIBC_MUSL)
/* openkal's C library composes fork upon openkal.space, which a system that
 * starts only named programs (Windows) does not have: fork reports ENOSYS
 * there, and posix_spawn starts the program instead.  It places the three
 * streams; a program START detaches forms a group of its own (setsid is not
 * answered there), and a priority START asks for is not applied. */
#include <spawn.h>
#include <openkal/env.h>

extern char **environ;

/* Whether the system's conventions are Windows': openkal states them
 * (kal_env_system), and its C library searches by the same rule. */
static int windows_conventions(void)
{
    char name[16];

    return kal_env_system(name, sizeof(name)) == 7 &&
           memcmp(name, "windows", 7) == 0;
}

/* Argument zero in Windows' spelling, in ZERO: cmd turned a typed '\'
 * into '/' to find the program, and a Windows program reads its own name
 * from its command line, where cmd.exe takes a '/' for a switch.  Returns
 * ARGV itself elsewhere, and a copy of it to free here; NULL when no copy
 * could be made. */
static char **system_argv(char *const argv[], char *zero, size_t size)
{
    char **copy;
    size_t n;
    char *p;

    if (!windows_conventions() || argv[0] == NULL ||
        strchr(argv[0], '/') == NULL || strlen(argv[0]) >= size)
        return (char **)argv;
    for (n = 0; argv[n] != NULL; n++)
        ;
    copy = (char **)malloc((n + 1) * sizeof(*copy));
    if (copy == NULL)
        return NULL;
    memcpy(copy, argv, (n + 1) * sizeof(*copy));
    strcpy(zero, argv[0]);
    for (p = zero; *p != '\0'; p++)
        if (*p == '/')
            *p = '\\';
    copy[0] = zero;
    return copy;
}

static int spawn_program(const char *path,
                         char *const argv[],
                         char *const envp[],
                         const int fds[3],
                         int detached,
                         pid_t *pid)
{
    posix_spawn_file_actions_t actions;
    posix_spawnattr_t attr;
    char zero[4096];
    char **args;
    int i, r;

    args = system_argv(argv, zero, sizeof(zero));
    if (args == NULL)
        return -1;
    posix_spawn_file_actions_init(&actions);
    posix_spawnattr_init(&attr);
    for (i = 0; i < 3; i++) {
        if (fds[i] >= 0 && fds[i] != i)
            posix_spawn_file_actions_adddup2(&actions, fds[i], i);
        else if (fds[i] < 0 && detached)
            posix_spawn_file_actions_addopen(&actions, i, "/dev/null",
                                             i == 0 ? O_RDONLY : O_WRONLY, 0);
    }
    if (detached) {
        posix_spawnattr_setflags(&attr, POSIX_SPAWN_SETPGROUP);
        posix_spawnattr_setpgroup(&attr, 0);
    }
    r = posix_spawn(pid, path, &actions, &attr, args,
                    envp != NULL ? envp : environ);
    posix_spawnattr_destroy(&attr);
    posix_spawn_file_actions_destroy(&actions);
    if (args != (char **)argv)
        free(args);
    if (r != 0) {
        errno = r;
        return -1;
    }
    return 0;
}

#else
#define windows_conventions() 0
#endif

/* Whether NAME is a file this process may start.  access(X_OK) asks the
 * system that decides: the owner's execute bit is the wrong question for
 * another user's file, and a system that keeps no execute permission
 * (Windows, under openkal's C library) reports none in stat while access
 * answers yes. */
static int is_runnable(const char *name)
{
    struct stat st;

    return stat(name, &st) == 0 && S_ISREG(st.st_mode) &&
           access(name, X_OK) == 0;
}

/* -------------------------------------------------------------------------
 * Raw fork / exit (used by cparser.c to run pipelines concurrently)
 * ---------------------------------------------------------------------- */

int libcmd_fork(void)
{
    return (int)fork();
}

void libcmd_exit(int status)
{
    _exit(status);
}

/* -------------------------------------------------------------------------
 * Public API
 * ---------------------------------------------------------------------- */

int libcmd_exec_sync(const char *path,
                     char *const argv[],
                     char *const envp[],
                     int stdin_fd,
                     int stdout_fd,
                     int stderr_fd,
                     int nice_level,
                     libcmd_exit_info_t *exit_info)
{
    pid_t pid;
    int wstatus;

    pid = fork();
#if defined(CMD_LIBC_MUSL)
    if (pid < 0 && errno == ENOSYS) {
        int fds[3];

        fds[0] = stdin_fd;
        fds[1] = stdout_fd;
        fds[2] = stderr_fd;
        if (spawn_program(path, argv, envp, fds, 0, &pid) < 0)
            return -1;
    }
#endif
    if (pid < 0)
        return -1;

    if (pid == 0) {
        /* Child */
        libcmd_set_process_priority(nice_level);
        child_setup_fds(stdin_fd, stdout_fd, stderr_fd);
        if (envp)
            execve(path, argv, envp);
        else
            execv(path, argv);
        /* If we get here, exec failed */
        _exit(127);
    }

    /* Parent */
    while (waitpid(pid, &wstatus, 0) < 0) {
        if (errno != EINTR)
            return -1;
    }

    fill_exit_info(wstatus, exit_info);
    return 0;
}

int libcmd_exec_async(const char *path,
                      char *const argv[],
                      char *const envp[],
                      int stdin_fd,
                      int stdout_fd,
                      int stderr_fd,
                      int nice_level)
{
    pid_t pid;

    pid = fork();
#if defined(CMD_LIBC_MUSL)
    if (pid < 0 && errno == ENOSYS) {
        int fds[3];

        fds[0] = stdin_fd;
        fds[1] = stdout_fd;
        fds[2] = stderr_fd;
        if (spawn_program(path, argv, envp, fds, 1, &pid) < 0)
            return -1;
    }
#endif
    if (pid < 0)
        return -1;

    if (pid == 0) {
        /* Child: detach from the shell's controlling terminal and process
         * group, so the started program cannot steal tty input, stop the
         * whole foreground group, or terminate the session (Windows START
         * semantics: program runs in its own "window"). */
        setsid();
        libcmd_set_process_priority(nice_level);
        child_setup_fds(stdin_fd, stdout_fd, stderr_fd);
        if (stdin_fd < 0)  child_redirect_to_null(STDIN_FILENO);
        if (stdout_fd < 0) child_redirect_to_null(STDOUT_FILENO);
        if (stderr_fd < 0) child_redirect_to_null(STDERR_FILENO);
        if (envp)
            execve(path, argv, envp);
        else
            execv(path, argv);
        _exit(127);
    }

    return (int)pid;
}

int libcmd_wait_pid(int pid, libcmd_exit_info_t *exit_info)
{
    int wstatus;

    while (waitpid((pid_t)pid, &wstatus, 0) < 0) {
        if (errno != EINTR)
            return -1;
    }

    fill_exit_info(wstatus, exit_info);
    return 0;
}

/* Tries DIR/NAME then SUFFIX (DIR is LEN long; none when LEN is 0) and
 * writes the name to OUT when it may be started. */
static int try_candidate(const char *dir, size_t len,
                         const char *name,
                         const char *suffix, size_t suffix_len,
                         char *out, size_t out_size)
{
    char candidate[4096];
    size_t n = len, name_len = strlen(name);

    if (len + 1 + name_len + suffix_len + 1 > sizeof(candidate))
        return -1;
    memcpy(candidate, dir, len);
    if (len > 0)
        candidate[n++] = '/';
    memcpy(candidate + n, name, name_len);
    memcpy(candidate + n + name_len, suffix, suffix_len);
    candidate[n + name_len + suffix_len] = '\0';
    if (!is_runnable(candidate))
        return -1;
    return libcmd_sprintf_s(out, out_size, "%s", candidate) >= 0 ? 0 : -1;
}

/* Tries NAME in DIR as the system would: as given, or, under Windows'
 * conventions and for a name without a suffix, with each suffix PATHEXT
 * lists in its order and not as given (Windows' own rule). */
static int try_in_dir(const char *dir, size_t len, const char *name, int win,
                      char *out, size_t out_size)
{
    const char *suffix, *end;

    if (!win || strchr(name, '.') != NULL)
        return try_candidate(dir, len, name, "", 0, out, out_size);
    suffix = getenv("PATHEXT");
    if (suffix == NULL)
        suffix = ".COM;.EXE;.BAT;.CMD";
    while (*suffix != '\0') {
        end = strchr(suffix, ';');
        if (end == NULL)
            end = suffix + strlen(suffix);
        if (end > suffix &&
            try_candidate(dir, len, name, suffix, (size_t)(end - suffix),
                          out, out_size) == 0)
            return 0;
        suffix = (*end == ';') ? end + 1 : end;
    }
    return -1;
}

int libcmd_find_exec(const char *name,
                     const char *path_env,
                     char *out,
                     size_t out_size)
{
    const char *start, *end;
    int win = windows_conventions();
    char sep = win ? ';' : ':';

    if (name == NULL || out == NULL || out_size == 0)
        return -1;

    /* A name with a directory (or, under Windows' conventions, a volume)
     * is not searched for */
    if (strchr(name, '/') != NULL || (win && strchr(name, ':') != NULL))
        return try_in_dir("", 0, name, win, out, out_size);

    if (path_env == NULL)
        path_env = win ? "" : "/usr/local/bin:/usr/bin:/bin";

    /* Windows cmd semantics: the current directory is searched before
     * PATH, so "build.sh" runs without a "./" prefix after cd. */
    if (try_in_dir(".", 1, name, win, out, out_size) == 0)
        return 0;

    start = path_env;
    while (start && *start) {
        end = strchr(start, sep);
        if (end == NULL)
            end = start + strlen(start);

        if (end > start &&
            try_in_dir(start, (size_t)(end - start), name, win,
                       out, out_size) == 0)
            return 0;

        start = (*end == sep) ? end + 1 : NULL;
    }

    return -1;
}

int libcmd_exec_pipeline(char *const *const *cmds,
                         const char *const *paths,
                         int n,
                         char *const envp[],
                         int stdin_fd,
                         int stdout_fd,
                         libcmd_exit_info_t *exit_info)
{
    int i;
    int prev_read = -1;
    int *pids;
    int wstatus;

    if (n <= 0 || cmds == NULL || paths == NULL)
        return -1;

    pids = (int *)malloc((size_t)n * sizeof(int));
    if (pids == NULL)
        return -1;

    for (i = 0; i < n; i++) {
        int pipe_fds[2] = {-1, -1};
        int child_stdin, child_stdout;
        pid_t pid;

        /* Create pipe for all but the last command */
        if (i < n - 1) {
            if (libcmd_pipe(pipe_fds) < 0) {
                free(pids);
                return -1;
            }
        }

        child_stdin  = (i == 0)     ? stdin_fd  : prev_read;
        child_stdout = (i == n - 1) ? stdout_fd : pipe_fds[1];

        pid = fork();
        if (pid < 0) {
            if (pipe_fds[0] >= 0) close(pipe_fds[0]);
            if (pipe_fds[1] >= 0) close(pipe_fds[1]);
            free(pids);
            return -1;
        }

        if (pid == 0) {
            /* Child: close unused pipe ends */
            if (pipe_fds[0] >= 0) close(pipe_fds[0]);
            child_setup_fds(child_stdin, child_stdout, -1);
            if (envp)
                execve(paths[i], cmds[i], envp);
            else
                execv(paths[i], cmds[i]);
            _exit(127);
        }

        /* Parent */
        pids[i] = (int)pid;
        if (prev_read >= 0) close(prev_read);
        if (pipe_fds[1] >= 0) close(pipe_fds[1]);
        prev_read = pipe_fds[0];
    }

    /* Wait for all children */
    for (i = 0; i < n; i++) {
        while (waitpid((pid_t)pids[i], &wstatus, 0) < 0) {
            if (errno != EINTR) break;
        }
        /* Only capture exit info of the last command */
        if (i == n - 1)
            fill_exit_info(wstatus, exit_info);
    }

    free(pids);
    return 0;
}

/* -------------------------------------------------------------------------
 * popen / pclose wrappers
 * ---------------------------------------------------------------------- */

FILE *libcmd_popen(const char *cmd, const char *mode)
{
    return popen(cmd, mode);
}

int libcmd_pclose(FILE *stream)
{
    if (stream == NULL)
        return -1;
    return pclose(stream);
}

/* -------------------------------------------------------------------------
 * New-console spawn (START "command")
 * ---------------------------------------------------------------------- */

int libcmd_exec_console(char *const argv[], char *const envp[])
{
    /* POSIX has no "new console window" primitive; START keeps its
     * x-terminal-emulator fallback on Unix. */
    (void)argv;
    (void)envp;
    return -1;
}
#else
int lexec_c_dummy;
#endif
