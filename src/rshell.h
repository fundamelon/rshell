// RSHELL.H
// Header for rshell

#ifndef RSHELL_H
#define RSHELL_H

void init();
int run();

std::string prompt();

int execute(const char* path, char* const argv[]);

std::vector<std::string> tokenize(std::string);
std::vector<std::string> tokenize(std::string, std::string);
std::vector<std::string> toksplit(std::string, std::string);

#endif
