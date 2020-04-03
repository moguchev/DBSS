#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sysexits.h>
#include <unistd.h>

#include <string>

const std::string DEFAULT = "std";

int Run(const char* program, char** argv,
    const std::string& out_file,
    const std::string& err_file
) {
    int pipefds[2];
    if (pipe(pipefds)) {
        perror("pipe");
        return -1;
    }
    
    // выставляем флаг close-on-exec на write конце
    if (fcntl(pipefds[1], F_SETFD, fcntl(pipefds[1], F_GETFD) | FD_CLOEXEC)) {
        perror("fcntl");
        return -1;
    }
 
    pid_t pid = fork();
 
    if (pid == -1) {
        perror("fork");
        return -1;
    } else if (pid > 0) {
        // printf("P> I am parent %d\n", getpid());
        // printf("P> Child is %d\n", pid);
        
        // закрываем write конец канала
        close(pipefds[1]);
 
        ssize_t count;
        int error;
        // читаем из read конца канала
        while ((count = read(pipefds[0], &error, sizeof(errno))) == -1) {
            // блокируемся, пытаясь прочитатать из read конца
            if (errno != EAGAIN && errno != EINTR) {
                // блогкировка снята, мы прочитали.
                break;
            }
        }
        // если прочитали ненулевое число(байт),
        // значит exec в дочернем процессе не выполнился
        if (count) {
            // printf("P> child's exec: %s\n", strerror(err));
            return -1;
        }
        // Закрываем read конец канала
        close(pipefds[0]);
 
        // printf("P> Waiting for child...\n");
        int status;
        // wait(&status);
        waitpid(pid, &status, 0);
        // printf("P> Wait OK \n");
        if (WIFEXITED(status)) {
            if (out_file != DEFAULT) {
                fprintf(stdout, "Exit code: %d\n", WEXITSTATUS(status));
            }
            if (err_file != DEFAULT && err_file != out_file) {
                fprintf(stderr, "Exit code: %d\n", WEXITSTATUS(status));
            }
            return WEXITSTATUS(status);
        }
    } else {
        // Дочерний процесс
        // printf("C> I am child %d of %d\n", getpid(), getppid());
        // Закрываем read конец канала
        close(pipefds[0]);

        if (execv(program, argv) == -1) {
            perror("exec");
            // если словили ошибку exec (не сработал)
            // пишем в канал код ошибки
            write(pipefds[1], &errno, sizeof(int));
            _exit(EXIT_SUCCESS);
        }
    }
}



int main(int argc, char** argv) {
    if (argc < 5) {
        perror("too fiew arguments");
        return EXIT_FAILURE;
    }
    std::string in = argv[1];
    std::string out = argv[2];
    std::string err = argv[3];
    std::string program = argv[4];
    char** args;
    size_t argsc = argc - 4;
    FILE* in_file;
    FILE* out_file;
    FILE* err_file;
    FILE* console = fdopen(dup(STDOUT_FILENO), "w");
  
    args = new char* [argsc];
    for (int i = 0; i < argsc; i++) {
        args[i] = argv[i + 4];
    }

    if (in != DEFAULT) {
        in_file = freopen(in.c_str(), "r", stdin);
        if (in_file == NULL) {
            perror("freopen() failed");
            return EXIT_FAILURE;
        }
    }

    if (out != DEFAULT) {
        out_file = freopen(out.c_str(), "w", stdout);
        if (out_file == NULL) {
            perror("freopen() failed");
            return EXIT_FAILURE;
        }
    }

    if (err != DEFAULT) {
        if (err == out) {
            dup2(fileno(out_file), fileno(stderr));
        } else {
            err_file = freopen(err.c_str(), "w", stderr);
            if (err_file == NULL) {
                perror("freopen() failed");
                return EXIT_FAILURE;
            }
        } 
    }

    int code = Run(program.c_str(), args, out, err);
    fprintf(console, "Exit code: %d\n", code);

    if (out != DEFAULT) {
        fclose(out_file);
        fclose(console);
    }
    if (err != DEFAULT && err != out) {
        fclose(err_file);
    }
    delete[] args;
 
    return EXIT_SUCCESS;
}