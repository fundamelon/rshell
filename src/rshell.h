// RSHELL.H
// Header for rshell

#ifndef RSHELL_H
#define RSHELL_H

void init();
int run();

std::string prompt();

std::vector<std::string> tokenize(std::string);

#endif
