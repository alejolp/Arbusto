/*
 * Arbusto: A Python Compiler.
 * Alejandro Santos, @alejolp.
 * Licence: BSD
 */

struct astnode {
    astnode(int node_type_) : node_type(node_type_) {}
    astnode(int node_type_, token_t t) : node_type(node_type_), token(t) {}

    int node_type;
    token_t token;
    std::vector<astnode*> childs;
};
