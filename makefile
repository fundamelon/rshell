
CXX = g++
CXXFLAGS = --std=c++11 -Wall -Werror -pedantic 
BOOSTFLAGS = -lboost_regex

MKDIR_P = mkdir -p
ODIR = bin/
SDIR = src/

_CPP = $(wildcard src/*.cpp)

_RSHELL_OBJ = main.o rshell.o
RSHELL_OBJ = $(addprefix $(ODIR), $(_RSHELL_OBJ) )

_LS_OBJ = ls.o
LS_OBJ = $(addprefix $(ODIR), $(_LS_OBJ) )

.PHONY: all
all: rshell ls

rshell: $(RSHELL_OBJ)
	$(CXX) -o $(ODIR)rshell $(RSHELL_OBJ) $(BOOSTFLAGS) $(CXXFLAGS) 

ls: $(LS_OBJ)
	$(CXX) -o $(ODIR)ls $(LS_OBJ) $(BOOSTFLAGS) $(CXXFLAGS) 

$(ODIR)%.o: $(SDIR)%.cpp
	$(MKDIR_P) $(ODIR)
	$(CXX) -MMD -c -o $@ $< $(CXXFLAGS)


.PHONY: clean
clean: 
	rm -f *.o *.d rshell *~
	rm -rf $(ODIR)
