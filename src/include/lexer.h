#ifndef LEXER_IMPL 

#define LEXER_IMPL

#define nextToken() state.num_tks_processed++;
//printf("[%d]: Token (%s)\n", num_tks_processed, TokenTypeRepr(current_token->token_type));

typedef struct TokenLocation {
    char *file;
    int line;
    int col;
} TokenLoc;

typedef enum TokenType {
    Tk_Null,
    Tk_ID,
    Tk_Type,
    Tk_Fncall,
    Tk_Intliteral,
    Tk_Strliteral,
    Tk_Openparen,
    Tk_Closeparen,
    Tk_Semicolon,
    Tk_Assign,
    Tk_EOF
}  TokenType;

typedef struct Token {
    void *values;
    int num_values;
    
    TokenType token_type;

    TokenLoc loc;
} Token;

typedef struct TokenList {
    Token *ref;
    int len;
    int capacity;
} Program;

/// @brief Pushes a token to a Program dynamic array
/// @param program A reference to the program
/// @param token The token to push
void push_token(Program *program, Token token) {
    if (program->len >= program->capacity) {
        program->capacity *= 2;
        program->ref = realloc(program->ref, program->capacity * sizeof(Token));
    }

    program->ref[program->len] = token;
    program->len++;
}

/// @brief Returns a string to represent a token type enum
/// @param tokenType An int from the token type enum
/// @return A string
char *TokenTypeRepr(enum TokenType tokenType) {
    switch (tokenType) {
        case Tk_ID:
            return "id";
        case Tk_Fncall:
            return "fncall";
        case Tk_Intliteral:
            return "intliteral";
        case Tk_Strliteral:
            return "strliteral";
        case Tk_Openparen:
            return "openparen";
        case Tk_Closeparen:
            return "closeparen";
        case Tk_Semicolon:
            return "semicolon";
        case Tk_EOF:
            return "EOF";
        case Tk_Null:
            return "null";
        case Tk_Assign:
            return "assign";
        case Tk_Type:
            return "type";
    }

    return "could not represent token type";
}

char *formatTokenLoc(TokenLoc loc) {
    Astr combined = _Astr(loc.file);

    combined = concat(combined, _Astr(":"));    
    combined = concat(combined, fromInt(loc.line));    
    combined = concat(combined, _Astr(":"));    
    combined = concat(combined, fromInt(loc.col));

    return AstrToStr(combined);
}

/// @brief Returns a token at a given index into a program struct
/// @param program The Program to index into
/// @param index The index we want to get the token from
/// @return A pointer to the token at the given index
Token* TokenAtIndex(Program program, int index) {
    if (index > program.len) {
        printf("Error: Indexing out of range, index: %d, len: %d\n", index, program.len);
        return (Token*)-1;
    }

    return program.ref + index;
}

#define TRACKER_CAPACITY 1024
typedef struct LexerState {
    char tracker[TRACKER_CAPACITY];
    int tracker_index;
    
    bool escaped;
    bool in_strliteral;

    int num_tks_processed;

    TokenLoc current_loc;
} Lexstate;

/// @brief Determines the token type of an identifier
/// @param id The identifier string
/// @return The TokenType of the identifier
TokenType idTokenType(char *id) {
    if (streq(id, "int") || streq(id, "float") || streq(id, "char") || streq(id, "string")) {
        return Tk_Type;
    }

    return Tk_ID;
}

/// @brief Lexes a given Astr-type input into a series of Token structs
/// @param input Input for the lexer to tokenize
/// @param filename Filename for error reporting using Token locations
/// @return A list of lexed Tokens
Program lex(Astr input, char *filename) {
    #define TOKEN_CAPACITY 1024
    
    char c = 'A'; // is null by default so set it to a non-null value to get the loop to start
    int index = 0;

    Token *tokens = malloc(TOKEN_CAPACITY * sizeof(Token));
    // Token* current_token = tokens;
    
    Program program = {
        .ref = tokens,
        .len = 0,
        .capacity = TOKEN_CAPACITY
    };

    Lexstate state = {
        .escaped = false,
        .in_strliteral = false,
        .tracker_index = 0,
        .current_loc = {
            .file = filename,
            .line = 1,
            .col = 1
        }   
    };

    while (c != '\0') {
        c = input.str_ref[index];
        state.tracker[state.tracker_index] = c;

        // printf("char: %c, tracker: %s [%d]\n", c, state.tracker, state.tracker_index);

        if (isWhiteSpace(c) && !state.in_strliteral && state.tracker_index != 1) state.tracker_index--;

        if (c == '\n') {
            state.current_loc.line++;
            state.current_loc.col = 1;
        }

        if ((c == ' ' || c == '(' || c == ')' || c == ',' || c == ';') && !state.in_strliteral && state.tracker_index > 0) {
            if (isWhiteSpace(c)) state.tracker_index++;

            char *tk_val = strndup(state.tracker, state.tracker_index);

            // printf("ID: %s, %d\n", tk_val, state.tracker_index);
        

            state.tracker_index = -1;
            // printf("IDDDD: %d, %s\n", state.num_tks_processed, TokenTypeRepr(current_token->token_type));
            push_token(&program, (Token){
                .token_type = idTokenType(tk_val),
                .values = tk_val,
                .num_values = 1,
                .loc = state.current_loc
            });

            nextToken();
        }

        if (c == '\\' && state.in_strliteral) {
            state.escaped = true;
        }
        else if (c == '"' && !state.in_strliteral) {
            state.in_strliteral = true;
        }  
        else if (c == '"' && state.in_strliteral && !state.escaped) {
            char *tk_val = strndup(state.tracker + 1, state.tracker_index - 1); // exclude quotes

            state.tracker_index = -1;

            push_token(&program, (Token){
                .token_type = Tk_Strliteral,
                .values = tk_val,
                .num_values = 1,
                .loc = state.current_loc
            });

            nextToken();

            state.in_strliteral = false;
        } 
        else if (c == '(' && !state.in_strliteral) {
            state.tracker_index = -1;

            push_token(&program, (Token){
                .token_type = Tk_Openparen,
                .values = NULL,
                .num_values = 0,
                .loc = state.current_loc
            });

            nextToken();
        }
        else if (c == ')' && !state.in_strliteral) {
            state.tracker_index = -1;

            push_token(&program, (Token){
                .token_type = Tk_Closeparen,
                .values = NULL,
                .num_values = 0,
                .loc = state.current_loc
            });

            nextToken();
        }
        else if (c == ';' && !state.in_strliteral) {
            state.tracker_index = -1;

            push_token(&program, (Token){
                .token_type = Tk_Semicolon,
                .values = NULL,
                .num_values = 0,
                .loc = state.current_loc
            });

            nextToken();
        }
        else if (c == '=' && !state.in_strliteral) {
            state.tracker_index = -1;

            push_token(&program, (Token){
                .token_type = Tk_Assign,
                .values = NULL,
                .num_values = 0,
                .loc = state.current_loc
            });

            nextToken();
        }

        if (state.escaped) state.escaped = false;

        index++;
        state.tracker_index++;
        state.current_loc.col++;
    }

    push_token(&program, (Token){
        .token_type = Tk_EOF,
        .num_values = 0,
        .values = NULL,
        .loc = state.current_loc
    });

    state.num_tks_processed++;

    return program;
}

#endif