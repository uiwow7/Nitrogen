#ifndef INTERPRETER_IMPL

#define INTERPRETER_IMPL

typedef struct InterpreterState {
    char *current_function;
    bool in_fn_call;
} InterpreterState;

typedef struct NodeValue {
    void* loc;
    enum {
        Type_null,
        Type_float,
        Type_int,
        Type_str,
        Type_char,
        Type_ptr_int,
        Type_ptr_float,
        Type_value
    } type;
} NodeValue;

/// @brief Converts a NodeValue into a string
/// @param val The NodeValue to convert
/// @return The NodeValue as a string
char *valueAsString(NodeValue val) {
    // printf("type: %d, loc: %p\n", val.type, val.loc);
    switch (val.type) {
        case Type_null:
            return "null";
        case Type_str:
            return val.loc;
        case Type_int:
            return AstrToStr(fromInt(*(int*)(val.loc)));
        case Type_value:
            return valueAsString(*(NodeValue*)val.loc);
        case Type_char:
            char *new = malloc(sizeof(char) * 2);
            new[0] = *(char*)(val.loc);
            new[1] = '\0';
            return new;
        case Type_float:
            return "TODO: Floats are not supported in valueAsString yet";
        case Type_ptr_int:
            return AstrToStr(concat(_Astr("int*: 0x"), fromInt((long)*(int**)(val.loc))));
        case Type_ptr_float:
            return AstrToStr(concat(_Astr("float*: 0x"), fromInt((long)*(int**)(val.loc))));
    }

    return "TODO";
}

/// @brief Interprets a function call
/// @param node The AstNode of the function call
/// @param state The Interpreter state
/// @param values A list of the values associated with the arguments
/// @param num_values The number of arguments
void interpretFunctionCall(AstNode* node, InterpreterState* state, NodeValue values[], int num_values) {
    if (node->token->values == NULL) {
        reportError(node->token, "ReferenceError", "Function with an undefined name");
    }

    if (streq((char*)node->token->values, "print")) {
        printf("%s\n", valueAsString(*values));
        return;
    }

    char *error_str = "Cannot find function `";
    strcat(error_str, (char*)node->token->values);
    strcat(error_str, "`");

    reportError(node->token, "ReferenceError", error_str);
}

/// @brief Traverses an AstNode and performs all necessary interpreting. The core function of the interpreter
/// @param node The node to traverse
/// @param state The Interpreter state
/// @return A NodeValue associated with the node (by default is the value of its children)
NodeValue traverseAstnode(AstNode* node, InterpreterState* state) {
    int i;

    if (node->token != NULL) {
        // printf("TT: %s\n", TokenTypeRepr(node->token->token_type));
        if (node->token->token_type == Tk_Strliteral) {
            // char *copied = strdup(node->token->values);
            return (NodeValue){
                .type = Type_str,
                .loc = node->token->values
            };
        }

        if (node->token->token_type == Tk_Intliteral) {
            // printf("intlit: %d\n", *(int*)(node->token->values));
            return (NodeValue){
                .type = Type_int,
                .loc = node->token->values
            };
        }
    }

    NodeValue* values;

    if (node->children.length == 0) {
        values = malloc(sizeof(NodeValue));
        *values = (NodeValue){
            .type = Type_null,
            .loc = NULL
        };
    } else {
        values = calloc(node->children.length, sizeof(NodeValue));
    }

    for (i = 0; i < node->children.length; i++) {
        values[i] = traverseAstnode(getChildAst(*node, i), state);
    }

    if (node->token != NULL) {
        if (node->token->token_type == Tk_Fncall) {
            // printf("calling: %p, %s\n", values, (char*)(node->token->values));
            interpretFunctionCall(node, state, values, node->children.length);
        }
    }

    int type = Type_value;

    return (NodeValue){
        .type = type,
        .loc = values
    };
}

/// @brief Initializes the interpreter and traverses the root node
/// @param root The root node
void interpretAst(AstNode* root) {
    InterpreterState state = {
        .current_function = NULL,
        .in_fn_call = false
    };

    traverseAstnode(root, &state);
}

#endif