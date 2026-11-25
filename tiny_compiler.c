#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* ================= CONFIGURATION ================= */

#define MAX_SOURCE 5000
#define MAX_TOKENS 500
#define MAX_SYMBOLS 100
#define MAX_TEXT 50

/* ================= TOKEN DEFINITIONS ================= */

typedef enum {
    INT, IF, ELSE, WHILE, PRINT,
    ID, NUM,
    PLUS, MINUS, MUL, DIV,
    ASSIGN,
    LT, GT, LE, GE, EQ, NE,
    LPAREN, RPAREN,
    LBRACE, RBRACE,
    SEMICOLON,
    END
} TokenType;

typedef struct {
    TokenType type;
    char text[MAX_TEXT];
} Token;

Token tokens[MAX_TOKENS];
int tokenCount = 0;
int pos = 0;

/* ================= AST NODE ================= */

typedef struct ASTNode {
    char type[20];
    char value[MAX_TEXT];

    struct ASTNode *left;
    struct ASTNode *right;
    struct ASTNode *third;
    struct ASTNode *next;

} Node;

/* third = ELSE branch
   next  = Next statement
*/

char symbolTable[MAX_SYMBOLS][MAX_TEXT];
int symbolCount = 0;

/* ================= UTILITY FUNCTIONS ================= */

void copyText(char *destination, const char *source)
{
    if (strlen(source) >= MAX_TEXT) {
        printf("ERROR: Identifier or token is too long.\n");
        exit(1);
    }

    strcpy(destination, source);
}

char *tokenName(TokenType type)
{
    switch (type) {
        case INT: return "INT";
        case IF: return "IF";
        case ELSE: return "ELSE";
        case WHILE: return "WHILE";
        case PRINT: return "PRINT";

        case ID: return "IDENTIFIER";
        case NUM: return "NUMBER";

        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case MUL: return "MULTIPLY";
        case DIV: return "DIVIDE";

        case ASSIGN: return "ASSIGN";

        case LT: return "LESS_THAN";
        case GT: return "GREATER_THAN";
        case LE: return "LESS_EQUAL";
        case GE: return "GREATER_EQUAL";
        case EQ: return "EQUAL";
        case NE: return "NOT_EQUAL";

        case LPAREN: return "LEFT_PAREN";
        case RPAREN: return "RIGHT_PAREN";

        case LBRACE: return "LEFT_BRACE";
        case RBRACE: return "RIGHT_BRACE";

        case SEMICOLON: return "SEMICOLON";
        case END: return "END";

        default: return "UNKNOWN";
    }
}

/* ================= 1. LEXICAL ANALYSIS ================= */

void addToken(TokenType type, const char *text)
{
    if (tokenCount >= MAX_TOKENS) {
        printf("LEXICAL ERROR: Too many tokens.\n");
        exit(1);
    }

    tokens[tokenCount].type = type;
    copyText(tokens[tokenCount].text, text);

    tokenCount++;
}

