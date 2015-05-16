// RSHELL.H
// Header for rshell

#ifndef RSHELL_H
#define RSHELL_H


struct redir {
    redir(int t) : type(t) {}
    redir(int r, int l, int t) : type(t) { pipefd[0] = r; pipefd[1] = l; }
    int type;
    int pipefd[2] = { 0, 1 };
    const char* file;
    int file_fd;
};


void init();
int run();

std::string prompt();

int execute(const char* path, char* const argv[]);
int execute(struct redir* redir_info, int* fd_fwd, const char* path, char* const argv[]);

std::vector<std::string> tokenize(std::string);
std::vector<std::string> tokenize(std::string, std::string);
std::vector<std::string> toksplit(std::string, std::string);

#endif
