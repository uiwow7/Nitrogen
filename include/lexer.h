#ifndef LEXER_IMPL 

#define LEXER_IMPL

#define nextToken() num_tks_processed++; current_token = (tokens + num_tks_processed * sizeof(Token));

typedef struct {
    char* file;
    int line;
    int col;
} TokenLoc;

typedef struct {
    void* values;
    int num_values;
    
    enum {
        Tk_ID,
        Tk_Fncall,
        Tk_Intliteral,
        Tk_Strliteral,
        Tk_Openparen,
        Tk_Closeparen,
        Tk_Semicolon,
        Tk_EOF
    } token_type;

    TokenLoc loc;
} Token;

typedef struct {
    Token* ref;
    int len;
} Program;

char* TokenTypeRepr(int tt) {
    switch (tt) {
        case 0:
            return "id";
        case 1:
            return "fncall";
        case 2:
            return "intliteral";
        case 3:
            return "strliteral";
        case 4:
            return "openparen";
        case 5:
            return "closeparen";
        case 6:
            return "semicolon";
        case 7:
            return "EOF";
    }

    return "unknown";
}

Token* TokenAtIndex(Program program, int index) {
    if (index > program.len) {
        printf("Error: Indexing out of range, index: %d, len: %d\n", index, program.len);
        return (Token*)-1;
    }

    return program.ref + index * sizeof(Token);
}

Program lex(Astr input, char* filename) {
    // Lexing takes 2 steps
    // For the first one, we simply compute the number of tokens we want
    // Then we allocate memory for the program
    // Then we actually compute the tokens and add them to the program
    char c = 'A'; // is null by default so set it to a non-null value to get the loop to start
    int index = 0;
    int tk_count = 0;
    bool in_strliteral = false;
    bool escaped = false;

    while (c != '\0') {
        c = input.str_ref[index];

        if (c == '\\' && in_strliteral) {
            escaped = true;
        }
        else if (c == '"' && !in_strliteral) {
            in_strliteral = true;
        } 
        else if (c == '"' && in_strliteral && !escaped) {
            tk_count++;
            in_strliteral = false;
        }
        else if (c == '(' || c == ')' || c == ';') {
            tk_count++;
        }
        else if (c == ' ' && !in_strliteral) {
            tk_count++;
        }

        if (escaped) {
            escaped = false;
        }

        index++;
    }

    tk_count++; // for eof

    Token* tokens = malloc(tk_count * sizeof(Token));
    int num_tks_processed = 0;
    Token* current_token = tokens;
    char tracker[1024]; // should be enough for everyone
    int tracker_index = 0;
    c = 'A'; // is null by default so set it to a non-null value to get the loop to start
    index = 0;
    TokenLoc current_loc;
    current_loc.file = filename;

    while (c != '\0') {
        c = input.str_ref[index];
        tracker[tracker_index] = c;

        if (c == '\n') {
            current_loc.line++;
            current_loc.col = 0;
        }

        if ((c == ' ' || c == '(' || c == ')' || c == ',' || c == ';') && !in_strliteral && tracker_index > 1) {
            char* tk_val = malloc(tracker_index + 1);
            memcpy(tk_val, tracker, tracker_index);
            tk_val[tracker_index + 1] = '\0';

            tracker_index = 0;

            *current_token = (Token){
                .token_type = Tk_ID,
                .values = tk_val,
                .num_values = 1,
                .loc = current_loc
            };

            nextToken();
        }

        if (c == '\\' && in_strliteral) {
            escaped = true;
        }
        else if (c == '"' && !in_strliteral) {
            in_strliteral = true;
        }  
        else if (c == '"' && in_strliteral && !escaped) {
            char* tk_val = malloc(tracker_index - 1);
            memcpy(tk_val, tracker + 1, tracker_index - 1);
            tk_val[tracker_index - 1] = '\0';

            tracker_index = 0;

            *current_token = (Token){
                .token_type = Tk_Strliteral,
                .values = tk_val,
                .num_values = 1,
                .loc = current_loc
            };

            nextToken();

            in_strliteral = false;
        } 
        else if (c == '(' && !in_strliteral) {
            tracker_index = 0;

            *current_token = (Token){
                .token_type = Tk_Openparen,
                .values = nullptr,
                .num_values = 0,
                .loc = current_loc
            };

            nextToken();
        }
        else if (c == ')' && !in_strliteral) {
            tracker_index = 0;

            *current_token = (Token){
                .token_type = Tk_Closeparen,
                .values = nullptr,
                .num_values = 0,
                .loc = current_loc
            };

            nextToken();
        }
        else if (c == ';' && !in_strliteral) {
            tracker_index = 0;

            *current_token = (Token){
                .token_type = Tk_Semicolon,
                .values = nullptr,
                .num_values = 0,
                .loc = current_loc
            };

            nextToken();
        }

        if (escaped) {
            escaped = false;
        }

        index++;
        tracker_index++;
        current_loc.col++;
    }

    *current_token = (Token){
        .token_type = Tk_EOF,
        .num_values = 0,
        .values = nullptr
    };

    num_tks_processed++;

    return (Program) {
        .len = num_tks_processed,
        .ref = tokens
    };
}

#endif