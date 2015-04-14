
CXX = g++
CXXFLAGS = -W -Wall -pedantic -ansi 

MKDIR_P = mkdir -p
ODIR = bin/

_OBJ = main.o 
OBJ = $(addprefix $(ODIR), $(_OBJ) )


.PHONY: all
all:
	$(MKDIR_P) $(ODIR)
	make rshell
	

$(ODIR)%.o: %.cpp
	$(CXX) -MMD -c -o $@ $< $(CXXFLAGS)

rshell: $(OBJ)
	$(CXX) -o mctest $(OBJ) $(CXXFLAGS) 
	

.PHONY: test
test:
	make
#	clear
	./rshell -A -B

.PHONY: clean
clean: 
	rm -f *.o *.d rshell *~
	rm -rf $(ODIR)
