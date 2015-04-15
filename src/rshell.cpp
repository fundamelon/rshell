// RSHELL.CPP
// Main source code for rshell

#include "unistd.h"
#include "sys/wait.h"
#include "stdio.h"
#include "errno.h"

#include <iostream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>

#include "rshell.h"

#define RSHELL_DEBUG

const char* CONN_AMP = "&&";
const char* CONN_PIPE = "||";
const char* CONN_SEMIC = ";";
 

/* Initialize environment */
void init() {}


/* Main loop - controls command line, parsing, and execution logic */
int run() {
   
    std::vector<std::string> tokens_conn;
    std::vector<std::string> tokens_word;
    std::string usr_input;

    while(true) {
        
        usr_input = prompt();

        tokens_conn = tokenize(usr_input, "(\\|\\||\\&\\&|;)");
        for(unsigned int i = 0; i < tokens_conn.size(); i++) {

#ifdef RSHELL_DEBUG
            std::cout << "<" << tokens_conn.at(i) << ">" << std::endl;
#endif

            tokens_word = toksplit(tokens_conn.at(i), " ");
            for(unsigned int j = 0; j < tokens_word.size(); j++) {
                
                std::string word = tokens_word.at(j);
                // kill empty words
                if(word == "") tokens_word.erase(tokens_word.begin() + j);

                // using boost for convenience - this can be implemented manually
                boost::trim_if(word, boost::is_any_of(" \t"));
            }
           
            std::vector<char*> cmd_argv(tokens_word.size() + 1);
            for(unsigned int k = 0; k < tokens_word.size(); k++) { 
               cmd_argv[k] = &tokens_word[k][0]; 
#ifdef RSHELL_DEBUG 
               std::cout << "\t" << "<" << tokens_word.at(k) << ">" << std::endl;
#endif
            }            

            // exit only if first word is "exit", after formatting
            if(tokens_word.at(0) == "exit") return 0;
            
            execute(cmd_argv[0], cmd_argv.data());
        }
    }
    
    return 0;
}


/* Prints the prompt symbol and takes in raw command line input */
std::string prompt() {

    std::cout << "$ " << std::flush;

    std::string input_raw;
    std::getline(std::cin, input_raw);

    return input_raw;
}


/* Basically a wrapper for execvp, has error checking */
int execute(const char* path, char* const argv[]) {

#ifdef RSHELL_DEBUG
    std::cout << "executing " << path << std::endl;
#endif
    int pid = fork();
    if(pid == -1) {
        perror(NULL);
        return -1;
    } else if(pid == 0) {
        execvp(path, argv);
        perror(path);
        return 1;
    } else {
        if(waitpid(pid, NULL, 0) == -1)
            perror(NULL);
    }

    return 0;
}


/* Overload to tokenize by whitespace */
std::vector<std::string> tokenize(std::string s) {
    return tokenize(s, "\\s");
}


/* Tokenize a string using boost regex */
std::vector<std::string> tokenize(std::string s, std::string r) {

    std::vector<std::string> token_vec;
 
    boost::algorithm::split_regex(token_vec, s, boost::regex(r));
    return token_vec;
}


/* Tokenize a string using boost split */
std::vector<std::string> toksplit(std::string s, std::string toks) {

    std::vector<std::string> token_vec;
    boost::split(token_vec, s, boost::is_any_of(toks), boost::token_compress_on);
    return token_vec;
}
