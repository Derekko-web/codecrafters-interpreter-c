#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TOKEN_LEFT_PAREN,
    TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE,
    TOKEN_RIGHT_BRACE,
    TOKEN_COMMA,
    TOKEN_DOT,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_SEMICOLON,
    TOKEN_SLASH,
    TOKEN_STAR,
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_EQUAL,
    TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_AND,
    TOKEN_CLASS,
    TOKEN_ELSE,
    TOKEN_FALSE,
    TOKEN_FOR,
    TOKEN_FUN,
    TOKEN_IF,
    TOKEN_NIL,
    TOKEN_OR,
    TOKEN_PRINT,
    TOKEN_RETURN,
    TOKEN_SUPER,
    TOKEN_THIS,
    TOKEN_TRUE,
    TOKEN_VAR,
    TOKEN_WHILE,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    const char *start;
    size_t length;
    int line;
    double number;
} Token;

typedef struct {
    Token *items;
    size_t count;
    size_t capacity;
} TokenArray;

typedef struct {
    const char *source;
    size_t start;
    size_t current;
    int line;
    int had_error;
    TokenArray tokens;
} Scanner;

typedef enum {
    EXPR_BINARY,
    EXPR_GROUPING,
    EXPR_LITERAL,
    EXPR_UNARY,
    EXPR_VARIABLE
} ExprType;

typedef enum {
    LITERAL_NIL,
    LITERAL_BOOL,
    LITERAL_NUMBER,
    LITERAL_STRING
} LiteralType;

typedef enum {
    VALUE_NIL,
    VALUE_BOOL,
    VALUE_NUMBER,
    VALUE_STRING
} ValueType;

typedef enum {
    STMT_EXPRESSION,
    STMT_PRINT,
    STMT_VAR
} StmtType;

typedef struct Expr Expr;
typedef struct Stmt Stmt;

struct Expr {
    ExprType type;
    union {
        struct {
            Expr *left;
            Token operator_token;
            Expr *right;
        } binary;
        struct {
            Expr *expression;
        } grouping;
        struct {
            LiteralType type;
            int boolean;
            double number;
            const char *string_start;
            size_t string_length;
        } literal;
        struct {
            Token operator_token;
            Expr *right;
        } unary;
        struct {
            Token name;
        } variable;
    } as;
};

struct Stmt {
    StmtType type;
    union {
        struct {
            Expr *expression;
        } expression;
        struct {
            Expr *expression;
        } print;
        struct {
            Token name;
            Expr *initializer;
        } var;
    } as;
};

typedef struct {
    ValueType type;
    union {
        int boolean;
        double number;
        struct {
            const char *start;
            size_t length;
            int owns_memory;
        } string;
    } as;
} Value;

typedef struct {
    char *name;
    size_t length;
    Value value;
} EnvironmentEntry;

typedef struct {
    Stmt **items;
    size_t count;
    size_t capacity;
} StmtArray;

typedef struct {
    EnvironmentEntry *items;
    size_t count;
    size_t capacity;
} Environment;

typedef struct {
    TokenArray *tokens;
    size_t current;
    int had_error;
} Parser;

char *read_file_contents(const char *filename);
void *xmalloc(size_t size);
void *xrealloc(void *pointer, size_t size);
void free_token_array(TokenArray *tokens);
void append_token(TokenArray *tokens, Token token);
void free_stmt_array(StmtArray *statements);
void append_stmt(StmtArray *statements, Stmt *stmt);
void init_environment(Environment *environment);
void free_environment(Environment *environment);
void init_scanner(Scanner *scanner, const char *source);
int scan_tokens(Scanner *scanner);
void scan_token(Scanner *scanner);
void scan_string(Scanner *scanner);
void scan_number(Scanner *scanner);
void scan_identifier(Scanner *scanner);
void add_token(Scanner *scanner, TokenType type);
void add_number_token(Scanner *scanner, double number);
int scanner_is_at_end(const Scanner *scanner);
char scanner_advance(Scanner *scanner);
char scanner_peek(const Scanner *scanner);
char scanner_peek_next(const Scanner *scanner);
int scanner_match(Scanner *scanner, char expected);
void scanner_error(Scanner *scanner, int line, const char *message);
TokenType identifier_type(const char *start, size_t length);
const char *token_type_name(TokenType type);
void print_tokens(const TokenArray *tokens);
void print_token(const Token *token);
void print_token_lexeme(const Token *token);
void print_token_literal(const Token *token);
void format_number_literal(double value, char *buffer, size_t buffer_size);
void format_number_value(double value, char *buffer, size_t buffer_size);
double parse_number_slice(const char *start, size_t length);
int is_digit(char c);
int is_alpha(char c);
int is_alphanumeric(char c);
Expr *parse_expression(Parser *parser);
Expr *parse_equality(Parser *parser);
Expr *parse_comparison(Parser *parser);
Expr *parse_term(Parser *parser);
Expr *parse_factor(Parser *parser);
Expr *parse_unary(Parser *parser);
Expr *parse_primary(Parser *parser);
Stmt *parse_declaration(Parser *parser);
Stmt *parse_statement(Parser *parser);
Stmt *parse_var_declaration(Parser *parser);
Stmt *parse_print_statement(Parser *parser);
Stmt *parse_expression_statement(Parser *parser);
Expr *new_binary_expr(Expr *left, Token operator_token, Expr *right);
Expr *new_grouping_expr(Expr *expression);
Expr *new_nil_literal_expr(void);
Expr *new_boolean_literal_expr(int boolean);
Expr *new_number_literal_expr(double number);
Expr *new_string_literal_expr(const char *start, size_t length);
Expr *new_unary_expr(Token operator_token, Expr *right);
Expr *new_variable_expr(Token name);
Stmt *new_expression_stmt(Expr *expression);
Stmt *new_print_stmt(Expr *expression);
Stmt *new_var_stmt(Token name, Expr *initializer);
void free_expr(Expr *expr);
void free_stmt(Stmt *stmt);
int parser_is_at_end(const Parser *parser);
Token parser_peek(const Parser *parser);
Token parser_previous(const Parser *parser);
Token parser_advance(Parser *parser);
int parser_check(const Parser *parser, TokenType type);
int parser_match(Parser *parser, const TokenType *types, size_t count);
Token parser_consume(Parser *parser, TokenType type, const char *message);
void parser_error(Parser *parser, Token token, const char *message);
void print_expr(const Expr *expr);
void print_parenthesized(const char *name, size_t name_length, const Expr *const *expressions, size_t expression_count);
int scan_file_to_tokens(const char *filename, char **source_out, TokenArray *tokens_out);
int parse_file_to_expression(const char *filename, char **source_out, TokenArray *tokens_out, Expr **expression_out);
int parse_file_to_statements(const char *filename, char **source_out, TokenArray *tokens_out, StmtArray *statements_out);
Value evaluate_expr(const Expr *expr, const Environment *environment, int *had_runtime_error);
Value make_nil_value(void);
Value make_boolean_value(int boolean);
Value make_number_value(double number);
Value make_string_value(const char *start, size_t length);
char *copy_string_slice(const char *start, size_t length);
Value clone_value(Value value);
Value concatenate_strings(Value left, Value right);
void free_value(Value *value);
int find_environment_entry(const Environment *environment, const char *name, size_t length);
void define_variable(Environment *environment, Token name, Value value);
Value get_variable(const Environment *environment, Token name, int *had_runtime_error);
int is_truthy(Value value);
int values_equal(Value left, Value right);
void print_value(Value value);
int interpret_statement(const Stmt *stmt, Environment *environment);
int interpret_statements(const StmtArray *statements, Environment *environment);
int run_tokenize_command(const char *filename);
int run_parse_command(const char *filename);
int run_evaluate_command(const char *filename);
int run_run_command(const char *filename);

