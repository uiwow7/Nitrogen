#ifndef INTERPRETER_IMPL

#define INTERPRETER_IMPL

typedef struct InterpreterState {
    char* current_function;
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

char* valueAsString(NodeValue val) {
    // printf("type: %d, loc: %p\n", val.type, val.loc);
    switch (val.type) {
        case Type_null:
            return "null";
        case Type_str:
            return val.loc + 1;
        case Type_int:
            return AstrToStr(fromInt(*(int*)(val.loc)));
        case Type_value:
            return valueAsString(*(NodeValue*)val.loc);
    }

    return "TODO";
}

void interpretFunctionCall(AstNode* node, InterpreterState* state, NodeValue values[], int num_values) {
    if (streq(node->token->values, "print")) {
        printf("%s\n", valueAsString(*values));
    }
}

NodeValue traverseAstnode(AstNode* node, InterpreterState* state) {
    int i;

    printf("Traversing node: %p, type %d, num children: %d\n", node->token, node->node_type, node->children.length);

    if (node->token != nullptr) {
        if (node->token->token_type == Tk_Strliteral) {
            return (NodeValue){
                .type = Type_str,
                .loc = node->token->values
            };
        }
    }

    NodeValue* values = calloc(node->children.length, sizeof(NodeValue));

    for (i = 0; i < node->children.length; i++) {
        values[i] = traverseAstnode(getChildAst(*node, i), state);
    }

    if (node->token != nullptr) {
        printf("test: %s\n", TokenTypeRepr(node->token->token_type));
        if (node->token->token_type == Tk_Fncall) {
            interpretFunctionCall(node, state, values, node->children.length);
        }
    }

    int type = Type_value;

    return (NodeValue){
        .type = type,
        .loc = values
    };
}

void interpretAst(AstNode* root) {
    InterpreterState state;
    traverseAstnode(root, &state);
}

#endif