// RSHELL.H
// Header for rshell

#ifndef RSHELL_H
#define RSHELL_H


struct redir {
    redir(int t) : type(t) {}
    redir(int r, int l, int t) : type(t) { pipefd[0] = r; pipefd[1] = l; }
    int type; // redirection type
    int pipefd[2] = { 0, 1 }; // fds of outgoing pipe
    const char* file; // input file path
    int file_fd; // fd of input file
    int out_fd = STDOUT_FILENO; // fd to link to pipe, default is 1 (stdout)
};


void init();
int run();

std::string prompt();

int changedir(char* const arg);

int execute(const char* path, char* const argv[]);
int execute(struct redir* redir_info, int* fd_fwd, const char* path, char* const argv[]);

std::vector<std::string> tokenize(std::string);
std::vector<std::string> tokenize(std::string, std::string);
std::vector<std::string> toksplit(std::string, std::string);

#endif
