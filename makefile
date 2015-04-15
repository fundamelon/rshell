
CXX = g++
CXXFLAGS = -Wall -Werror -ansi -pedantic 

MKDIR_P = mkdir -p
ODIR = bin/
SDIR = src/

_CPP = $(wildcard src/*.cpp)

_OBJ = main.o rshell.o
OBJ = $(addprefix $(ODIR), $(_OBJ) )


.PHONY: all
all:
	$(MKDIR_P) $(ODIR)
	make rshell


rshell: $(OBJ)
	$(CXX) -o $(ODIR)rshell $(OBJ) $(CXXFLAGS) 
	
$(ODIR)%.o: $(SDIR)%.cpp
	$(CXX) -MMD -c -o $@ $< $(CXXFLAGS)


.PHONY: clean
clean: 
	rm -f *.o *.d rshell *~
	rm -rf $(ODIR)