void lexer(char *source)
{
    int i = 0;

    while (source[i] != '\0') {

        if (isspace((unsigned char)source[i])) {
            i++;
            continue;
        }

        /* Keywords and identifiers */

        if (isalpha((unsigned char)source[i]) ||
            source[i] == '_') {

            char word[MAX_TEXT];
            int j = 0;

            while (isalnum((unsigned char)source[i]) ||
                   source[i] == '_') {

                if (j >= MAX_TEXT - 1) {
                    printf("LEXICAL ERROR: Identifier too long.\n");
                    exit(1);
                }

                word[j++] = source[i++];
            }

            word[j] = '\0';

            if (strcmp(word, "int") == 0)
                addToken(INT, word);

            else if (strcmp(word, "if") == 0)
                addToken(IF, word);

            else if (strcmp(word, "else") == 0)
                addToken(ELSE, word);

            else if (strcmp(word, "while") == 0)
                addToken(WHILE, word);

            else if (strcmp(word, "print") == 0)
                addToken(PRINT, word);

            else
                addToken(ID, word);

            continue;
        }

        /* Numbers */

        if (isdigit((unsigned char)source[i])) {

            char number[MAX_TEXT];
            int j = 0;

            while (isdigit((unsigned char)source[i])) {

                if (j >= MAX_TEXT - 1) {
                    printf("LEXICAL ERROR: Number too long.\n");
                    exit(1);
                }

                number[j++] = source[i++];
            }

            number[j] = '\0';

            addToken(NUM, number);
            continue;
        }

        /* Operators and special symbols */

        switch (source[i]) {

            case '+':
                addToken(PLUS, "+");
                i++;
                break;

            case '-':
                addToken(MINUS, "-");
                i++;
                break;

            case '*':
                addToken(MUL, "*");
                i++;
                break;

            case '/':
                addToken(DIV, "/");
                i++;
                break;

            case '=':
                if (source[i + 1] == '=') {
                    addToken(EQ, "==");
                    i += 2;
                } else {
                    addToken(ASSIGN, "=");
                    i++;
                }
                break;

            case '<':
                if (source[i + 1] == '=') {
                    addToken(LE, "<=");
                    i += 2;
                } else {
                    addToken(LT, "<");
                    i++;
                }
                break;

            case '>':
                if (source[i + 1] == '=') {
                    addToken(GE, ">=");
                    i += 2;
                } else {
                    addToken(GT, ">");
                    i++;
                }
                break;

            case '!':
                if (source[i + 1] == '=') {
                    addToken(NE, "!=");
                    i += 2;
                } else {
                    printf("LEXICAL ERROR: Invalid !\n");
                    exit(1);
                }
                break;

            case '(':
                addToken(LPAREN, "(");
                i++;
                break;

            case ')':
                addToken(RPAREN, ")");
                i++;
                break;

            case '{':
                addToken(LBRACE, "{");
                i++;
                break;

            case '}':
                addToken(RBRACE, "}");
                i++;
                break;

            case ';':
                addToken(SEMICOLON, ";");
                i++;
                break;

            default:
                printf("LEXICAL ERROR: Invalid character '%c'\n",
                       source[i]);
                exit(1);
        }
    }

    addToken(END, "END");
}

/* ================= 2. SEMANTIC ANALYSIS ================= */

/* Find a variable in the symbol table */

int findSymbol(char *name)
{
    for (int i = 0; i < symbolCount; i++) {

        if (strcmp(symbolTable[i], name) == 0)
            return 1;
    }

    return 0;
}

/* Declare a variable */

void declareSymbol(char *name)
{
    if (findSymbol(name)) {
        printf("SEMANTIC ERROR: Variable %s already declared.\n",
               name);
        exit(1);
    }

    if (symbolCount >= MAX_SYMBOLS) {
        printf("SEMANTIC ERROR: Symbol table is full.\n");
        exit(1);
    }

    copyText(symbolTable[symbolCount], name);
    symbolCount++;
}

/* Check whether a variable is declared */

void checkSymbol(char *name)
{
    if (!findSymbol(name)) {
        printf("SEMANTIC ERROR: Variable %s not declared.\n",
               name);
        exit(1);
    }
}

/* ================= AST CREATION ================= */

Node *createNode(char *type, char *value)
{
    Node *n = malloc(sizeof(Node));

    if (n == NULL) {
        printf("ERROR: Memory allocation failed.\n");
        exit(1);
    }

    copyText(n->type, type);

    if (value != NULL)
        copyText(n->value, value);
    else
        n->value[0] = '\0';

    n->left = NULL;
    n->right = NULL;
    n->third = NULL;
    n->next = NULL;

    return n;
}

/* ================= SYNTAX ANALYSIS ================= */

void syntaxError()
{
    printf("\nSYNTAX ERROR near: %s\n", tokens[pos].text);
    exit(1);
}

void match(TokenType type)
{
    if (tokens[pos].type == type)
        pos++;
    else
        syntaxError();
}

/* Forward declarations */

Node *expression();
Node *statement();
Node *condition();

/* ================= PRIMARY EXPRESSION ================= */

Node *primary()
{
    Node *n;

    if (tokens[pos].type == NUM) {

        n = createNode("NUMBER", tokens[pos].text);

        match(NUM);

        return n;
    }

    if (tokens[pos].type == ID) {

        checkSymbol(tokens[pos].text);

        n = createNode("IDENTIFIER", tokens[pos].text);

        match(ID);

        return n;
    }

    if (tokens[pos].type == LPAREN) {

        match(LPAREN);

        n = expression();

        match(RPAREN);

        return n;
    }

    syntaxError();

    return NULL;
}

