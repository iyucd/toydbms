CC    ?= clang
CXX   ?= clang++

EXE = fp.out

CDEBUG = -g 

CXXDEBUG = -g 

CSTD = -std=c99
CXXSTD = -std=c++11

CFLAGS = -O0  $(CDEBUG) $(CSTD)
CXXFLAGS = -O0  $(CXXDEBUG) $(CXXSTD)


CPPOBJ = sql_test sql_driver sql_expr dbms_funcs globals joinfuncs rewrite Table groupby
SOBJ =  parser lexer

FILES = $(addsuffix .cpp, $(CPPOBJ))

OBJS  = $(addsuffix .o, $(CPPOBJ))

CLEANLIST =  $(addsuffix .o, $(OBJ)) $(OBJS) \
				 sql_parser.tab.cc sql_parser.tab.hh \
				 location.hh position.hh \
			    stack.hh sql_parser.output parser.o \
				 lexer.o sql_parser.tab.cc $(EXE)\

.PHONY: all
all: clean wc

wc: $(FILES)
	$(MAKE) $(SOBJ)
	$(MAKE) $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(EXE) $(OBJS) parser.o lexer.o $(LIBS)

# -v generates teh sql_parser.output file
parser: sql_parser.yy
	bison -d sql_parser.yy
	$(CXX) $(CXXFLAGS) -c -o parser.o sql_parser.tab.cc

lexer: sql_parser.l
	flex --outfile=sql_parser.tab.cc  $<
	$(CXX)  $(CXXFLAGS) -c sql_parser.tab.cc -o lexer.o

.PHONY: test
test:
	cd test && ./test0.pl

.PHONY: clean
clean:
	rm -rf $(CLEANLIST)
