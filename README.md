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

### known issues


This program mostly just invokes stuff in /bin, so it inherits all functionality from them.
More advanced bash commands aren''t guaranteed to work at all - in fact, they may crash rshell outright.

Connector syntax and logic was thoroughly tested and all known bugs mended - if you happen to find any wrong behavior, please submit a comment or email me at '''ichor001@ucr.edu'''.

Don''t ask it to remove itself, strange things will happen.