/* ================= TERM: *, / ================= */

Node *term()
{
    Node *left;
    Node *right;
    Node *op;

    left = primary();

    while (tokens[pos].type == MUL ||
           tokens[pos].type == DIV) {

        char operation[MAX_TEXT];

        copyText(operation, tokens[pos].text);

        match(tokens[pos].type);

        right = primary();

        op = createNode("OPERATOR", operation);

        op->left = left;
        op->right = right;

        left = op;
    }

    return left;
}

/* ================= EXPRESSION: +, - ================= */

Node *expression()
{
    Node *left;
    Node *right;
    Node *op;

    left = term();

    while (tokens[pos].type == PLUS ||
           tokens[pos].type == MINUS) {

        char operation[MAX_TEXT];

        copyText(operation, tokens[pos].text);

        match(tokens[pos].type);

        right = term();

        op = createNode("OPERATOR", operation);

        op->left = left;
        op->right = right;

        left = op;
    }

    return left;
}

/* ================= RELATIONAL CONDITIONS ================= */

Node *condition()
{
    Node *left;
    Node *right;
    Node *op;

    left = expression();

    if (tokens[pos].type == LT ||
        tokens[pos].type == GT ||
        tokens[pos].type == LE ||
        tokens[pos].type == GE ||
        tokens[pos].type == EQ ||
        tokens[pos].type == NE) {

        char operation[MAX_TEXT];

        copyText(operation, tokens[pos].text);

        match(tokens[pos].type);

        right = expression();

        op = createNode("OPERATOR", operation);

        op->left = left;
        op->right = right;

        return op;
    }

    return left;
}

/* ================= DECLARATION ================= */

Node *declaration()
{
    Node *n;

    match(INT);

    if (tokens[pos].type != ID)
        syntaxError();

    declareSymbol(tokens[pos].text);

    n = createNode("DECLARATION", tokens[pos].text);

    match(ID);

    match(SEMICOLON);

    return n;
}

/* ================= ASSIGNMENT ================= */

Node *assignment()
{
    Node *n;
    char name[MAX_TEXT];

    copyText(name, tokens[pos].text);

    checkSymbol(name);

    match(ID);

    match(ASSIGN);

    n = createNode("ASSIGNMENT", name);

    n->left = expression();

    match(SEMICOLON);

    return n;
}

/* ================= PRINT STATEMENT ================= */

Node *printStatement()
{
    Node *n;

    match(PRINT);

    match(LPAREN);

    n = createNode("PRINT", "print");

    n->left = expression();

    match(RPAREN);

    match(SEMICOLON);

    return n;
}

/* ================= BLOCK ================= */

Node *block()
{
    Node *blockNode;
    Node *first = NULL;
    Node *last = NULL;

    match(LBRACE);

    blockNode = createNode("BLOCK", "block");

    while (tokens[pos].type != RBRACE &&
           tokens[pos].type != END) {

        Node *stmt = statement();

        if (first == NULL) {
            first = stmt;
            last = stmt;
        } else {
            last->next = stmt;
            last = stmt;
        }
    }

    match(RBRACE);

    blockNode->left = first;

    return blockNode;
}

/* ================= IF-ELSE STATEMENT ================= */

Node *ifStatement()
{
    Node *n;

    match(IF);

    match(LPAREN);

    n = createNode("IF", "if");

    n->left = condition();

    match(RPAREN);

    /* IF body */

    n->right = statement();

    /* ELSE body */

    if (tokens[pos].type == ELSE) {

        match(ELSE);

        n->third = statement();
    }

    return n;
}

/* ================= WHILE STATEMENT ================= */

Node *whileStatement()
{
    Node *n;

    match(WHILE);

    match(LPAREN);

    n = createNode("WHILE", "while");

    n->left = condition();

    match(RPAREN);

    n->right = statement();

    return n;
}

/* ================= STATEMENT ================= */

