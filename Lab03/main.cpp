#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <sys/wait.h>
#include <sysexits.h>
#include <unistd.h>
 
void Run(const char* program, const char* arg) {
    printf("P> Running %s %s\n", program, arg);
 
    int pipefds[2];
    if (pipe(pipefds)) {
        perror("pipe");
        return;
    }
 
    if (fcntl(pipefds[1], F_SETFD, fcntl(pipefds[1], F_GETFD) | FD_CLOEXEC)) {
        perror("fcntl");
        return;
    }
 
    pid_t pid = fork();
 
    if (pid == -1) {
        perror("fork");
        return;
    } else if (pid > 0) {
        printf("P> I am parent %d\n", getpid());
        printf("P> Child is %d\n", pid);
 
        close(pipefds[1]);
 
        ssize_t count;
        int err;
        while ((count = read(pipefds[0], &err, sizeof(errno))) == -1) {
            if (errno != EAGAIN && errno != EINTR) {
                break;
            }
        }
        if (count) {
            printf("P> child's execvp: %s\n", strerror(err));
            return;
        }
        close(pipefds[0]);
 
        printf("P> Waiting for child...\n");
        int status;
        // wait(&status);
        waitpid(pid, &status, 0);
        printf("P> Wait OK\n");
        if (WIFEXITED(status)) {
            printf("P> Exit code = %d\n", WEXITSTATUS(status));
        }
        printf("\n");
    } else {
        // we are the child
        printf("C> I am child %d of %d\n", getpid(), getppid());
 
        close(pipefds[0]);
        if (execlp(program, program, arg, NULL) == -1) {
            perror("exec");
            write(pipefds[1], &errno, sizeof(int));
            _exit(0);
        }
    }
}
 
int main(int argc, char* argv[]) {
    Run("./hello", "");
    return 0;
}