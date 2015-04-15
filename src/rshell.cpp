// RSHELL.CPP
// Main source code for rshell

#include <iostream>
#include <string>
#include <vector>

#include <boost/algorithm/string.hpp>
#include <boost/algorithm/string/regex.hpp>
#include <boost/regex.hpp>

#include "rshell.h"


const char* CONN_AMP = "&&";
const char* CONN_PIPE = "||";
const char* CONN_SEMIC = ";";
 

void init() {}

int run() {
   
    std::vector<std::string> tokens_conn;
    std::vector<std::string> tokens_word;
    std::string usr_input;

    while(true) {
        
        usr_input = prompt();

        tokens_conn = tokenize(usr_input, "(\\|\\||\\&\\&|;)");
        for(unsigned int i = 0; i < tokens_conn.size(); i++) {
       
            std::cout << "<" << tokens_conn.at(i) << ">" << std::endl;

            tokens_word = toksplit(tokens_conn.at(i), " ");
            for(unsigned int j = 0; j < tokens_word.size(); j++) {
                
                std::string word = tokens_word.at(j);
                // kill empty words
                if(word == "") tokens_word.erase(tokens_word.begin() + j);

                // using boost for convenience - this can be implemented manually
                boost::trim_if(word, boost::is_any_of(" \t"));


                if(word == "exit") return 0;
            }
           
            std::vector<char*> cmd_argv(tokens_word.size() + 1);
            for(unsigned int k = 0; k < tokens_word.size(); k++) { 
               cmd_argv[k] = &tokens_word[k][0]; 
                std::cout << "\t" << "<" << tokens_word.at(k) << ">" << std::endl;
            }            

            execute(cmd_argv[0], cmd_argv.data());
        }
    }
    
    return 0;
}


std::string prompt() {

    std::cout << "$ " << std::flush;

    std::string input_raw;
    std::getline(std::cin, input_raw);

    return input_raw;
}


int execute(const char* path, char* const argv[]) {

    std::cout << "executing " << path << std::endl;
}


std::vector<std::string> tokenize(std::string s) {
    return tokenize(s, "\\s");
}


std::vector<std::string> tokenize(std::string s, std::string r) {

    std::vector<std::string> token_vec;
 
    boost::algorithm::split_regex(token_vec, s, boost::regex(r));
    return token_vec;
}


std::vector<std::string> toksplit(std::string s, std::string toks) {

    std::vector<std::string> token_vec;
    boost::split(token_vec, s, boost::is_any_of(toks), boost::token_compress_on);
    return token_vec;
}