Node *statement()
{
    if (tokens[pos].type == INT)
        return declaration();

    if (tokens[pos].type == ID)
        return assignment();

    if (tokens[pos].type == PRINT)
        return printStatement();

    if (tokens[pos].type == IF)
        return ifStatement();

    if (tokens[pos].type == WHILE)
        return whileStatement();

    if (tokens[pos].type == LBRACE)
        return block();

    syntaxError();

    return NULL;
}

/* ================= PROGRAM ================= */

Node *program()
{
    Node *root;
    Node *first = NULL;
    Node *last = NULL;

    root = createNode("PROGRAM", "Tiny-C");

    while (tokens[pos].type != END) {

        Node *stmt = statement();

        if (first == NULL) {
            first = stmt;
            last = stmt;
        } else {
            last->next = stmt;
            last = stmt;
        }
    }

    root->left = first;

    return root;
}

/* ================= 3. ABSTRACT SYNTAX TREE ================= */

void printAST(Node *node, int level)
{
    while (node != NULL) {

        for (int i = 0; i < level; i++)
            printf("   ");

        printf("%s", node->type);

        if (strlen(node->value) > 0)
            printf(" (%s)", node->value);

        printf("\n");

        if (node->left != NULL) {
            printAST(node->left, level + 1);
        }

        if (node->right != NULL) {

            for (int i = 0; i <= level; i++)
                printf("   ");

            printf("THEN / RIGHT\n");

            printAST(node->right, level + 2);
        }

        if (node->third != NULL) {

            for (int i = 0; i <= level; i++)
                printf("   ");

            printf("ELSE\n");

            printAST(node->third, level + 2);
        }

        node = node->next;
    }
}

/* ================= 4. THREE ADDRESS CODE ================= */

int temp = 0;
int label = 0;

