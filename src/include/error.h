#ifndef _ERROR_H

#define _ERROR_H

void reportError(Token *token, char *errorType, char *errorMsg) {
    fprintf(stderr, "\x1B[31mERROR at %s:\n%s: %s\n\x1B[0m", formatTokenLoc(token->loc), errorType, errorMsg);
    exit(1);
}

void reportWarning(Token *token, char *errorType, char *errorMsg) {
    fprintf(stderr, "\x1B[33mERROR at %s:\n%s: %s\n\x1B[0m", formatTokenLoc(token->loc), errorType, errorMsg);
}

#endif