# rshell

A simple command shell *sorta* modeled after gnu bash.  For educational purposes.
This project will be expanded to include more functionality in the coming weeks.

## usage

To install, simply ```make``` in the project root directory. 

(This project requires boost regex and string libraries to build.)

To run, invoke ```bin/rshell``` in bash.

### notes

All basic programs in '''/bin''' are supported. (This means cd does not work, yet.)

The bash connectors '''&&''', '''||''', ''';''' are suported and function exactly the same.

Connectors at the beginning of a line are treated as syntax errors.

When ```ls``` is invoked, the flag ```--color``` is appended to enable colors.

If you want to modify the source code to better understand the logic, defining the flags RSHELL_DEBUG and RSHELL_PREPEND should help.
