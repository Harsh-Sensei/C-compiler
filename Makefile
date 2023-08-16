CXX  = g++-8
EXE = iplC
CXXDEBUG = -g -Wall
CXXSTD = -std=c++11


.PHONY: all parser lexer clean 

all: parser lexer 	
	$(CXX) $(CXXDEBUG) $(CXXSTD) -o $(EXE) main.cpp symbtab.cpp gencode.cpp ast.cpp parser.o scanner.o 

parser: parser.yy scanner.hh
	bison -d -v $<
	$(CXX) $(CXXDEBUG) $(CXXSTD) -c parser.tab.cc -o parser.o 

lexer: scanner.l scanner.hh parser.tab.hh parser.tab.cc	
	flex++ --outfile=scanner.yy.cc  $<
	$(CXX)  $(CXXDEBUG) $(CXXSTD) -c scanner.yy.cc -o scanner.o

clean:
	rm *.o parser.tab.cc parser.tab.hh iplC location.hh parser.output position.hh scanner.yy.cc stack.hh ipl_htg
