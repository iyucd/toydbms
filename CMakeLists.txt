find_package(BISON 3.0.2)
find_package(FLEX 2.5.35)
BISON_TARGET(MyParser sql_parser.yy ${CMAKE_CURRENT_BINARY_DIR}/sql_parser.tab.cc COMPILE_FLAGS "-d")
set (BISON_EXTRA_HEADERS
        ${PROJECT_BINARY_DIR}/position.hh
        ${PROJECT_BINARY_DIR}/location.hh
        ${PROJECT_BINARY_DIR}/stack.hh)
FLEX_TARGET(MyScanner sql_parser.l ${CMAKE_CURRENT_BINARY_DIR}/sql_lexer.tab.cc
        COMPILE_FLAGS "--outfile=sql_parser.tab.cc")
ADD_FLEX_BISON_DEPENDENCY(MyScanner MyParser)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

message("bison out: ${BISON_MyParser_OUTPUTS}")
message("flex out: ${FLEX_MyScanner_OUTPUTS}")
set(BISON_SRC ${BISON_MyParser_OUTPUTS} PARENT_SCOPE)
set(FLEX_SRC ${FLEX_MyScanner_OUTPUTS} PARENT_SCOPE)
## Create target for the parser
#ADD_CUSTOM_TARGET(FooParser echo "Creating parser.c")
#
## Create custom command for flex/lex (note the outputs)
#ADD_CUSTOM_COMMAND(
#        SOURCE ${Foo_SOURCE_DIR}/src/lexer.l
#        COMMAND ${FLEX_EXECUTABLE}
#        ARGS -o${Foo_BINARY_DIR}/src/lexer.c
#        ${Foo_SOURCE_DIR}/src/lexer.l
#        TARGET FooParser
#        OUTPUTS ${Foo_BINARY_DIR}/src/lexer.c)
#
## Create custom command for bison/yacc (note the DEPENDS)
#ADD_CUSTOM_COMMAND(
#        SOURCE ${Foo_SOURCE_DIR}/src/parser.y
#        COMMAND ${BISON_EXECUTABLE}
#        ARGS -y ${Foo_SOURCE_DIR}/src/parser.y
#        -o ${Foo_BINARY_DIR}/src/parser.c
#        TARGET FooParser
#        DEPENDS ${Foo_BINARY_DIR}/src/lexer.c
#        OUTPUTS ${Foo_BINARY_DIR}/src/parser.c)
#
## Add parser.c to the list of sources
#SET(Foo_SRCS ${Foo_SRCS} ${Foo_BINARY_DIR}/src/parser.c)
#
## Since parser.c does not exists yet when cmake is run, mark
## it as generated
#SET_SOURCE_FILES_PROPERTIES(${Foo_BINARY_DIR}/src/parser.c GENERATED)
#
## Include binary directory to include lexer.c in parser.c
#INCLUDE_DIRECTORIES(${Foo_BINARY_DIR}/src)