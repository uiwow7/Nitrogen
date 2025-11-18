#ifndef PARSER_IMPL

#define PARSER_IMPL

typedef struct AstChildren {
    int capacity;
    int length;
    struct AstNode **loc;
} AstChildren;

typedef enum AstNodeType {
    Node_Root, // Currently only used for the root node
    Node_Expr, // node with 1+ children of Node_Expr or Node_Value
    Node_Args, // node with 0+ children of Node_Expr or Node_Value
    Node_Declr, // Node with 2 children, a Node_Value type and a Node_Value id
    Node_Action, // Node with 2 children
    Node_Value // node with no children
} NodeType;

typedef struct AstNode {
    Token *token;

    NodeType node_type;

    AstChildren children;
    struct AstNode *parent;
} AstNode;

/// @brief Adds an AstNode as a child to another given AstNode
/// @param parent The AstNode to add the child to
/// @param new_child The AstNode to be added as a child
void addChildAst(AstNode *parent, AstNode *new_child) {
    AstChildren *children = &(parent->children);
    
    children->length++;

    if (children->length > children->capacity) {
        children->capacity *= 2;
        children->loc = realloc(children->loc, children->capacity * sizeof(AstNode*));
    }
    
    // Pointer arithmetic: advance by (index) elements, not bytes.
    AstNode **insert_loc = (children->loc + (children->length - 1));
    new_child->parent = parent;
    *insert_loc = new_child;
}

/// @brief Removes an AstNode's `index`th child
/// @param parent The node to remove the child from
/// @param index The index the child is at
void removeChildAst(AstNode *parent, int index) {
    if (parent->children.length == 0 || index >= parent->children.length || index < 0) {
        return;
    }

    parent->children.length--;
    parent->children.loc[index] = NULL;
}

/// @brief Get's an AstNode's `index`th child
/// @param parent The parent node to get the child from
/// @param index The index of the child
/// @return The child at `index`
AstNode *getChildAst(AstNode parent, int index) {
    if (index < 0 || index >= parent.children.length) {
        return (AstNode*)(-1);
    }

    if (parent.children.loc == NULL) {
        return (AstNode*)(-1);
    }

    return parent.children.loc[index];
}

/// @brief Allocates and initializes an AstNode
/// @return A pointer to the new node
AstNode *new_AstNode() {
    AstNode *node = calloc(1, sizeof(AstNode));
    node->children.loc = malloc(sizeof(AstNode*));
    node->children.capacity = 1;
    node->children.length = 0;
    
    return node;
}

// Stores useful info about the current parser state
typedef struct ParserState {
    bool inExpr;
    bool inArgs;

    AstNode *node_ref;
    int node_ref_index;
    AstNode *scope_ref;
} Parsestate;

/// @brief Parses a program (list of tokens) into an Abstract Syntax Tree
/// @param program The list of tokens to convert into AST
/// @return The root node of the AST
AstNode *parse(Program program) {
    AstNode *root = new_AstNode();
    AstNode *current_node = root; 

    root->node_type = Node_Root;
    root->token = NULL;
    root->parent = NULL;

    Parsestate state = {
        .inArgs = false,
        .inExpr = false,
        .node_ref = NULL,
        .scope_ref = root
    };

    Token *current_token = program.ref;
    Token *prev_token = NULL;
    int loc = 0;

    while (current_token->token_type != Tk_EOF) {
        // printf("current node: %d, tt: %s\n", current_node->node_type, TokenTypeRepr(current_token->token_type));

        if (current_token->token_type == Tk_ID) {
            AstNode *new = new_AstNode();
            bool declaration = false;

            if (prev_token != NULL) { 
                declaration = (prev_token->token_type == Tk_Type);
            }

            new->token = current_token;
            new->node_type = declaration ? Node_Declr : Node_Value;
            state.node_ref = new;
            state.node_ref_index = current_node->children.length + 1;

            if (declaration) {
                AstNode *type_node = new_AstNode();

                type_node->token = prev_token;
                type_node->node_type = Node_Value;

                addChildAst(new, type_node);
            }

            addChildAst(current_node, new);
        }

        else if (current_token->token_type == Tk_Assign) {
            AstNode *new = new_AstNode();
            new->token = current_token;
            new->node_type = Node_Action;
            
            if (state.node_ref == NULL) {
                reportError(current_token, "SyntaxError", "Improper assignment.");
                exit(1); 
            }

            removeChildAst(current_node, state.node_ref_index);
            addChildAst(new, state.node_ref);

            state.node_ref = NULL;
        }

        else if (current_token->token_type == Tk_Openparen) {
            AstNode *new = new_AstNode();
            new->token = current_token;
            new->node_type = Node_Expr;

            if (prev_token != NULL) {
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

        else if (current_token->token_type == Tk_Strliteral) {
            AstNode *new = new_AstNode();
            new->token = current_token;
            new->node_type = Node_Value;
            // printf("str literal: %s\n", TokenTypeRepr(new->token->token_type));

            addChildAst(current_node, new);
        }

        else if (current_token->token_type == Tk_Closeparen) {
            if (state.inExpr) {
                state.inExpr = false;
            }
            if (state.inArgs) {
                state.inArgs = false;
            }
        }

        else if (current_token->token_type == Tk_Semicolon) {
            current_node = state.scope_ref;
        }

        loc++;
        prev_token = current_token;
        current_token = TokenAtIndex(program, loc);
    }

    return root;
}

#endif