// RSHELL.H
// Header for rshell

#ifndef RSHELL_H
#define RSHELL_H


struct redir {
    redir(int t) : type(t) {}
    redir(int r, int l, int t) : type(t) { pids[0] = r; pids[1] = l; }
    int type;
    int pipefd[2] = { 0, 1 };
    int pids[2];
    const char* redir_file;
};


void init();
int run();

std::string prompt();

int execute(const char* path, char* const argv[]);
int execute(struct redir* pipe, int* fd_fwd, const char* path, char* const argv[]);

std::vector<std::string> tokenize(std::string);
std::vector<std::string> tokenize(std::string, std::string);
std::vector<std::string> toksplit(std::string, std::string);

#endif
