// RSHELL.CPP
// Main source code for rshell

#include <iostream>
#include <string>
#include <vector>

#include <boost/tokenizer.hpp>

#include "rshell.h"


void init() {}

int run() {
    
    std::vector<std::string> input = tokenize(prompt());
    for(unsigned int i = 0; i < input.size(); i++) 
        std::cout << input.at(i) << std::endl;
    return 0;
}


std::string prompt() {

    std::cout << "$ ";

    std::string input_raw;
    std::getline(std::cin, input_raw);

    return input_raw;
}


std::vector<std::string> tokenize(std::string s) {
    
    std::vector<std::string> token_vec;
    
    // only tokenize by whitespace for now - possible to expand later
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep(" ");
    tokenizer s_tok(s, sep);
    for(tokenizer::iterator iter = s_tok.begin(); iter != s_tok.end(); ++iter)
        token_vec.push_back(*iter);

    return token_vec;
}
