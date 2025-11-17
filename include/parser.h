#ifndef PARSER_IMPL

#define PARSER_IMPL

typedef struct AstChildren {
    int capacity;
    int length;
    struct AstNode** loc;
} AstChildren;

typedef struct AstNode {
    Token* token;

    enum {
        Node_Root, // Currently only used for the root node
        Node_Expr, // node with 1+ children of Node_Expr or Node_Value
        Node_Args, // node with 0+ children of Node_Expr or Node_Value
        Node_Value // node with no children
    } node_type;

    AstChildren children;
    struct AstNode* parent;
} AstNode;

void addChildAst(AstNode* parent, AstNode* new_child) {
    AstChildren* children = &(parent->children);
    
    children->length++;

    if (children->length > children->capacity) {
        children->capacity *= 2;
        children->loc = realloc(children->loc, children->capacity * sizeof(AstNode*));
    }
    
    AstNode** insert_loc = (children->loc + (children->length - 1) * sizeof(AstNode*));
    new_child->parent = parent;
    *insert_loc = new_child;
}

AstNode* getChildAst(AstNode parent, int index) {
    if (index > parent.children.length) {
        return (AstNode*)(-1);
    }

    if (parent.children.loc == nullptr) {
        return (AstNode*)(-1);
    }

    return *(parent.children.loc + index * sizeof(AstNode*));
}

AstNode* new_AstNode() {
    AstNode* node = calloc(1, sizeof(AstNode));
    node->children.loc = malloc(sizeof(AstNode*));
    node->children.capacity = 1;
    node->children.length = 0;
    
    return node;
}

// Stores useful info about the current parser state
typedef struct ParserState {
    bool inExpr;
    bool inArgs;
    AstNode* node_ref;
    AstNode* scope_ref;
} Parsestate;

AstNode* parse(Program program) {
    Parsestate state;

    AstNode* root = new_AstNode();
    AstNode* current_node = root;
    state.scope_ref = root;    

    root->node_type = Node_Root;
    root->token = nullptr;
    root->parent = nullptr;

    Token* current_token = program.ref;
    Token* prev_token = nullptr;
    int loc = 0;

    while (current_token->token_type != Tk_EOF) {
        // printf("current node: %d\n", current_node->node_type);

        if (current_token->token_type == Tk_ID) {
            AstNode* new = new_AstNode();
            new->token = current_token;
            new->node_type = Node_Value;
            state.node_ref = new;

            addChildAst(current_node, new);
        }

        if (current_token->token_type == Tk_Openparen) {
            AstNode* new = new_AstNode();
            new->token = current_token;
            new->node_type = Node_Expr;

            if (prev_token != nullptr) {
                if (prev_token->token_type == Tk_ID) {
                    new->node_type = Node_Args;
                    state.inArgs = true;
                }
            }

            if (state.inArgs) {
                addChildAst(state.node_ref, new);
                state.node_ref->token->token_type = Tk_Fncall;
                state.node_ref->node_type = Node_Expr;
            } else {
                addChildAst(current_node, new);
            }
            
            current_node = new;
        }

        if (current_token->token_type == Tk_Strliteral) {
            AstNode* new = new_AstNode();
            new->token = current_token;
            new->node_type = Node_Value;

            addChildAst(current_node, new);
        }

        if (current_token->token_type == Tk_Closeparen) {
            if (state.inExpr) {
                state.inExpr = false;
            }
            if (state.inArgs) {
                state.inArgs = false;
            }
        }

        if (current_token->token_type == Tk_Semicolon) {
            current_node = state.scope_ref;
        }

        loc++;
        prev_token = current_token;
        current_token = TokenAtIndex(program, loc); 
    }

    return root;
}

#endif