#include "include/util/astr.h"
#include "include/util/list.h"
#include "include/lexer.h"
#include "include/parser.h"
#include "include/interpreter.h"

// #define GDB_MODE
#define GDB_DEBUG_FILENAME "hello.n"
#define DEBUG_LOGS

#define INTERPRET 0
#define COMPILE 1

bool inArgv(char *argv[], int argc, char *str) {
    int i;

    for (i = 0; i < argc; i++) {
        if (streq(argv[i], str)) {
            return true;
        }
    }

    return false;
}

int main(int argc, char *argv[]) {
    #ifndef GDB_MODE
    if (argc <= 1) {
        printf("Please provide a command. To see help, use \"-help\"\n");
        return 1;
    }

    if (streq(argv[1], "-help")) {
        printf("TODO: Write help\n");
        return 0;
    }

    char *filename = argv[1];

    int run_type = INTERPRET;
    char *com_type;

    if (argc > 2) {
        if (streq(argv[2], "-c")) {
            run_type = COMPILE;
            if (argc >= 3) {
                com_type = argv[3];
            } else {
                com_type = "asm";
            }
        }
    }

    if (inArgv(argv, argc, "-d") || inArgv(argv, argc, "-debug") || inArgv(argv, argc, "-log")) {
        #define DEBUG_LOGS
    }

    #endif

    #ifdef GDB_MODE
    char *filename = GDB_DEBUG_FILENAME;
    #endif

    Astr file = fileToAstr(filename);

    if (Astreq(file, (Astr){})) {
        printf("Error with opening file %s\n", filename);
        return 1;
    }

    Program _program = lex(file, filename);

    int i = 0;

    #ifdef DEBUG_LOGS
        // for (i = 0; i < _program.len; i++) {
        //     Token* tk = TokenAtIndex(_program, i);

        //     printf("tk: %s\n", TokenTypeRepr(tk->token_type));

        //     if (tk->token_type == 0) {
        //         printf("id: %s\n", (char*)tk->values);
        //     }
        // }
    #endif

    AstNode* _ast = parse(_program);

    #ifndef GDB_MODE
    if (run_type == COMPILE) { // compiling
        printf("Compilation is not yet supported\n");
    } else if (run_type == INTERPRET) {
        interpretAst(_ast);
    } else {
        printf("Invalid run type %d\n", run_type);
        return 1;
    }
    #endif

    #ifdef GDB_MODE
    interpretAst(_ast);
    #endif

    return 0;
}