int main(int argc, char *argv[]) {
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    if (argc < 3) {
        fprintf(stderr, "Usage: ./your_program <command> <filename>\n");
        return 1;
    }

    if (strcmp(argv[1], "tokenize") == 0) {
        return run_tokenize_command(argv[2]);
    }

    if (strcmp(argv[1], "parse") == 0) {
        return run_parse_command(argv[2]);
    }

    if (strcmp(argv[1], "evaluate") == 0) {
        return run_evaluate_command(argv[2]);
    }

    if (strcmp(argv[1], "run") == 0) {
        return run_run_command(argv[2]);
    }

    fprintf(stderr, "Unknown command: %s\n", argv[1]);
    return 1;
}

int run_tokenize_command(const char *filename) {
    char *source = read_file_contents(filename);
    if (source == NULL) {
        return 1;
    }

    Scanner scanner;
    init_scanner(&scanner, source);
    int exit_code = scan_tokens(&scanner);

    print_tokens(&scanner.tokens);

    free_token_array(&scanner.tokens);
    free(source);
    return exit_code;
}

int run_parse_command(const char *filename) {
    char *source = NULL;
    TokenArray tokens = {0};
    Expr *expression = NULL;
    int exit_code = parse_file_to_expression(filename, &source, &tokens, &expression);
    if (exit_code != 0) {
        return exit_code;
    }

    print_expr(expression);
    printf("\n");

    free_expr(expression);
    free_token_array(&tokens);
    free(source);
    return 0;
}

int run_evaluate_command(const char *filename) {
    char *source = NULL;
    TokenArray tokens = {0};
    Expr *expression = NULL;
    Environment environment;
    init_environment(&environment);
    int exit_code = parse_file_to_expression(filename, &source, &tokens, &expression);
    if (exit_code != 0) {
        free_environment(&environment);
        return exit_code;
    }

    int had_runtime_error = 0;
    Value value = evaluate_expr(expression, &environment, &had_runtime_error);
    if (had_runtime_error) {
        free_expr(expression);
        free_token_array(&tokens);
        free(source);
        free_environment(&environment);
        return 70;
    }

    print_value(value);
    printf("\n");
    free_value(&value);

    free_expr(expression);
    free_token_array(&tokens);
    free(source);
    free_environment(&environment);
    return 0;
}

char *read_file_contents(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error reading file: %s\n", filename);
        return NULL;
    }

    if (fseek(file, 0, SEEK_END) != 0) {
        fprintf(stderr, "Error determining file size\n");
        fclose(file);
        return NULL;
    }

    long file_size_long = ftell(file);
    if (file_size_long < 0) {
        fprintf(stderr, "Error determining file size\n");
        fclose(file);
        return NULL;
    }

    rewind(file);

    size_t file_size = (size_t) file_size_long;
    char *file_contents = xmalloc(file_size + 1);

    size_t bytes_read = fread(file_contents, 1, file_size, file);
    if (bytes_read < file_size) {
        fprintf(stderr, "Error reading file contents\n");
        free(file_contents);
        fclose(file);
        return NULL;
    }

    file_contents[file_size] = '\0';
    fclose(file);
    return file_contents;
}