char *newTemp()
{
    char *t = malloc(30);

    if (t == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    sprintf(t, "t%d", ++temp);

    return t;
}

char *newLabel()
{
    char *l = malloc(30);

    if (l == NULL) {
        printf("Memory allocation failed.\n");
        exit(1);
    }

    sprintf(l, "L%d", ++label);

    return l;
}

/* Generate arithmetic expressions */

char *generateExpression(Node *n)
{
    char *left;
    char *right;
    char *t;

    if (strcmp(n->type, "NUMBER") == 0 ||
        strcmp(n->type, "IDENTIFIER") == 0) {

        char *result = malloc(MAX_TEXT);

        if (result == NULL)
            exit(1);

        copyText(result, n->value);

        return result;
    }

    if (strcmp(n->type, "OPERATOR") == 0) {

        left = generateExpression(n->left);

        right = generateExpression(n->right);

        t = newTemp();

        printf("%s = %s %s %s\n",
               t, left, n->value, right);

        free(left);
        free(right);

        return t;
    }

    return NULL;
}

/* Generate conditional jumps */

void generateCondition(Node *n, char *trueLabel, char *falseLabel)
{
    char *left;
    char *right;

    if (strcmp(n->type, "OPERATOR") == 0 &&
        (strcmp(n->value, "<") == 0 ||
         strcmp(n->value, ">") == 0 ||
         strcmp(n->value, "<=") == 0 ||
         strcmp(n->value, ">=") == 0 ||
         strcmp(n->value, "==") == 0 ||
         strcmp(n->value, "!=") == 0)) {

        left = generateExpression(n->left);

        right = generateExpression(n->right);

        printf("IF %s %s %s GOTO %s\n",
               left, n->value, right, trueLabel);

        printf("GOTO %s\n", falseLabel);

        free(left);
        free(right);
    }

    else {

        left = generateExpression(n);

        printf("IF %s != 0 GOTO %s\n",
               left, trueLabel);

        printf("GOTO %s\n", falseLabel);

        free(left);
    }
}

/* Generate TAC for all statements */

void generateTAC(Node *node)
{
    while (node != NULL) {

        /* Declaration */

        if (strcmp(node->type, "DECLARATION") == 0) {

            printf("// declare %s\n", node->value);
        }

        /* Assignment */

        else if (strcmp(node->type, "ASSIGNMENT") == 0) {

            char *result;

            result = generateExpression(node->left);

            printf("%s = %s\n", node->value, result);

            free(result);
        }

        /* Print */

        else if (strcmp(node->type, "PRINT") == 0) {

            char *result;

            result = generateExpression(node->left);

            printf("PRINT %s\n", result);

            free(result);
        }

        /* Block */

        else if (strcmp(node->type, "BLOCK") == 0) {

            generateTAC(node->left);
        }

        /* IF-ELSE */

        else if (strcmp(node->type, "IF") == 0) {

            char *trueLabel = newLabel();
            char *falseLabel = newLabel();
            char *endLabel = newLabel();

            generateCondition(node->left,
                              trueLabel, falseLabel);

            printf("%s:\n", trueLabel);

            generateTAC(node->right);

            if (node->third != NULL) {

                printf("GOTO %s\n", endLabel);

                printf("%s:\n", falseLabel);

                generateTAC(node->third);

                printf("%s:\n", endLabel);
            }

            else {

                printf("%s:\n", falseLabel);
            }

            free(trueLabel);
            free(falseLabel);
            free(endLabel);
        }

        /* WHILE LOOP */

        else if (strcmp(node->type, "WHILE") == 0) {

            char *startLabel = newLabel();
            char *bodyLabel = newLabel();
            char *endLabel = newLabel();

            printf("%s:\n", startLabel);

            generateCondition(node->left,
                              bodyLabel, endLabel);

            printf("%s:\n", bodyLabel);

            generateTAC(node->right);

            printf("GOTO %s\n", startLabel);

            printf("%s:\n", endLabel);

            free(startLabel);
            free(bodyLabel);
            free(endLabel);
        }

        node = node->next;
    }
}

/* ================= MAIN FUNCTION ================= */

int main()
{
    char source[MAX_SOURCE] = "";
    char line[500];

    int n;

    printf("============================================\n");
    printf("          MINI COMPILER FOR TINY-C\n");
    printf("============================================\n");

    printf("\nEnter number of lines: ");

    if (scanf("%d", &n) != 1 || n < 1) {
        printf("ERROR: Invalid number of lines.\n");
        return 1;
    }

    getchar();

    printf("\nEnter your Tiny-C program:\n");

    for (int i = 0; i < n; i++) {

        if (fgets(line, sizeof(line), stdin) == NULL) {
            printf("ERROR: Unable to read source code.\n");
            return 1;
        }

        if (strlen(source) + strlen(line) >= MAX_SOURCE) {
            printf("ERROR: Source program is too large.\n");
            return 1;
        }

        strcat(source, line);
    }

    /* PHASE 1: LEXICAL ANALYSIS */

    printf("\n========== 1. LEXICAL ANALYSIS ==========\n");

    lexer(source);

    printf("\n%-20s %-20s\n", "LEXEME", "TOKEN");

    printf("----------------------------------------\n");

    for (int i = 0; i < tokenCount; i++) {

        printf("%-20s %-20s\n",
               tokens[i].text,
               tokenName(tokens[i].type));
    }

    /* PHASE 2: SYNTAX ANALYSIS */

    printf("\n========== 2. SYNTAX ANALYSIS ==========\n");

    pos = 0;

    Node *root = program();

    printf("Syntax Analysis Successful!\n");
    printf("No Syntax Errors Found.\n");

    /* PHASE 3: SEMANTIC ANALYSIS */

    printf("\n========== 3. SEMANTIC ANALYSIS ==========\n");

    printf("\nSymbol Table:\n");

    printf("----------------------------------------\n");

    for (int i = 0; i < symbolCount; i++) {

        printf("%d. %s : int\n",
               i + 1, symbolTable[i]);
    }

    printf("\nSemantic Analysis Successful!\n");
    printf("No Semantic Errors Found.\n");

    /* PHASE 4: ABSTRACT SYNTAX TREE */

    printf("\n========== 4. ABSTRACT SYNTAX TREE ==========\n");

    printAST(root, 0);

    /* PHASE 5: THREE ADDRESS CODE */

    printf("\n========== 5. THREE ADDRESS CODE ==========\n");

    generateTAC(root->left);

    printf("\n============================================\n");
    printf("          COMPILATION SUCCESSFUL\n");
    printf("============================================\n");

    return 0;
}
