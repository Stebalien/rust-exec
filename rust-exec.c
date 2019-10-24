#include <stdlib.h>
#include <string.h>

#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <err.h>
#include <fcntl.h>

#define FORMAT "/proc/self/fd/%d"
#define BIN_PATH_LEN "/proc/self/fd/%d"

extern char **environ;

int main(int argc, char *argv[]) {
    char dir[] = P_tmpdir "/rust-bin.XXXXXX";
    if (mkdtemp(dir) == NULL) {
        err(1, "Could not create temporary directory");
    }

    char path[sizeof(dir) + 4];
    memcpy(path, dir, sizeof(dir));
    path[sizeof(dir)-1] = '/';
    memcpy(&path[sizeof(dir)], "bin", 4);

    char *rustc_args[] = {"rustc", "-o", path, NULL, NULL, NULL};

    if (strncmp(argv[1], "-O", 2) == 0) {
        argv = &argv[2];
        rustc_args[3] = "-O";
        rustc_args[4] = argv[0];
    } else {
        argv = &argv[1];
        rustc_args[3] = argv[0];
    }

    pid_t child_pid = fork();
    if (child_pid == 0) {
        // is child
        execvp("rustc", rustc_args);
        err(1, "faild to execute the rust compiler");
    } else if (child_pid > 0) {
        // is parent.
        int status;
        if (waitpid(child_pid, &status, 0) < 0) {
            rmdir(dir);
            err(1, "failed to wait for child");
        }
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                unlink(path);
                rmdir(dir);
                return status;
            }
        } else {
            unlink(path);
            rmdir(dir);
            if (WIFSIGNALED(status)) {
                errx(2, "rustc died with signal %d", WTERMSIG(status));
            } else {
                errx(3, "rustc exited for an unknown reason");
            }
        }
    } else {
        rmdir(dir);
        err(1, "failed to fork rustc");
    }

    int fd = open(path, O_RDONLY);
    unlink(path);
    rmdir(dir);
    if (fd < 0) {
        errx(1, "failed to open compiled binary");
    }
    fflush(stdout);
    fflush(stderr);
    fexecve(fd, argv, environ);
    err(1, "Failed to execute program");
}