void *xmalloc(size_t size) {
    void *pointer = malloc(size);
    if (pointer == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    return pointer;
}

void *xrealloc(void *pointer, size_t size) {
    void *new_pointer = realloc(pointer, size);
    if (new_pointer == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(1);
    }

    return new_pointer;
}

void free_token_array(TokenArray *tokens) {
    free(tokens->items);
    tokens->items = NULL;
    tokens->count = 0;
    tokens->capacity = 0;
}

void free_stmt_array(StmtArray *statements) {
    for (size_t i = 0; i < statements->count; i++) {
        free_stmt(statements->items[i]);
    }

    free(statements->items);
    statements->items = NULL;
    statements->count = 0;
    statements->capacity = 0;
}

void append_token(TokenArray *tokens, Token token) {
    if (tokens->count == tokens->capacity) {
        size_t new_capacity = tokens->capacity < 8 ? 8 : tokens->capacity * 2;
        tokens->items = xrealloc(tokens->items, new_capacity * sizeof(Token));
        tokens->capacity = new_capacity;
    }

    tokens->items[tokens->count++] = token;
}

void append_stmt(StmtArray *statements, Stmt *stmt) {
    if (statements->count == statements->capacity) {
        size_t new_capacity = statements->capacity < 8 ? 8 : statements->capacity * 2;
        statements->items = xrealloc(statements->items, new_capacity * sizeof(Stmt *));
        statements->capacity = new_capacity;
    }

    statements->items[statements->count++] = stmt;
}

void init_environment(Environment *environment) {
    environment->items = NULL;
    environment->count = 0;
    environment->capacity = 0;
}

void free_environment(Environment *environment) {
    for (size_t i = 0; i < environment->count; i++) {
        free(environment->items[i].name);
        free_value(&environment->items[i].value);
    }

    free(environment->items);
    environment->items = NULL;
    environment->count = 0;
    environment->capacity = 0;
}

void init_scanner(Scanner *scanner, const char *source) {
    scanner->source = source;
    scanner->start = 0;
    scanner->current = 0;
    scanner->line = 1;
    scanner->had_error = 0;
    scanner->tokens.items = NULL;
    scanner->tokens.count = 0;
    scanner->tokens.capacity = 0;
}

int scan_tokens(Scanner *scanner) {
    while (!scanner_is_at_end(scanner)) {
        scanner->start = scanner->current;
        scan_token(scanner);
    }

    scanner->start = scanner->current;
    add_token(scanner, TOKEN_EOF);
    return scanner->had_error ? 65 : 0;
}

void scan_token(Scanner *scanner) {
    char c = scanner_advance(scanner);

    switch (c) {
        case '(':
            add_token(scanner, TOKEN_LEFT_PAREN);
            return;
        case ')':
            add_token(scanner, TOKEN_RIGHT_PAREN);
            return;
        case '{':
            add_token(scanner, TOKEN_LEFT_BRACE);
            return;
        case '}':
            add_token(scanner, TOKEN_RIGHT_BRACE);
            return;
        case ',':
            add_token(scanner, TOKEN_COMMA);
            return;
        case '.':
            add_token(scanner, TOKEN_DOT);
            return;
        case '-':
            add_token(scanner, TOKEN_MINUS);
            return;
        case '+':
            add_token(scanner, TOKEN_PLUS);
            return;
        case ';':
            add_token(scanner, TOKEN_SEMICOLON);
            return;
        case '*':
            add_token(scanner, TOKEN_STAR);
            return;
        case '!':
            add_token(scanner, scanner_match(scanner, '=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
            return;
        case '=':
            add_token(scanner, scanner_match(scanner, '=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
            return;
        case '<':
            add_token(scanner, scanner_match(scanner, '=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
            return;
        case '>':
            add_token(scanner, scanner_match(scanner, '=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
            return;
        case '/':
            if (scanner_match(scanner, '/')) {
                while (scanner_peek(scanner) != '\n' && !scanner_is_at_end(scanner)) {
                    scanner_advance(scanner);
                }
                return;
            }

            add_token(scanner, TOKEN_SLASH);
            return;
        case ' ':
        case '\r':
        case '\t':
            return;
        case '\n':
            scanner->line++;
            return;
        case '"':
            scan_string(scanner);
            return;
        default:
            if (is_digit(c)) {
                scan_number(scanner);
                return;
            }

            if (is_alpha(c)) {
                scan_identifier(scanner);
                return;
            }

            fprintf(stderr, "[line %d] Error: Unexpected character: %c\n", scanner->line, c);
            scanner->had_error = 1;
            return;
    }
}

void scan_string(Scanner *scanner) {
    while (scanner_peek(scanner) != '"' && !scanner_is_at_end(scanner)) {
        if (scanner_peek(scanner) == '\n') {
            scanner->line++;
        }

        scanner_advance(scanner);
    }

    if (scanner_is_at_end(scanner)) {
        scanner_error(scanner, scanner->line, "Unterminated string.");
        return;
    }

    scanner_advance(scanner);
    add_token(scanner, TOKEN_STRING);
}

void scan_number(Scanner *scanner) {
    while (is_digit(scanner_peek(scanner))) {
        scanner_advance(scanner);
    }

    if (scanner_peek(scanner) == '.' && is_digit(scanner_peek_next(scanner))) {
        scanner_advance(scanner);

        while (is_digit(scanner_peek(scanner))) {
            scanner_advance(scanner);
        }
    }

    add_number_token(scanner, parse_number_slice(scanner->source + scanner->start, scanner->current - scanner->start));
}

void scan_identifier(Scanner *scanner) {
    while (is_alphanumeric(scanner_peek(scanner))) {
        scanner_advance(scanner);
    }

    size_t length = scanner->current - scanner->start;
    add_token(scanner, identifier_type(scanner->source + scanner->start, length));
}

void add_token(Scanner *scanner, TokenType type) {
    Token token = {
        .type = type,
        .start = scanner->source + scanner->start,
        .length = scanner->current - scanner->start,
        .line = scanner->line,
        .number = 0,
    };

    append_token(&scanner->tokens, token);
}

void add_number_token(Scanner *scanner, double number) {
    Token token = {
        .type = TOKEN_NUMBER,
        .start = scanner->source + scanner->start,
        .length = scanner->current - scanner->start,
        .line = scanner->line,
        .number = number,
    };

    append_token(&scanner->tokens, token);
}

int scanner_is_at_end(const Scanner *scanner) {
    return scanner->source[scanner->current] == '\0';
}

char scanner_advance(Scanner *scanner) {
    char c = scanner->source[scanner->current];
    scanner->current++;
    return c;
}

char scanner_peek(const Scanner *scanner) {
    if (scanner_is_at_end(scanner)) {
        return '\0';
    }

    return scanner->source[scanner->current];
}

char scanner_peek_next(const Scanner *scanner) {
    if (scanner->source[scanner->current] == '\0' || scanner->source[scanner->current + 1] == '\0') {
        return '\0';
    }

    return scanner->source[scanner->current + 1];
}

int scanner_match(Scanner *scanner, char expected) {
    if (scanner_is_at_end(scanner)) {
        return 0;
    }

    if (scanner->source[scanner->current] != expected) {
        return 0;
    }

    scanner->current++;
    return 1;
}

void scanner_error(Scanner *scanner, int line, const char *message) {
    fprintf(stderr, "[line %d] Error: %s\n", line, message);
    scanner->had_error = 1;
}

TokenType identifier_type(const char *start, size_t length) {
    if (length == 2 && strncmp(start, "if", 2) == 0) return TOKEN_IF;
    if (length == 2 && strncmp(start, "or", 2) == 0) return TOKEN_OR;
    if (length == 3 && strncmp(start, "and", 3) == 0) return TOKEN_AND;
    if (length == 3 && strncmp(start, "for", 3) == 0) return TOKEN_FOR;
    if (length == 3 && strncmp(start, "fun", 3) == 0) return TOKEN_FUN;
    if (length == 3 && strncmp(start, "nil", 3) == 0) return TOKEN_NIL;
    if (length == 3 && strncmp(start, "var", 3) == 0) return TOKEN_VAR;
    if (length == 4 && strncmp(start, "else", 4) == 0) return TOKEN_ELSE;
    if (length == 4 && strncmp(start, "true", 4) == 0) return TOKEN_TRUE;
    if (length == 4 && strncmp(start, "this", 4) == 0) return TOKEN_THIS;
    if (length == 5 && strncmp(start, "class", 5) == 0) return TOKEN_CLASS;
    if (length == 5 && strncmp(start, "false", 5) == 0) return TOKEN_FALSE;
    if (length == 5 && strncmp(start, "print", 5) == 0) return TOKEN_PRINT;
    if (length == 5 && strncmp(start, "super", 5) == 0) return TOKEN_SUPER;
    if (length == 5 && strncmp(start, "while", 5) == 0) return TOKEN_WHILE;
    if (length == 6 && strncmp(start, "return", 6) == 0) return TOKEN_RETURN;
    return TOKEN_IDENTIFIER;
}

const char *token_type_name(TokenType type) {
    switch (type) {
        case TOKEN_LEFT_PAREN: return "LEFT_PAREN";
        case TOKEN_RIGHT_PAREN: return "RIGHT_PAREN";
        case TOKEN_LEFT_BRACE: return "LEFT_BRACE";
        case TOKEN_RIGHT_BRACE: return "RIGHT_BRACE";
        case TOKEN_COMMA: return "COMMA";
        case TOKEN_DOT: return "DOT";
        case TOKEN_MINUS: return "MINUS";
        case TOKEN_PLUS: return "PLUS";
        case TOKEN_SEMICOLON: return "SEMICOLON";
        case TOKEN_SLASH: return "SLASH";
        case TOKEN_STAR: return "STAR";
        case TOKEN_BANG: return "BANG";
        case TOKEN_BANG_EQUAL: return "BANG_EQUAL";
        case TOKEN_EQUAL: return "EQUAL";
        case TOKEN_EQUAL_EQUAL: return "EQUAL_EQUAL";
        case TOKEN_GREATER: return "GREATER";
        case TOKEN_GREATER_EQUAL: return "GREATER_EQUAL";
        case TOKEN_LESS: return "LESS";
        case TOKEN_LESS_EQUAL: return "LESS_EQUAL";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_AND: return "AND";
        case TOKEN_CLASS: return "CLASS";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_FALSE: return "FALSE";
        case TOKEN_FOR: return "FOR";
        case TOKEN_FUN: return "FUN";
        case TOKEN_IF: return "IF";
        case TOKEN_NIL: return "NIL";
        case TOKEN_OR: return "OR";
        case TOKEN_PRINT: return "PRINT";
        case TOKEN_RETURN: return "RETURN";
        case TOKEN_SUPER: return "SUPER";
        case TOKEN_THIS: return "THIS";
        case TOKEN_TRUE: return "TRUE";
        case TOKEN_VAR: return "VAR";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_EOF: return "EOF";
    }

    return "UNKNOWN";
}

void print_tokens(const TokenArray *tokens) {
    for (size_t i = 0; i < tokens->count; i++) {
        print_token(&tokens->items[i]);
    }
}

void print_token(const Token *token) {
    printf("%s ", token_type_name(token->type));
    print_token_lexeme(token);
    printf(" ");
    print_token_literal(token);
    printf("\n");
}

void print_token_lexeme(const Token *token) {
    printf("%.*s", (int) token->length, token->start);
}

void print_token_literal(const Token *token) {
    switch (token->type) {
        case TOKEN_STRING:
            printf("%.*s", (int) (token->length - 2), token->start + 1);
            return;
        case TOKEN_NUMBER: {
            char literal[64];
            format_number_literal(token->number, literal, sizeof(literal));
            printf("%s", literal);
            return;
        }
        default:
            printf("null");
            return;
    }
}

void format_number_literal(double value, char *buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, "%.15g", value);

    if (strchr(buffer, '.') == NULL && strchr(buffer, 'e') == NULL && strchr(buffer, 'E') == NULL) {
        size_t length = strlen(buffer);
        if (length + 2 < buffer_size) {
            buffer[length] = '.';
            buffer[length + 1] = '0';
            buffer[length + 2] = '\0';
        }
    }
}

void format_number_value(double value, char *buffer, size_t buffer_size) {
    snprintf(buffer, buffer_size, "%.15g", value);
}

double parse_number_slice(const char *start, size_t length) {
    char *buffer = xmalloc(length + 1);
    memcpy(buffer, start, length);
    buffer[length] = '\0';

    double value = strtod(buffer, NULL);
    free(buffer);
    return value;
}

int is_digit(char c) {
    return c >= '0' && c <= '9';
}

int is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

int is_alphanumeric(char c) {
    return is_alpha(c) || is_digit(c);
}

Expr *parse_expression(Parser *parser) {
    return parse_equality(parser);
}

Expr *parse_equality(Parser *parser) {
    Expr *expression = parse_comparison(parser);
    if (expression == NULL) {
        return NULL;
    }

    TokenType operators[] = {TOKEN_BANG_EQUAL, TOKEN_EQUAL_EQUAL};
    while (parser_match(parser, operators, sizeof(operators) / sizeof(operators[0]))) {
        Token operator_token = parser_previous(parser);
        Expr *right = parse_comparison(parser);
        if (right == NULL) {
            free_expr(expression);
            return NULL;
        }

        expression = new_binary_expr(expression, operator_token, right);
    }

    return expression;
}

Expr *parse_comparison(Parser *parser) {
    Expr *expression = parse_term(parser);
    if (expression == NULL) {
        return NULL;
    }

    TokenType operators[] = {
        TOKEN_GREATER,
        TOKEN_GREATER_EQUAL,
        TOKEN_LESS,
        TOKEN_LESS_EQUAL,
    };

    while (parser_match(parser, operators, sizeof(operators) / sizeof(operators[0]))) {
        Token operator_token = parser_previous(parser);
        Expr *right = parse_term(parser);
        if (right == NULL) {
            free_expr(expression);
            return NULL;
        }

        expression = new_binary_expr(expression, operator_token, right);
    }

    return expression;
}

Expr *parse_term(Parser *parser) {
    Expr *expression = parse_factor(parser);
    if (expression == NULL) {
        return NULL;
    }

    TokenType operators[] = {TOKEN_MINUS, TOKEN_PLUS};
    while (parser_match(parser, operators, sizeof(operators) / sizeof(operators[0]))) {
        Token operator_token = parser_previous(parser);
        Expr *right = parse_factor(parser);
        if (right == NULL) {
            free_expr(expression);
            return NULL;
        }

        expression = new_binary_expr(expression, operator_token, right);
    }

    return expression;
}

Expr *parse_factor(Parser *parser) {
    Expr *expression = parse_unary(parser);
    if (expression == NULL) {
        return NULL;
    }

    TokenType operators[] = {TOKEN_SLASH, TOKEN_STAR};
    while (parser_match(parser, operators, sizeof(operators) / sizeof(operators[0]))) {
        Token operator_token = parser_previous(parser);
        Expr *right = parse_unary(parser);
        if (right == NULL) {
            free_expr(expression);
            return NULL;
        }

        expression = new_binary_expr(expression, operator_token, right);
    }

    return expression;
}

Expr *parse_unary(Parser *parser) {
    TokenType operators[] = {TOKEN_BANG, TOKEN_MINUS};
    if (parser_match(parser, operators, sizeof(operators) / sizeof(operators[0]))) {
        Token operator_token = parser_previous(parser);
        Expr *right = parse_unary(parser);
        if (right == NULL) {
            return NULL;
        }

        return new_unary_expr(operator_token, right);
    }

    return parse_primary(parser);
}

Expr *parse_primary(Parser *parser) {
    if (parser_match(parser, &(TokenType){TOKEN_FALSE}, 1)) {
        return new_boolean_literal_expr(0);
    }

    if (parser_match(parser, &(TokenType){TOKEN_TRUE}, 1)) {
        return new_boolean_literal_expr(1);
    }

    if (parser_match(parser, &(TokenType){TOKEN_NIL}, 1)) {
        return new_nil_literal_expr();
    }

    if (parser_match(parser, &(TokenType){TOKEN_NUMBER}, 1)) {
        return new_number_literal_expr(parser_previous(parser).number);
    }

    if (parser_match(parser, &(TokenType){TOKEN_STRING}, 1)) {
        Token string_token = parser_previous(parser);
        return new_string_literal_expr(string_token.start + 1, string_token.length - 2);
    }

    if (parser_match(parser, &(TokenType){TOKEN_IDENTIFIER}, 1)) {
        return new_variable_expr(parser_previous(parser));
    }

    if (parser_match(parser, &(TokenType){TOKEN_LEFT_PAREN}, 1)) {
        Expr *expression = parse_expression(parser);
        if (expression == NULL) {
            return NULL;
        }

        parser_consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after expression.");
        if (parser->had_error) {
            free_expr(expression);
            return NULL;
        }

        return new_grouping_expr(expression);
    }

    parser_error(parser, parser_peek(parser), "Expect expression.");
    return NULL;
}

Stmt *parse_declaration(Parser *parser) {
    if (parser_match(parser, &(TokenType){TOKEN_VAR}, 1)) {
        return parse_var_declaration(parser);
    }

    return parse_statement(parser);
}

Stmt *parse_statement(Parser *parser) {
    if (parser_match(parser, &(TokenType){TOKEN_PRINT}, 1)) {
        return parse_print_statement(parser);
    }

    return parse_expression_statement(parser);
}

Stmt *parse_var_declaration(Parser *parser) {
    Token name = parser_consume(parser, TOKEN_IDENTIFIER, "Expect variable name.");
    if (parser->had_error) {
        return NULL;
    }

    Expr *initializer = NULL;
    if (parser_match(parser, &(TokenType){TOKEN_EQUAL}, 1)) {
        initializer = parse_expression(parser);
        if (initializer == NULL) {
            return NULL;
        }
    }

    parser_consume(parser, TOKEN_SEMICOLON, "Expect ';' after variable declaration.");
    if (parser->had_error) {
        free_expr(initializer);
        return NULL;
    }

    return new_var_stmt(name, initializer);
}

Stmt *parse_print_statement(Parser *parser) {
    Expr *expression = parse_expression(parser);
    if (expression == NULL) {
        return NULL;
    }

    parser_consume(parser, TOKEN_SEMICOLON, "Expect ';' after value.");
    if (parser->had_error) {
        free_expr(expression);
        return NULL;
    }

    return new_print_stmt(expression);
}

Stmt *parse_expression_statement(Parser *parser) {
    Expr *expression = parse_expression(parser);
    if (expression == NULL) {
        return NULL;
    }

    parser_consume(parser, TOKEN_SEMICOLON, "Expect ';' after expression.");
    if (parser->had_error) {
        free_expr(expression);
        return NULL;
    }

    return new_expression_stmt(expression);
}

Expr *new_binary_expr(Expr *left, Token operator_token, Expr *right) {
    Expr *expr = xmalloc(sizeof(Expr));
    expr->type = EXPR_BINARY;
    expr->as.binary.left = left;
    expr->as.binary.operator_token = operator_token;
    expr->as.binary.right = right;
    return expr;
}

Expr *new_grouping_expr(Expr *expression) {
    Expr *expr = xmalloc(sizeof(Expr));
    expr->type = EXPR_GROUPING;
    expr->as.grouping.expression = expression;
    return expr;
}

Expr *new_nil_literal_expr(void) {
    Expr *expr = xmalloc(sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->as.literal.type = LITERAL_NIL;
    expr->as.literal.boolean = 0;
    expr->as.literal.number = 0;
    expr->as.literal.string_start = NULL;
    expr->as.literal.string_length = 0;
    return expr;
}

Expr *new_boolean_literal_expr(int boolean) {
    Expr *expr = xmalloc(sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->as.literal.type = LITERAL_BOOL;
    expr->as.literal.boolean = boolean;
    expr->as.literal.number = 0;
    expr->as.literal.string_start = NULL;
    expr->as.literal.string_length = 0;
    return expr;
}

Expr *new_number_literal_expr(double number) {
    Expr *expr = xmalloc(sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->as.literal.type = LITERAL_NUMBER;
    expr->as.literal.boolean = 0;
    expr->as.literal.number = number;
    expr->as.literal.string_start = NULL;
    expr->as.literal.string_length = 0;
    return expr;
}

Expr *new_string_literal_expr(const char *start, size_t length) {
    Expr *expr = xmalloc(sizeof(Expr));
    expr->type = EXPR_LITERAL;
    expr->as.literal.type = LITERAL_STRING;
    expr->as.literal.boolean = 0;
    expr->as.literal.number = 0;
    expr->as.literal.string_start = start;
    expr->as.literal.string_length = length;
    return expr;
}

Expr *new_unary_expr(Token operator_token, Expr *right) {
    Expr *expr = xmalloc(sizeof(Expr));
    expr->type = EXPR_UNARY;
    expr->as.unary.operator_token = operator_token;
    expr->as.unary.right = right;
    return expr;
}

Expr *new_variable_expr(Token name) {
    Expr *expr = xmalloc(sizeof(Expr));
    expr->type = EXPR_VARIABLE;
    expr->as.variable.name = name;
    return expr;
}

Stmt *new_expression_stmt(Expr *expression) {
    Stmt *stmt = xmalloc(sizeof(Stmt));
    stmt->type = STMT_EXPRESSION;
    stmt->as.expression.expression = expression;
    return stmt;
}

Stmt *new_print_stmt(Expr *expression) {
    Stmt *stmt = xmalloc(sizeof(Stmt));
    stmt->type = STMT_PRINT;
    stmt->as.print.expression = expression;
    return stmt;
}

Stmt *new_var_stmt(Token name, Expr *initializer) {
    Stmt *stmt = xmalloc(sizeof(Stmt));
    stmt->type = STMT_VAR;
    stmt->as.var.name = name;
    stmt->as.var.initializer = initializer;
    return stmt;
}

void free_expr(Expr *expr) {
    if (expr == NULL) {
        return;
    }

    switch (expr->type) {
        case EXPR_BINARY:
            free_expr(expr->as.binary.left);
            free_expr(expr->as.binary.right);
            break;
        case EXPR_GROUPING:
            free_expr(expr->as.grouping.expression);
            break;
        case EXPR_UNARY:
            free_expr(expr->as.unary.right);
            break;
        case EXPR_VARIABLE:
        case EXPR_LITERAL:
            break;
    }

    free(expr);
}

void free_stmt(Stmt *stmt) {
    if (stmt == NULL) {
        return;
    }

    switch (stmt->type) {
        case STMT_EXPRESSION:
            free_expr(stmt->as.expression.expression);
            break;
        case STMT_PRINT:
            free_expr(stmt->as.print.expression);
            break;
        case STMT_VAR:
            free_expr(stmt->as.var.initializer);
            break;
    }

    free(stmt);
}

int parser_is_at_end(const Parser *parser) {
    return parser_peek(parser).type == TOKEN_EOF;
}

Token parser_peek(const Parser *parser) {
    return parser->tokens->items[parser->current];
}

Token parser_previous(const Parser *parser) {
    return parser->tokens->items[parser->current - 1];
}

Token parser_advance(Parser *parser) {
    if (!parser_is_at_end(parser)) {
        parser->current++;
    }

    return parser_previous(parser);
}

int parser_check(const Parser *parser, TokenType type) {
    if (parser_is_at_end(parser)) {
        return type == TOKEN_EOF;
    }

    return parser_peek(parser).type == type;
}

int parser_match(Parser *parser, const TokenType *types, size_t count) {
    for (size_t i = 0; i < count; i++) {
        if (parser_check(parser, types[i])) {
            parser_advance(parser);
            return 1;
        }
    }

    return 0;
}

Token parser_consume(Parser *parser, TokenType type, const char *message) {
    if (parser_check(parser, type)) {
        return parser_advance(parser);
    }

    parser_error(parser, parser_peek(parser), message);
    return parser_peek(parser);
}

void parser_error(Parser *parser, Token token, const char *message) {
    if (token.type == TOKEN_EOF) {
        fprintf(stderr, "[line %d] Error at end: %s\n", token.line, message);
    } else {
        fprintf(stderr, "[line %d] Error at '%.*s': %s\n", token.line, (int) token.length, token.start, message);
    }

    parser->had_error = 1;
}

void print_expr(const Expr *expr) {
    switch (expr->type) {
        case EXPR_BINARY: {
            const Expr *expressions[] = {
                expr->as.binary.left,
                expr->as.binary.right,
            };
            print_parenthesized(
                expr->as.binary.operator_token.start,
                expr->as.binary.operator_token.length,
                expressions,
                sizeof(expressions) / sizeof(expressions[0]));
            return;
        }
        case EXPR_GROUPING: {
            const Expr *expressions[] = {expr->as.grouping.expression};
            print_parenthesized("group", strlen("group"), expressions, sizeof(expressions) / sizeof(expressions[0]));
            return;
        }
        case EXPR_LITERAL:
            switch (expr->as.literal.type) {
                case LITERAL_NIL:
                    printf("nil");
                    return;
                case LITERAL_BOOL:
                    printf("%s", expr->as.literal.boolean ? "true" : "false");
                    return;
                case LITERAL_NUMBER: {
                    char literal[64];
                    format_number_literal(expr->as.literal.number, literal, sizeof(literal));
                    printf("%s", literal);
                    return;
                }
                case LITERAL_STRING:
                    printf("%.*s", (int) expr->as.literal.string_length, expr->as.literal.string_start);
                    return;
            }
            return;
        case EXPR_UNARY: {
            const Expr *expressions[] = {expr->as.unary.right};
            print_parenthesized(
                expr->as.unary.operator_token.start,
                expr->as.unary.operator_token.length,
                expressions,
                sizeof(expressions) / sizeof(expressions[0]));
            return;
        }
        case EXPR_VARIABLE:
            printf("%.*s", (int) expr->as.variable.name.length, expr->as.variable.name.start);
            return;
    }
}

void print_parenthesized(const char *name, size_t name_length, const Expr *const *expressions, size_t expression_count) {
    printf("(");
    printf("%.*s", (int) name_length, name);

    for (size_t i = 0; i < expression_count; i++) {
        printf(" ");
        print_expr(expressions[i]);
    }

    printf(")");
}

int scan_file_to_tokens(const char *filename, char **source_out, TokenArray *tokens_out) {
    *source_out = NULL;
    tokens_out->items = NULL;
    tokens_out->count = 0;
    tokens_out->capacity = 0;

    char *source = read_file_contents(filename);
    if (source == NULL) {
        return 1;
    }

    Scanner scanner;
    init_scanner(&scanner, source);
    int scan_exit_code = scan_tokens(&scanner);

    if (scan_exit_code != 0) {
        free_token_array(&scanner.tokens);
        free(source);
        return scan_exit_code;
    }

    *source_out = source;
    *tokens_out = scanner.tokens;
    return 0;
}

int parse_file_to_expression(const char *filename, char **source_out, TokenArray *tokens_out, Expr **expression_out) {
    *expression_out = NULL;

    int scan_exit_code = scan_file_to_tokens(filename, source_out, tokens_out);
    if (scan_exit_code != 0) {
        return scan_exit_code;
    }

    Parser parser = {
        .tokens = tokens_out,
        .current = 0,
        .had_error = 0,
    };

    Expr *expression = parse_expression(&parser);
    if (!parser.had_error && !parser_is_at_end(&parser)) {
        parser_error(&parser, parser_peek(&parser), "Expect end of expression.");
    }

    if (expression == NULL || parser.had_error) {
        free_expr(expression);
        free_token_array(tokens_out);
        free(*source_out);
        return 65;
    }

    *expression_out = expression;
    return 0;
}

int parse_file_to_statements(const char *filename, char **source_out, TokenArray *tokens_out, StmtArray *statements_out) {
    statements_out->items = NULL;
    statements_out->count = 0;
    statements_out->capacity = 0;

    int scan_exit_code = scan_file_to_tokens(filename, source_out, tokens_out);
    if (scan_exit_code != 0) {
        return scan_exit_code;
    }

    Parser parser = {
        .tokens = tokens_out,
        .current = 0,
        .had_error = 0,
    };

    while (!parser_is_at_end(&parser)) {
        Stmt *statement = parse_declaration(&parser);
        if (statement == NULL) {
            break;
        }

        append_stmt(statements_out, statement);
    }

    if (parser.had_error) {
        free_stmt_array(statements_out);
        free_token_array(tokens_out);
        free(*source_out);
        return 65;
    }

    return 0;
}

Value evaluate_expr(const Expr *expr, const Environment *environment, int *had_runtime_error) {
    switch (expr->type) {
        case EXPR_LITERAL:
            switch (expr->as.literal.type) {
                case LITERAL_NIL:
                    return make_nil_value();
                case LITERAL_BOOL:
                    return make_boolean_value(expr->as.literal.boolean);
                case LITERAL_NUMBER:
                    return make_number_value(expr->as.literal.number);
                case LITERAL_STRING:
                    return make_string_value(expr->as.literal.string_start, expr->as.literal.string_length);
            }
            break;
        case EXPR_GROUPING:
            return evaluate_expr(expr->as.grouping.expression, environment, had_runtime_error);
        case EXPR_UNARY: {
            Value right = evaluate_expr(expr->as.unary.right, environment, had_runtime_error);
            if (*had_runtime_error) {
                free_value(&right);
                return make_nil_value();
            }

            switch (expr->as.unary.operator_token.type) {
                case TOKEN_BANG:
                    {
                        Value result = make_boolean_value(!is_truthy(right));
                        free_value(&right);
                        return result;
                    }
                case TOKEN_MINUS:
                    if (right.type != VALUE_NUMBER) {
                        fprintf(stderr, "[line %d] Runtime error: Operand must be a number.\n", expr->as.unary.operator_token.line);
                        *had_runtime_error = 1;
                        free_value(&right);
                        return make_nil_value();
                    }
                    {
                        Value result = make_number_value(-right.as.number);
                        free_value(&right);
                        return result;
                    }
                default:
                    break;
            }
            free_value(&right);
            break;
        }
        case EXPR_BINARY: {
            Value left = evaluate_expr(expr->as.binary.left, environment, had_runtime_error);
            if (*had_runtime_error) {
                free_value(&left);
                return make_nil_value();
            }

            Value right = evaluate_expr(expr->as.binary.right, environment, had_runtime_error);
            if (*had_runtime_error) {
                free_value(&left);
                free_value(&right);
                return make_nil_value();
            }

            switch (expr->as.binary.operator_token.type) {
                case TOKEN_BANG_EQUAL:
                    {
                        Value result = make_boolean_value(!values_equal(left, right));
                        free_value(&left);
                        free_value(&right);
                        return result;
                    }
                case TOKEN_EQUAL_EQUAL:
                    {
                        Value result = make_boolean_value(values_equal(left, right));
                        free_value(&left);
                        free_value(&right);
                        return result;
                    }
                case TOKEN_GREATER:
                    if (left.type != VALUE_NUMBER || right.type != VALUE_NUMBER) {
                        fprintf(stderr, "[line %d] Runtime error: Operands must be numbers.\n", expr->as.binary.operator_token.line);
                        *had_runtime_error = 1;
                        free_value(&left);
                        free_value(&right);
                        return make_nil_value();
                    }
                    {
                        Value result = make_boolean_value(left.as.number > right.as.number);
                        free_value(&left);
                        free_value(&right);
                        return result;
                    }
                case TOKEN_GREATER_EQUAL:
                    if (left.type != VALUE_NUMBER || right.type != VALUE_NUMBER) {
                        fprintf(stderr, "[line %d] Runtime error: Operands must be numbers.\n", expr->as.binary.operator_token.line);
                        *had_runtime_error = 1;
                        free_value(&left);
                        free_value(&right);
                        return make_nil_value();
                    }
                    {
                        Value result = make_boolean_value(left.as.number >= right.as.number);
                        free_value(&left);
                        free_value(&right);
                        return result;
                    }
                case TOKEN_LESS:
                    if (left.type != VALUE_NUMBER || right.type != VALUE_NUMBER) {
                        fprintf(stderr, "[line %d] Runtime error: Operands must be numbers.\n", expr->as.binary.operator_token.line);
                        *had_runtime_error = 1;
                        free_value(&left);
                        free_value(&right);
                        return make_nil_value();
                    }
                    {
                        Value result = make_boolean_value(left.as.number < right.as.number);
                        free_value(&left);
                        free_value(&right);
                        return result;
                    }
                case TOKEN_LESS_EQUAL:
                    if (left.type != VALUE_NUMBER || right.type != VALUE_NUMBER) {
                        fprintf(stderr, "[line %d] Runtime error: Operands must be numbers.\n", expr->as.binary.operator_token.line);
                        *had_runtime_error = 1;
                        free_value(&left);
                        free_value(&right);
                        return make_nil_value();
                    }
                    {
                        Value result = make_boolean_value(left.as.number <= right.as.number);
                        free_value(&left);
                        free_value(&right);
                        return result;
                    }
                case TOKEN_MINUS:
                    if (left.type != VALUE_NUMBER || right.type != VALUE_NUMBER) {
                        fprintf(stderr, "[line %d] Runtime error: Operands must be numbers.\n", expr->as.binary.operator_token.line);
                        *had_runtime_error = 1;
                        free_value(&left);
                        free_value(&right);
                        return make_nil_value();
                    }
                    {
                        Value result = make_number_value(left.as.number - right.as.number);
                        free_value(&left);
                        free_value(&right);
                        return result;
                    }
                case TOKEN_PLUS:
                    if (left.type == VALUE_STRING && right.type == VALUE_STRING) {
                        Value result = concatenate_strings(left, right);
                        free_value(&left);
                        free_value(&right);
                        return result;
                    }
                    if (left.type != VALUE_NUMBER || right.type != VALUE_NUMBER) {
                        fprintf(stderr, "[line %d] Runtime error: Operands must be two numbers or two strings.\n", expr->as.binary.operator_token.line);
                        *had_runtime_error = 1;
                        free_value(&left);
                        free_value(&right);
                        return make_nil_value();
                    }
                    {
                        Value result = make_number_value(left.as.number + right.as.number);
                        free_value(&left);
                        free_value(&right);
                        return result;
                    }
                case TOKEN_SLASH:
                    if (left.type != VALUE_NUMBER || right.type != VALUE_NUMBER) {
                        fprintf(stderr, "[line %d] Runtime error: Operands must be numbers.\n", expr->as.binary.operator_token.line);
                        *had_runtime_error = 1;
                        free_value(&left);
                        free_value(&right);
                        return make_nil_value();
                    }
                    {
                        Value result = make_number_value(left.as.number / right.as.number);
                        free_value(&left);
                        free_value(&right);
                        return result;
                    }
                case TOKEN_STAR:
                    if (left.type != VALUE_NUMBER || right.type != VALUE_NUMBER) {
                        fprintf(stderr, "[line %d] Runtime error: Operands must be numbers.\n", expr->as.binary.operator_token.line);
                        *had_runtime_error = 1;
                        free_value(&left);
                        free_value(&right);
                        return make_nil_value();
                    }
                    {
                        Value result = make_number_value(left.as.number * right.as.number);
                        free_value(&left);
                        free_value(&right);
                        return result;
                    }
                default:
                    break;
            }
            free_value(&left);
            free_value(&right);
            break;
        }
        case EXPR_VARIABLE:
            return get_variable(environment, expr->as.variable.name, had_runtime_error);
    }

    fprintf(stderr, "Runtime error: Unsupported expression.\n");
    *had_runtime_error = 1;
    return make_nil_value();
}

Value make_nil_value(void) {
    Value value = {.type = VALUE_NIL};
    return value;
}

Value make_boolean_value(int boolean) {
    Value value = {
        .type = VALUE_BOOL,
        .as.boolean = boolean,
    };
    return value;
}

Value make_number_value(double number) {
    Value value = {
        .type = VALUE_NUMBER,
        .as.number = number,
    };
    return value;
}

Value make_string_value(const char *start, size_t length) {
    Value value = {
        .type = VALUE_STRING,
        .as.string.start = start,
        .as.string.length = length,
        .as.string.owns_memory = 0,
    };
    return value;
}

char *copy_string_slice(const char *start, size_t length) {
    char *buffer = xmalloc(length + 1);
    memcpy(buffer, start, length);
    buffer[length] = '\0';
    return buffer;
}

Value clone_value(Value value) {
    switch (value.type) {
        case VALUE_NIL:
            return make_nil_value();
        case VALUE_BOOL:
            return make_boolean_value(value.as.boolean);
        case VALUE_NUMBER:
            return make_number_value(value.as.number);
        case VALUE_STRING: {
            char *buffer = copy_string_slice(value.as.string.start, value.as.string.length);
            Value copy = {
                .type = VALUE_STRING,
                .as.string.start = buffer,
                .as.string.length = value.as.string.length,
                .as.string.owns_memory = 1,
            };
            return copy;
        }
    }

    return make_nil_value();
}

Value concatenate_strings(Value left, Value right) {
    size_t length = left.as.string.length + right.as.string.length;
    char *buffer = xmalloc(length + 1);
    memcpy(buffer, left.as.string.start, left.as.string.length);
    memcpy(buffer + left.as.string.length, right.as.string.start, right.as.string.length);
    buffer[length] = '\0';

    Value value = {
        .type = VALUE_STRING,
        .as.string.start = buffer,
        .as.string.length = length,
        .as.string.owns_memory = 1,
    };
    return value;
}

void free_value(Value *value) {
    if (value->type == VALUE_STRING && value->as.string.owns_memory) {
        free((void *) value->as.string.start);
    }

    *value = make_nil_value();
}

int find_environment_entry(const Environment *environment, const char *name, size_t length) {
    if (environment == NULL) {
        return -1;
    }

    for (size_t i = 0; i < environment->count; i++) {
        if (environment->items[i].length == length &&
            strncmp(environment->items[i].name, name, length) == 0) {
            return (int) i;
        }
    }

    return -1;
}

void define_variable(Environment *environment, Token name, Value value) {
    int existing_index = find_environment_entry(environment, name.start, name.length);
    Value stored_value = clone_value(value);

    if (existing_index >= 0) {
        free_value(&environment->items[existing_index].value);
        environment->items[existing_index].value = stored_value;
        return;
    }

    if (environment->count == environment->capacity) {
        size_t new_capacity = environment->capacity < 8 ? 8 : environment->capacity * 2;
        environment->items = xrealloc(environment->items, new_capacity * sizeof(EnvironmentEntry));
        environment->capacity = new_capacity;
    }

    environment->items[environment->count].name = copy_string_slice(name.start, name.length);
    environment->items[environment->count].length = name.length;
    environment->items[environment->count].value = stored_value;
    environment->count++;
}

Value get_variable(const Environment *environment, Token name, int *had_runtime_error) {
    int index = find_environment_entry(environment, name.start, name.length);
    if (index < 0) {
        fprintf(stderr, "[line %d] Runtime error: Undefined variable '%.*s'.\n", name.line, (int) name.length, name.start);
        *had_runtime_error = 1;
        return make_nil_value();
    }

    return clone_value(environment->items[index].value);
}

int is_truthy(Value value) {
    switch (value.type) {
        case VALUE_NIL:
            return 0;
        case VALUE_BOOL:
            return value.as.boolean;
        case VALUE_NUMBER:
        case VALUE_STRING:
            return 1;
    }

    return 0;
}

int values_equal(Value left, Value right) {
    if (left.type != right.type) {
        return 0;
    }

    switch (left.type) {
        case VALUE_NIL:
            return 1;
        case VALUE_BOOL:
            return left.as.boolean == right.as.boolean;
        case VALUE_NUMBER:
            return left.as.number == right.as.number;
        case VALUE_STRING:
            return left.as.string.length == right.as.string.length &&
                   strncmp(left.as.string.start, right.as.string.start, left.as.string.length) == 0;
    }

    return 0;
}

void print_value(Value value) {
    switch (value.type) {
        case VALUE_NIL:
            printf("nil");
            return;
        case VALUE_BOOL:
            printf("%s", value.as.boolean ? "true" : "false");
            return;
        case VALUE_NUMBER: {
            char literal[64];
            format_number_value(value.as.number, literal, sizeof(literal));
            printf("%s", literal);
            return;
        }
        case VALUE_STRING:
            printf("%.*s", (int) value.as.string.length, value.as.string.start);
            return;
    }
}

int interpret_statement(const Stmt *stmt, Environment *environment) {
    int had_runtime_error = 0;
    Value value;

    switch (stmt->type) {
        case STMT_EXPRESSION:
            value = evaluate_expr(stmt->as.expression.expression, environment, &had_runtime_error);
            free_value(&value);
            return had_runtime_error ? 70 : 0;
        case STMT_PRINT:
            value = evaluate_expr(stmt->as.print.expression, environment, &had_runtime_error);
            if (had_runtime_error) {
                free_value(&value);
                return 70;
            }
            print_value(value);
            printf("\n");
            free_value(&value);
            return 0;
        case STMT_VAR:
            if (stmt->as.var.initializer == NULL) {
                value = make_nil_value();
            } else {
                value = evaluate_expr(stmt->as.var.initializer, environment, &had_runtime_error);
                if (had_runtime_error) {
                    free_value(&value);
                    return 70;
                }
            }
            define_variable(environment, stmt->as.var.name, value);
            free_value(&value);
            return 0;
    }

    return 70;
}

int interpret_statements(const StmtArray *statements, Environment *environment) {
    for (size_t i = 0; i < statements->count; i++) {
        int exit_code = interpret_statement(statements->items[i], environment);
        if (exit_code != 0) {
            return exit_code;
        }
    }

    return 0;
}

int run_run_command(const char *filename) {
    char *source = NULL;
    TokenArray tokens = {0};
    StmtArray statements = {0};
    Environment environment;
    init_environment(&environment);
    int exit_code = parse_file_to_statements(filename, &source, &tokens, &statements);
    if (exit_code != 0) {
        free_environment(&environment);
        return exit_code;
    }

    exit_code = interpret_statements(&statements, &environment);

    free_stmt_array(&statements);
    free_token_array(&tokens);
    free(source);
    free_environment(&environment);
    return exit_code;
}
