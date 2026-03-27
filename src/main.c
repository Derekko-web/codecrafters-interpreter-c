#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

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
    EXPR_VARIABLE,
    EXPR_ASSIGN,
    EXPR_LOGICAL,
    EXPR_CALL,
    EXPR_GET,
    EXPR_SET
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
    VALUE_STRING,
    VALUE_CLASS,
    VALUE_INSTANCE,
    VALUE_NATIVE_FUNCTION,
    VALUE_FUNCTION
} ValueType;

typedef enum {
    STMT_EXPRESSION,
    STMT_PRINT,
    STMT_VAR,
    STMT_CLASS,
    STMT_FUNCTION,
    STMT_RETURN,
    STMT_BLOCK,
    STMT_IF,
    STMT_WHILE
} StmtType;

typedef struct Expr Expr;
typedef struct Stmt Stmt;
typedef struct ExprArray {
    Expr **items;
    size_t count;
    size_t capacity;
} ExprArray;
typedef struct StmtArray {
    Stmt **items;
    size_t count;
    size_t capacity;
} StmtArray;
typedef struct NativeFunction NativeFunction;
typedef struct ClassObject ClassObject;
typedef struct ClassMethodEntry ClassMethodEntry;
typedef struct InstanceObject InstanceObject;
typedef struct EnvironmentEntry EnvironmentEntry;
typedef struct Value Value;
typedef struct Environment Environment;

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
            size_t resolved_depth;
            int is_resolved;
        } variable;
        struct {
            Token name;
            Expr *value;
            size_t resolved_depth;
            int is_resolved;
        } assign;
        struct {
            Expr *left;
            Token operator_token;
            Expr *right;
        } logical;
        struct {
            Expr *callee;
            Token paren;
            ExprArray arguments;
        } call;
        struct {
            Expr *object;
            Token name;
        } get;
        struct {
            Expr *object;
            Token name;
            Expr *value;
        } set;
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
        struct {
            Token name;
            Expr *superclass;
            StmtArray methods;
        } class_statement;
        struct {
            Token name;
            TokenArray parameters;
            StmtArray body;
        } function;
        struct {
            Token keyword;
            Expr *value;
        } return_statement;
        struct {
            StmtArray statements;
        } block;
        struct {
            Expr *condition;
            Stmt *then_branch;
            Stmt *else_branch;
        } if_statement;
        struct {
            Expr *condition;
            Stmt *body;
        } while_statement;
    } as;
};

struct Value {
    ValueType type;
    union {
        int boolean;
        double number;
        struct {
            const char *start;
            size_t length;
            int owns_memory;
        } string;
        ClassObject *class_object;
        InstanceObject *instance_object;
        const NativeFunction *native_function;
        struct {
            const Stmt *declaration;
            Environment *closure;
            int is_initializer;
        } function;
    } as;
};

struct NativeFunction {
    const char *name;
    int arity;
    Value (*function)(int argument_count, const Value *arguments);
};

struct ClassObject {
    char *name;
    ClassObject *superclass;
    ClassMethodEntry *methods;
    size_t method_count;
    size_t method_capacity;
    size_t ref_count;
};

struct ClassMethodEntry {
    char *name;
    size_t length;
    Value value;
};

struct InstanceObject {
    ClassObject *class_object;
    EnvironmentEntry *fields;
    size_t field_count;
    size_t field_capacity;
    size_t ref_count;
};

struct EnvironmentEntry {
    char *name;
    size_t length;
    Value value;
};

struct Environment {
    EnvironmentEntry *items;
    size_t count;
    size_t capacity;
    size_t ref_count;
    Environment *enclosing;
};

typedef struct {
    TokenArray *tokens;
    size_t current;
    int had_error;
} Parser;

typedef struct {
    char *name;
    size_t length;
    int is_defined;
} ResolverScopeEntry;

typedef struct {
    ResolverScopeEntry *items;
    size_t count;
    size_t capacity;
} ResolverScope;

typedef enum {
    RESOLVER_FUNCTION_NONE,
    RESOLVER_FUNCTION_FUNCTION,
    RESOLVER_FUNCTION_METHOD,
    RESOLVER_FUNCTION_INITIALIZER
} ResolverFunctionType;

typedef enum {
    RESOLVER_CLASS_NONE,
    RESOLVER_CLASS_CLASS
} ResolverClassType;

typedef struct {
    ResolverScope *items;
    size_t count;
    size_t capacity;
    int had_error;
    ResolverFunctionType current_function;
    ResolverClassType current_class;
} Resolver;

char *read_file_contents(const char *filename);
void *xmalloc(size_t size);
void *xrealloc(void *pointer, size_t size);
void free_token_array(TokenArray *tokens);
void free_expr_array(ExprArray *expressions);
void append_token(TokenArray *tokens, Token token);
void append_expr(ExprArray *expressions, Expr *expression);
void free_stmt_array(StmtArray *statements);
void append_stmt(StmtArray *statements, Stmt *stmt);
Environment *new_environment(void);
Environment *new_enclosed_environment(Environment *enclosing);
Environment *retain_environment(Environment *environment);
void release_environment(Environment *environment);
Environment *ancestor_environment(Environment *environment, size_t distance);
Environment *global_environment(Environment *environment);
void define_native_functions(Environment *environment);
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
Expr *parse_assignment(Parser *parser);
Expr *parse_or(Parser *parser);
Expr *parse_and(Parser *parser);
Expr *parse_equality(Parser *parser);
Expr *parse_comparison(Parser *parser);
Expr *parse_term(Parser *parser);
Expr *parse_factor(Parser *parser);
Expr *parse_unary(Parser *parser);
Expr *parse_call(Parser *parser);
Expr *parse_finish_call(Parser *parser, Expr *callee);
Expr *parse_primary(Parser *parser);
Stmt *parse_declaration(Parser *parser);
Stmt *parse_class_declaration(Parser *parser);
Stmt *parse_function_declaration(Parser *parser);
Stmt *parse_statement(Parser *parser);
Stmt *parse_var_declaration(Parser *parser);
int parse_block_contents(Parser *parser, StmtArray *statements_out);
Stmt *parse_block_statement(Parser *parser);
Stmt *parse_if_statement(Parser *parser);
Stmt *parse_for_statement(Parser *parser);
Stmt *parse_while_statement(Parser *parser);
Stmt *parse_return_statement(Parser *parser);
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
Expr *new_assign_expr(Token name, Expr *value);
Expr *new_logical_expr(Expr *left, Token operator_token, Expr *right);
Expr *new_call_expr(Expr *callee, Token paren, ExprArray arguments);
Expr *new_get_expr(Expr *object, Token name);
Expr *new_set_expr(Expr *object, Token name, Expr *value);
Stmt *new_expression_stmt(Expr *expression);
Stmt *new_print_stmt(Expr *expression);
Stmt *new_var_stmt(Token name, Expr *initializer);
Stmt *new_class_stmt(Token name, Expr *superclass, StmtArray methods);
Stmt *new_function_stmt(Token name, TokenArray parameters, StmtArray body);
Stmt *new_return_stmt(Token keyword, Expr *value);
Stmt *new_block_stmt(StmtArray statements);
Stmt *new_if_stmt(Expr *condition, Stmt *then_branch, Stmt *else_branch);
Stmt *new_while_stmt(Expr *condition, Stmt *body);
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
void free_resolver_scope(ResolverScope *scope);
void init_resolver(Resolver *resolver);
void free_resolver(Resolver *resolver);
void append_resolver_scope(Resolver *resolver, ResolverScope scope);
void resolver_begin_scope(Resolver *resolver);
void resolver_end_scope(Resolver *resolver);
int resolver_find_scope_entry(const ResolverScope *scope, const char *name, size_t length);
void resolver_declare(Resolver *resolver, Token name);
void resolver_define(Resolver *resolver, Token name);
void resolver_resolve_local_variable(Resolver *resolver, Expr *expr);
void resolver_resolve_local_assignment(Resolver *resolver, Expr *expr);
void resolver_resolve_function(Resolver *resolver, const Stmt *stmt, ResolverFunctionType function_type);
void resolver_resolve_expr(Resolver *resolver, Expr *expr);
void resolver_resolve_statement(Resolver *resolver, const Stmt *stmt);
void resolver_resolve_statements(Resolver *resolver, const StmtArray *statements);
void resolver_error(Resolver *resolver, Token token, const char *message);
Value evaluate_expr(const Expr *expr, Environment *environment, int *had_runtime_error);
Value make_nil_value(void);
Value make_boolean_value(int boolean);
Value make_number_value(double number);
Value make_string_value(const char *start, size_t length);
ClassObject *new_class_object(const char *name, size_t length, ClassObject *superclass);
ClassObject *retain_class_object(ClassObject *class_object);
void release_class_object(ClassObject *class_object);
Value make_class_value(ClassObject *class_object);
InstanceObject *new_instance_object(ClassObject *class_object);
InstanceObject *retain_instance_object(InstanceObject *instance_object);
void release_instance_object(InstanceObject *instance_object);
Value make_instance_value(InstanceObject *instance_object);
Value make_native_function_value(const NativeFunction *native_function);
Value make_function_value(const Stmt *declaration, Environment *closure, int is_initializer);
Value bind_method(Value method, InstanceObject *instance_object);
char *copy_string_slice(const char *start, size_t length);
Value clone_value(Value value);
Value concatenate_strings(Value left, Value right);
void free_value(Value *value);
Value native_clock(int argument_count, const Value *arguments);
void runtime_error(int line, const char *format, ...);
int find_instance_field(const InstanceObject *instance_object, const char *name, size_t length);
void set_instance_field(InstanceObject *instance_object, Token name, Value value);
Value get_instance_field(const InstanceObject *instance_object, Token name, int *had_runtime_error);
int find_class_method(const ClassObject *class_object, const char *name, size_t length);
const Value *find_inherited_class_method(const ClassObject *class_object, const char *name, size_t length);
void set_class_method(ClassObject *class_object, Token name, Value value);
int find_environment_entry(const Environment *environment, const char *name, size_t length);
int is_initializer_name(Token name);
Value get_bound_this_value(Value function, int *had_runtime_error);
int call_function_value(
    Value callee,
    size_t argument_count,
    const Value *arguments,
    int line,
    int *had_runtime_error,
    Value *result_out);
void define_variable(Environment *environment, Token name, Value value);
Value get_resolved_variable(const Environment *environment, size_t distance, Token name, int *had_runtime_error);
Value get_variable(const Environment *environment, Token name, int *had_runtime_error);
int assign_resolved_variable(Environment *environment, size_t distance, Token name, Value value, int *had_runtime_error);
int assign_variable(Environment *environment, Token name, Value value, int *had_runtime_error);
int is_truthy(Value value);
int values_equal(Value left, Value right);
void print_value(Value value);
int interpret_statement(const Stmt *stmt, Environment *environment, int *did_return, Value *return_value);
int interpret_statements(const StmtArray *statements, Environment *environment, int *did_return, Value *return_value);
int run_tokenize_command(const char *filename);
int run_parse_command(const char *filename);
int run_evaluate_command(const char *filename);
int run_run_command(const char *filename);

static const NativeFunction CLOCK_NATIVE_FUNCTION = {
    .name = "clock",
    .arity = 0,
    .function = native_clock,
};

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
    Environment *environment = new_environment();
    define_native_functions(environment);
    int exit_code = parse_file_to_expression(filename, &source, &tokens, &expression);
    if (exit_code != 0) {
        release_environment(environment);
        return exit_code;
    }

    int had_runtime_error = 0;
    Value value = evaluate_expr(expression, environment, &had_runtime_error);
    if (had_runtime_error) {
        free_expr(expression);
        free_token_array(&tokens);
        free(source);
        release_environment(environment);
        return 70;
    }

    print_value(value);
    printf("\n");
    free_value(&value);

    free_expr(expression);
    free_token_array(&tokens);
    free(source);
    release_environment(environment);
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

void free_expr_array(ExprArray *expressions) {
    for (size_t i = 0; i < expressions->count; i++) {
        free_expr(expressions->items[i]);
    }

    free(expressions->items);
    expressions->items = NULL;
    expressions->count = 0;
    expressions->capacity = 0;
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

void append_expr(ExprArray *expressions, Expr *expression) {
    if (expressions->count == expressions->capacity) {
        size_t new_capacity = expressions->capacity < 8 ? 8 : expressions->capacity * 2;
        expressions->items = xrealloc(expressions->items, new_capacity * sizeof(Expr *));
        expressions->capacity = new_capacity;
    }

    expressions->items[expressions->count++] = expression;
}

void append_stmt(StmtArray *statements, Stmt *stmt) {
    if (statements->count == statements->capacity) {
        size_t new_capacity = statements->capacity < 8 ? 8 : statements->capacity * 2;
        statements->items = xrealloc(statements->items, new_capacity * sizeof(Stmt *));
        statements->capacity = new_capacity;
    }

    statements->items[statements->count++] = stmt;
}

Environment *new_environment(void) {
    Environment *environment = xmalloc(sizeof(Environment));
    environment->items = NULL;
    environment->count = 0;
    environment->capacity = 0;
    environment->ref_count = 1;
    environment->enclosing = NULL;
    return environment;
}

Environment *new_enclosed_environment(Environment *enclosing) {
    Environment *environment = new_environment();
    environment->enclosing = retain_environment(enclosing);
    return environment;
}

Environment *retain_environment(Environment *environment) {
    if (environment != NULL) {
        environment->ref_count++;
    }

    return environment;
}

Environment *ancestor_environment(Environment *environment, size_t distance) {
    Environment *current = environment;
    for (size_t i = 0; i < distance; i++) {
        if (current == NULL) {
            return NULL;
        }

        current = current->enclosing;
    }

    return current;
}

Environment *global_environment(Environment *environment) {
    Environment *current = environment;
    while (current != NULL && current->enclosing != NULL) {
        current = current->enclosing;
    }

    return current;
}

void define_native_functions(Environment *environment) {
    Token clock_name = {
        .type = TOKEN_IDENTIFIER,
        .start = "clock",
        .length = strlen("clock"),
        .line = 0,
        .number = 0,
    };

    Value clock = make_native_function_value(&CLOCK_NATIVE_FUNCTION);
    define_variable(environment, clock_name, clock);
    free_value(&clock);
}

void release_environment(Environment *environment) {
    if (environment == NULL) {
        return;
    }

    if (environment->ref_count > 1) {
        environment->ref_count--;
        return;
    }

    for (size_t i = 0; i < environment->count; i++) {
        free(environment->items[i].name);
        free_value(&environment->items[i].value);
    }

    free(environment->items);
    Environment *enclosing = environment->enclosing;
    free(environment);
    release_environment(enclosing);
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
    return parse_assignment(parser);
}

Expr *parse_assignment(Parser *parser) {
    Expr *expression = parse_or(parser);
    if (expression == NULL) {
        return NULL;
    }

    if (parser_match(parser, &(TokenType){TOKEN_EQUAL}, 1)) {
        Token equals = parser_previous(parser);
        Expr *value = parse_assignment(parser);
        if (value == NULL) {
            free_expr(expression);
            return NULL;
        }

        if (expression->type == EXPR_VARIABLE) {
            Token name = expression->as.variable.name;
            free_expr(expression);
            return new_assign_expr(name, value);
        }

        if (expression->type == EXPR_GET) {
            Expr *object = expression->as.get.object;
            Token name = expression->as.get.name;
            free(expression);
            return new_set_expr(object, name, value);
        }

        parser_error(parser, equals, "Invalid assignment target.");
        free_expr(expression);
        free_expr(value);
        return NULL;
    }

    return expression;
}

Expr *parse_or(Parser *parser) {
    Expr *expression = parse_and(parser);
    if (expression == NULL) {
        return NULL;
    }

    while (parser_match(parser, &(TokenType){TOKEN_OR}, 1)) {
        Token operator_token = parser_previous(parser);
        Expr *right = parse_and(parser);
        if (right == NULL) {
            free_expr(expression);
            return NULL;
        }

        expression = new_logical_expr(expression, operator_token, right);
    }

    return expression;
}

Expr *parse_and(Parser *parser) {
    Expr *expression = parse_equality(parser);
    if (expression == NULL) {
        return NULL;
    }

    while (parser_match(parser, &(TokenType){TOKEN_AND}, 1)) {
        Token operator_token = parser_previous(parser);
        Expr *right = parse_equality(parser);
        if (right == NULL) {
            free_expr(expression);
            return NULL;
        }

        expression = new_logical_expr(expression, operator_token, right);
    }

    return expression;
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

    return parse_call(parser);
}

Expr *parse_call(Parser *parser) {
    Expr *expression = parse_primary(parser);
    if (expression == NULL) {
        return NULL;
    }

    while (1) {
        if (parser_match(parser, &(TokenType){TOKEN_LEFT_PAREN}, 1)) {
            expression = parse_finish_call(parser, expression);
            if (expression == NULL) {
                return NULL;
            }
        } else if (parser_match(parser, &(TokenType){TOKEN_DOT}, 1)) {
            Token name = parser_consume(parser, TOKEN_IDENTIFIER, "Expect property name after '.'.");
            if (parser->had_error) {
                free_expr(expression);
                return NULL;
            }

            expression = new_get_expr(expression, name);
        } else {
            break;
        }
    }

    return expression;
}

Expr *parse_finish_call(Parser *parser, Expr *callee) {
    ExprArray arguments = {0};

    if (!parser_check(parser, TOKEN_RIGHT_PAREN)) {
        do {
            if (arguments.count >= 255) {
                parser_error(parser, parser_peek(parser), "Can't have more than 255 arguments.");
            }

            Expr *argument = parse_expression(parser);
            if (argument == NULL) {
                free_expr(callee);
                free_expr_array(&arguments);
                return NULL;
            }

            append_expr(&arguments, argument);
        } while (parser_match(parser, &(TokenType){TOKEN_COMMA}, 1));
    }

    Token paren = parser_consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after arguments.");
    if (parser->had_error) {
        free_expr(callee);
        free_expr_array(&arguments);
        return NULL;
    }

    return new_call_expr(callee, paren, arguments);
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

    if (parser_match(parser, &(TokenType){TOKEN_THIS}, 1)) {
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
    if (parser_match(parser, &(TokenType){TOKEN_CLASS}, 1)) {
        return parse_class_declaration(parser);
    }

    if (parser_match(parser, &(TokenType){TOKEN_FUN}, 1)) {
        return parse_function_declaration(parser);
    }

    if (parser_match(parser, &(TokenType){TOKEN_VAR}, 1)) {
        return parse_var_declaration(parser);
    }

    return parse_statement(parser);
}

Stmt *parse_class_declaration(Parser *parser) {
    Token name = parser_consume(parser, TOKEN_IDENTIFIER, "Expect class name.");
    if (parser->had_error) {
        return NULL;
    }

    Expr *superclass = NULL;
    if (parser_match(parser, &(TokenType){TOKEN_LESS}, 1)) {
        Token superclass_name = parser_consume(parser, TOKEN_IDENTIFIER, "Expect superclass name.");
        if (parser->had_error) {
            return NULL;
        }

        superclass = new_variable_expr(superclass_name);
    }

    parser_consume(parser, TOKEN_LEFT_BRACE, "Expect '{' before class body.");
    if (parser->had_error) {
        free_expr(superclass);
        return NULL;
    }

    StmtArray methods = {0};
    while (!parser_check(parser, TOKEN_RIGHT_BRACE) && !parser_is_at_end(parser)) {
        Stmt *method = parse_function_declaration(parser);
        if (method == NULL) {
            free_expr(superclass);
            free_stmt_array(&methods);
            return NULL;
        }

        append_stmt(&methods, method);
    }

    parser_consume(parser, TOKEN_RIGHT_BRACE, "Expect '}' after class body.");
    if (parser->had_error) {
        free_expr(superclass);
        free_stmt_array(&methods);
        return NULL;
    }

    return new_class_stmt(name, superclass, methods);
}

Stmt *parse_function_declaration(Parser *parser) {
    Token name = parser_consume(parser, TOKEN_IDENTIFIER, "Expect function name.");
    if (parser->had_error) {
        return NULL;
    }

    parser_consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after function name.");
    if (parser->had_error) {
        return NULL;
    }

    TokenArray parameters = {0};
    if (!parser_check(parser, TOKEN_RIGHT_PAREN)) {
        do {
            if (parameters.count >= 255) {
                parser_error(parser, parser_peek(parser), "Can't have more than 255 parameters.");
            }

            Token parameter = parser_consume(parser, TOKEN_IDENTIFIER, "Expect parameter name.");
            if (parser->had_error) {
                free_token_array(&parameters);
                return NULL;
            }

            append_token(&parameters, parameter);
        } while (parser_match(parser, &(TokenType){TOKEN_COMMA}, 1));
    }

    parser_consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after parameters.");
    if (parser->had_error) {
        free_token_array(&parameters);
        return NULL;
    }

    parser_consume(parser, TOKEN_LEFT_BRACE, "Expect '{' before function body.");
    if (parser->had_error) {
        free_token_array(&parameters);
        return NULL;
    }

    StmtArray body = {0};
    if (!parse_block_contents(parser, &body)) {
        free_token_array(&parameters);
        return NULL;
    }

    return new_function_stmt(name, parameters, body);
}

Stmt *parse_statement(Parser *parser) {
    if (parser_match(parser, &(TokenType){TOKEN_LEFT_BRACE}, 1)) {
        return parse_block_statement(parser);
    }

    if (parser_match(parser, &(TokenType){TOKEN_FOR}, 1)) {
        return parse_for_statement(parser);
    }

    if (parser_match(parser, &(TokenType){TOKEN_IF}, 1)) {
        return parse_if_statement(parser);
    }

    if (parser_match(parser, &(TokenType){TOKEN_WHILE}, 1)) {
        return parse_while_statement(parser);
    }

    if (parser_match(parser, &(TokenType){TOKEN_RETURN}, 1)) {
        return parse_return_statement(parser);
    }

    if (parser_match(parser, &(TokenType){TOKEN_PRINT}, 1)) {
        return parse_print_statement(parser);
    }

    return parse_expression_statement(parser);
}

Stmt *parse_for_statement(Parser *parser) {
    parser_consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'for'.");
    if (parser->had_error) {
        return NULL;
    }

    Stmt *initializer = NULL;
    if (parser_match(parser, &(TokenType){TOKEN_SEMICOLON}, 1)) {
        initializer = NULL;
    } else if (parser_match(parser, &(TokenType){TOKEN_VAR}, 1)) {
        initializer = parse_var_declaration(parser);
    } else {
        initializer = parse_expression_statement(parser);
    }

    if (parser->had_error) {
        free_stmt(initializer);
        return NULL;
    }

    Expr *condition = NULL;
    if (!parser_check(parser, TOKEN_SEMICOLON)) {
        condition = parse_expression(parser);
        if (condition == NULL) {
            free_stmt(initializer);
            return NULL;
        }
    }

    parser_consume(parser, TOKEN_SEMICOLON, "Expect ';' after loop condition.");
    if (parser->had_error) {
        free_stmt(initializer);
        free_expr(condition);
        return NULL;
    }

    Expr *increment = NULL;
    if (!parser_check(parser, TOKEN_RIGHT_PAREN)) {
        increment = parse_expression(parser);
        if (increment == NULL) {
            free_stmt(initializer);
            free_expr(condition);
            return NULL;
        }
    }

    parser_consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after for clauses.");
    if (parser->had_error) {
        free_stmt(initializer);
        free_expr(condition);
        free_expr(increment);
        return NULL;
    }

    Stmt *body = parse_statement(parser);
    if (body == NULL) {
        free_stmt(initializer);
        free_expr(condition);
        free_expr(increment);
        return NULL;
    }

    if (increment != NULL) {
        StmtArray statements = {
            .items = NULL,
            .count = 0,
            .capacity = 0,
        };
        append_stmt(&statements, body);
        append_stmt(&statements, new_expression_stmt(increment));
        body = new_block_stmt(statements);
    }

    if (condition == NULL) {
        condition = new_boolean_literal_expr(1);
    }

    body = new_while_stmt(condition, body);

    if (initializer != NULL) {
        StmtArray statements = {
            .items = NULL,
            .count = 0,
            .capacity = 0,
        };
        append_stmt(&statements, initializer);
        append_stmt(&statements, body);
        body = new_block_stmt(statements);
    }

    return body;
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

int parse_block_contents(Parser *parser, StmtArray *statements_out) {
    statements_out->items = NULL;
    statements_out->count = 0;
    statements_out->capacity = 0;

    while (!parser_check(parser, TOKEN_RIGHT_BRACE) && !parser_is_at_end(parser)) {
        Stmt *statement = parse_declaration(parser);
        if (statement == NULL) {
            free_stmt_array(statements_out);
            return 0;
        }

        append_stmt(statements_out, statement);
    }

    parser_consume(parser, TOKEN_RIGHT_BRACE, "Expect '}' .");
    if (parser->had_error) {
        free_stmt_array(statements_out);
        return 0;
    }

    return 1;
}

Stmt *parse_block_statement(Parser *parser) {
    StmtArray statements = {0};
    if (!parse_block_contents(parser, &statements)) {
        return NULL;
    }

    return new_block_stmt(statements);
}

Stmt *parse_if_statement(Parser *parser) {
    parser_consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'if'.");
    if (parser->had_error) {
        return NULL;
    }

    Expr *condition = parse_expression(parser);
    if (condition == NULL) {
        return NULL;
    }

    parser_consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after if condition.");
    if (parser->had_error) {
        free_expr(condition);
        return NULL;
    }

    Stmt *then_branch = parse_statement(parser);
    if (then_branch == NULL) {
        free_expr(condition);
        return NULL;
    }

    Stmt *else_branch = NULL;
    if (parser_match(parser, &(TokenType){TOKEN_ELSE}, 1)) {
        else_branch = parse_statement(parser);
        if (else_branch == NULL) {
            free_expr(condition);
            free_stmt(then_branch);
            return NULL;
        }
    }

    return new_if_stmt(condition, then_branch, else_branch);
}

Stmt *parse_while_statement(Parser *parser) {
    parser_consume(parser, TOKEN_LEFT_PAREN, "Expect '(' after 'while'.");
    if (parser->had_error) {
        return NULL;
    }

    Expr *condition = parse_expression(parser);
    if (condition == NULL) {
        return NULL;
    }

    parser_consume(parser, TOKEN_RIGHT_PAREN, "Expect ')' after while condition.");
    if (parser->had_error) {
        free_expr(condition);
        return NULL;
    }

    Stmt *body = parse_statement(parser);
    if (body == NULL) {
        free_expr(condition);
        return NULL;
    }

    return new_while_stmt(condition, body);
}

Stmt *parse_return_statement(Parser *parser) {
    Token keyword = parser_previous(parser);
    Expr *value = NULL;

    if (!parser_check(parser, TOKEN_SEMICOLON)) {
        value = parse_expression(parser);
        if (value == NULL) {
            return NULL;
        }
    }

    parser_consume(parser, TOKEN_SEMICOLON, "Expect ';' after return value.");
    if (parser->had_error) {
        free_expr(value);
        return NULL;
    }

    return new_return_stmt(keyword, value);
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
    expr->as.variable.resolved_depth = 0;
    expr->as.variable.is_resolved = 0;
    return expr;
}

Expr *new_assign_expr(Token name, Expr *value) {
    Expr *expr = xmalloc(sizeof(Expr));
    expr->type = EXPR_ASSIGN;
    expr->as.assign.name = name;
    expr->as.assign.value = value;
    expr->as.assign.resolved_depth = 0;
    expr->as.assign.is_resolved = 0;
    return expr;
}

Expr *new_logical_expr(Expr *left, Token operator_token, Expr *right) {
    Expr *expr = xmalloc(sizeof(Expr));
    expr->type = EXPR_LOGICAL;
    expr->as.logical.left = left;
    expr->as.logical.operator_token = operator_token;
    expr->as.logical.right = right;
    return expr;
}

Expr *new_call_expr(Expr *callee, Token paren, ExprArray arguments) {
    Expr *expr = xmalloc(sizeof(Expr));
    expr->type = EXPR_CALL;
    expr->as.call.callee = callee;
    expr->as.call.paren = paren;
    expr->as.call.arguments = arguments;
    return expr;
}

Expr *new_get_expr(Expr *object, Token name) {
    Expr *expr = xmalloc(sizeof(Expr));
    expr->type = EXPR_GET;
    expr->as.get.object = object;
    expr->as.get.name = name;
    return expr;
}

Expr *new_set_expr(Expr *object, Token name, Expr *value) {
    Expr *expr = xmalloc(sizeof(Expr));
    expr->type = EXPR_SET;
    expr->as.set.object = object;
    expr->as.set.name = name;
    expr->as.set.value = value;
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

Stmt *new_class_stmt(Token name, Expr *superclass, StmtArray methods) {
    Stmt *stmt = xmalloc(sizeof(Stmt));
    stmt->type = STMT_CLASS;
    stmt->as.class_statement.name = name;
    stmt->as.class_statement.superclass = superclass;
    stmt->as.class_statement.methods = methods;
    return stmt;
}

Stmt *new_function_stmt(Token name, TokenArray parameters, StmtArray body) {
    Stmt *stmt = xmalloc(sizeof(Stmt));
    stmt->type = STMT_FUNCTION;
    stmt->as.function.name = name;
    stmt->as.function.parameters = parameters;
    stmt->as.function.body = body;
    return stmt;
}

Stmt *new_return_stmt(Token keyword, Expr *value) {
    Stmt *stmt = xmalloc(sizeof(Stmt));
    stmt->type = STMT_RETURN;
    stmt->as.return_statement.keyword = keyword;
    stmt->as.return_statement.value = value;
    return stmt;
}

Stmt *new_block_stmt(StmtArray statements) {
    Stmt *stmt = xmalloc(sizeof(Stmt));
    stmt->type = STMT_BLOCK;
    stmt->as.block.statements = statements;
    return stmt;
}

Stmt *new_if_stmt(Expr *condition, Stmt *then_branch, Stmt *else_branch) {
    Stmt *stmt = xmalloc(sizeof(Stmt));
    stmt->type = STMT_IF;
    stmt->as.if_statement.condition = condition;
    stmt->as.if_statement.then_branch = then_branch;
    stmt->as.if_statement.else_branch = else_branch;
    return stmt;
}

Stmt *new_while_stmt(Expr *condition, Stmt *body) {
    Stmt *stmt = xmalloc(sizeof(Stmt));
    stmt->type = STMT_WHILE;
    stmt->as.while_statement.condition = condition;
    stmt->as.while_statement.body = body;
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
        case EXPR_ASSIGN:
            free_expr(expr->as.assign.value);
            break;
        case EXPR_LOGICAL:
            free_expr(expr->as.logical.left);
            free_expr(expr->as.logical.right);
            break;
        case EXPR_CALL:
            free_expr(expr->as.call.callee);
            free_expr_array(&expr->as.call.arguments);
            break;
        case EXPR_GET:
            free_expr(expr->as.get.object);
            break;
        case EXPR_SET:
            free_expr(expr->as.set.object);
            free_expr(expr->as.set.value);
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
        case STMT_CLASS:
            free_expr(stmt->as.class_statement.superclass);
            free_stmt_array(&stmt->as.class_statement.methods);
            break;
        case STMT_FUNCTION:
            free_token_array(&stmt->as.function.parameters);
            free_stmt_array(&stmt->as.function.body);
            break;
        case STMT_RETURN:
            free_expr(stmt->as.return_statement.value);
            break;
        case STMT_BLOCK:
            free_stmt_array(&stmt->as.block.statements);
            break;
        case STMT_IF:
            free_expr(stmt->as.if_statement.condition);
            free_stmt(stmt->as.if_statement.then_branch);
            free_stmt(stmt->as.if_statement.else_branch);
            break;
        case STMT_WHILE:
            free_expr(stmt->as.while_statement.condition);
            free_stmt(stmt->as.while_statement.body);
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
        case EXPR_ASSIGN: {
            printf("(= %.*s ", (int) expr->as.assign.name.length, expr->as.assign.name.start);
            print_expr(expr->as.assign.value);
            printf(")");
            return;
        }
        case EXPR_LOGICAL: {
            const Expr *expressions[] = {
                expr->as.logical.left,
                expr->as.logical.right,
            };
            print_parenthesized(
                expr->as.logical.operator_token.start,
                expr->as.logical.operator_token.length,
                expressions,
                sizeof(expressions) / sizeof(expressions[0]));
            return;
        }
        case EXPR_CALL:
            printf("(call ");
            print_expr(expr->as.call.callee);
            for (size_t i = 0; i < expr->as.call.arguments.count; i++) {
                printf(" ");
                print_expr(expr->as.call.arguments.items[i]);
            }
            printf(")");
            return;
        case EXPR_GET:
            printf("(. ");
            print_expr(expr->as.get.object);
            printf(" %.*s)", (int) expr->as.get.name.length, expr->as.get.name.start);
            return;
        case EXPR_SET:
            printf("(= ");
            print_expr(expr->as.set.object);
            printf(" %.*s ", (int) expr->as.set.name.length, expr->as.set.name.start);
            print_expr(expr->as.set.value);
            printf(")");
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

void free_resolver_scope(ResolverScope *scope) {
    if (scope == NULL) {
        return;
    }

    for (size_t i = 0; i < scope->count; i++) {
        free(scope->items[i].name);
    }

    free(scope->items);
    scope->items = NULL;
    scope->count = 0;
    scope->capacity = 0;
}

void init_resolver(Resolver *resolver) {
    resolver->items = NULL;
    resolver->count = 0;
    resolver->capacity = 0;
    resolver->had_error = 0;
    resolver->current_function = RESOLVER_FUNCTION_NONE;
    resolver->current_class = RESOLVER_CLASS_NONE;
}

void free_resolver(Resolver *resolver) {
    for (size_t i = 0; i < resolver->count; i++) {
        free_resolver_scope(&resolver->items[i]);
    }

    free(resolver->items);
    resolver->items = NULL;
    resolver->count = 0;
    resolver->capacity = 0;
    resolver->had_error = 0;
    resolver->current_function = RESOLVER_FUNCTION_NONE;
    resolver->current_class = RESOLVER_CLASS_NONE;
}

void append_resolver_scope(Resolver *resolver, ResolverScope scope) {
    if (resolver->count == resolver->capacity) {
        size_t new_capacity = resolver->capacity < 8 ? 8 : resolver->capacity * 2;
        resolver->items = xrealloc(resolver->items, new_capacity * sizeof(ResolverScope));
        resolver->capacity = new_capacity;
    }

    resolver->items[resolver->count++] = scope;
}

void resolver_begin_scope(Resolver *resolver) {
    ResolverScope scope = {
        .items = NULL,
        .count = 0,
        .capacity = 0,
    };
    append_resolver_scope(resolver, scope);
}

void resolver_end_scope(Resolver *resolver) {
    if (resolver->count == 0) {
        return;
    }

    free_resolver_scope(&resolver->items[resolver->count - 1]);
    resolver->count--;
}

int resolver_find_scope_entry(const ResolverScope *scope, const char *name, size_t length) {
    if (scope == NULL) {
        return -1;
    }

    for (size_t i = 0; i < scope->count; i++) {
        if (scope->items[i].length == length &&
            strncmp(scope->items[i].name, name, length) == 0) {
            return (int) i;
        }
    }

    return -1;
}

void resolver_declare(Resolver *resolver, Token name) {
    if (resolver->count == 0) {
        return;
    }

    ResolverScope *scope = &resolver->items[resolver->count - 1];
    if (resolver_find_scope_entry(scope, name.start, name.length) >= 0) {
        resolver_error(resolver, name, "Already a variable with this name in this scope.");
        return;
    }

    if (scope->count == scope->capacity) {
        size_t new_capacity = scope->capacity < 8 ? 8 : scope->capacity * 2;
        scope->items = xrealloc(scope->items, new_capacity * sizeof(ResolverScopeEntry));
        scope->capacity = new_capacity;
    }

    scope->items[scope->count].name = copy_string_slice(name.start, name.length);
    scope->items[scope->count].length = name.length;
    scope->items[scope->count].is_defined = 0;
    scope->count++;
}

void resolver_define(Resolver *resolver, Token name) {
    if (resolver->count == 0) {
        return;
    }

    ResolverScope *scope = &resolver->items[resolver->count - 1];
    int index = resolver_find_scope_entry(scope, name.start, name.length);
    if (index >= 0) {
        scope->items[index].is_defined = 1;
    }
}

void resolver_resolve_local_variable(Resolver *resolver, Expr *expr) {
    for (size_t i = resolver->count; i > 0; i--) {
        size_t scope_index = i - 1;
        ResolverScope *scope = &resolver->items[scope_index];
        if (resolver_find_scope_entry(scope, expr->as.variable.name.start, expr->as.variable.name.length) >= 0) {
            expr->as.variable.is_resolved = 1;
            expr->as.variable.resolved_depth = resolver->count - 1 - scope_index;
            return;
        }
    }

    expr->as.variable.is_resolved = 0;
}

void resolver_resolve_local_assignment(Resolver *resolver, Expr *expr) {
    for (size_t i = resolver->count; i > 0; i--) {
        size_t scope_index = i - 1;
        ResolverScope *scope = &resolver->items[scope_index];
        if (resolver_find_scope_entry(scope, expr->as.assign.name.start, expr->as.assign.name.length) >= 0) {
            expr->as.assign.is_resolved = 1;
            expr->as.assign.resolved_depth = resolver->count - 1 - scope_index;
            return;
        }
    }

    expr->as.assign.is_resolved = 0;
}

void resolver_resolve_function(Resolver *resolver, const Stmt *stmt, ResolverFunctionType function_type) {
    ResolverFunctionType enclosing_function = resolver->current_function;
    resolver->current_function = function_type;

    resolver_begin_scope(resolver);
    for (size_t i = 0; i < stmt->as.function.parameters.count; i++) {
        Token parameter = stmt->as.function.parameters.items[i];
        resolver_declare(resolver, parameter);
        resolver_define(resolver, parameter);
    }
    resolver_resolve_statements(resolver, &stmt->as.function.body);
    resolver_end_scope(resolver);

    resolver->current_function = enclosing_function;
}

void resolver_resolve_expr(Resolver *resolver, Expr *expr) {
    if (expr == NULL) {
        return;
    }

    switch (expr->type) {
        case EXPR_BINARY:
            resolver_resolve_expr(resolver, expr->as.binary.left);
            resolver_resolve_expr(resolver, expr->as.binary.right);
            return;
        case EXPR_GROUPING:
            resolver_resolve_expr(resolver, expr->as.grouping.expression);
            return;
        case EXPR_LITERAL:
            return;
        case EXPR_UNARY:
            resolver_resolve_expr(resolver, expr->as.unary.right);
            return;
        case EXPR_VARIABLE:
            if (expr->as.variable.name.type == TOKEN_THIS &&
                resolver->current_class == RESOLVER_CLASS_NONE) {
                resolver_error(resolver, expr->as.variable.name, "Can't use 'this' outside of a class.");
            }
            if (resolver->count > 0) {
                ResolverScope *scope = &resolver->items[resolver->count - 1];
                int index = resolver_find_scope_entry(scope, expr->as.variable.name.start, expr->as.variable.name.length);
                if (index >= 0 && !scope->items[index].is_defined) {
                    resolver_error(resolver, expr->as.variable.name, "Can't read local variable in its own initializer.");
                }
            }
            resolver_resolve_local_variable(resolver, expr);
            return;
        case EXPR_ASSIGN:
            resolver_resolve_expr(resolver, expr->as.assign.value);
            resolver_resolve_local_assignment(resolver, expr);
            return;
        case EXPR_LOGICAL:
            resolver_resolve_expr(resolver, expr->as.logical.left);
            resolver_resolve_expr(resolver, expr->as.logical.right);
            return;
        case EXPR_CALL:
            resolver_resolve_expr(resolver, expr->as.call.callee);
            for (size_t i = 0; i < expr->as.call.arguments.count; i++) {
                resolver_resolve_expr(resolver, expr->as.call.arguments.items[i]);
            }
            return;
        case EXPR_GET:
            resolver_resolve_expr(resolver, expr->as.get.object);
            return;
        case EXPR_SET:
            resolver_resolve_expr(resolver, expr->as.set.object);
            resolver_resolve_expr(resolver, expr->as.set.value);
            return;
    }
}

void resolver_resolve_statement(Resolver *resolver, const Stmt *stmt) {
    if (stmt == NULL) {
        return;
    }

    switch (stmt->type) {
        case STMT_EXPRESSION:
            resolver_resolve_expr(resolver, stmt->as.expression.expression);
            return;
        case STMT_PRINT:
            resolver_resolve_expr(resolver, stmt->as.print.expression);
            return;
        case STMT_VAR:
            resolver_declare(resolver, stmt->as.var.name);
            resolver_resolve_expr(resolver, stmt->as.var.initializer);
            resolver_define(resolver, stmt->as.var.name);
            return;
        case STMT_CLASS:
            {
                ResolverClassType enclosing_class = resolver->current_class;
                resolver->current_class = RESOLVER_CLASS_CLASS;

                if (stmt->as.class_statement.superclass != NULL &&
                    stmt->as.class_statement.superclass->type == EXPR_VARIABLE &&
                    stmt->as.class_statement.name.length == stmt->as.class_statement.superclass->as.variable.name.length &&
                    strncmp(
                        stmt->as.class_statement.name.start,
                        stmt->as.class_statement.superclass->as.variable.name.start,
                        stmt->as.class_statement.name.length) == 0) {
                    resolver_error(
                        resolver,
                        stmt->as.class_statement.superclass->as.variable.name,
                        "A class can't inherit from itself.");
                }

                resolver_declare(resolver, stmt->as.class_statement.name);
                resolver_define(resolver, stmt->as.class_statement.name);
                resolver_resolve_expr(resolver, stmt->as.class_statement.superclass);

                resolver_begin_scope(resolver);
                Token this_token = {
                    .type = TOKEN_THIS,
                    .start = "this",
                    .length = strlen("this"),
                    .line = stmt->as.class_statement.name.line,
                    .number = 0,
                };
                resolver_declare(resolver, this_token);
                resolver_define(resolver, this_token);

                for (size_t i = 0; i < stmt->as.class_statement.methods.count; i++) {
                    const Stmt *method = stmt->as.class_statement.methods.items[i];
                    ResolverFunctionType function_type =
                        is_initializer_name(method->as.function.name)
                            ? RESOLVER_FUNCTION_INITIALIZER
                            : RESOLVER_FUNCTION_METHOD;
                    resolver_resolve_function(resolver, method, function_type);
                }

                resolver_end_scope(resolver);
                resolver->current_class = enclosing_class;
            }
            return;
        case STMT_FUNCTION:
            resolver_declare(resolver, stmt->as.function.name);
            resolver_define(resolver, stmt->as.function.name);
            resolver_resolve_function(resolver, stmt, RESOLVER_FUNCTION_FUNCTION);
            return;
        case STMT_RETURN:
            if (resolver->current_function == RESOLVER_FUNCTION_NONE) {
                resolver_error(resolver, stmt->as.return_statement.keyword, "Can't return from top-level code.");
            }
            if (resolver->current_function == RESOLVER_FUNCTION_INITIALIZER &&
                stmt->as.return_statement.value != NULL) {
                resolver_error(resolver, stmt->as.return_statement.keyword, "Can't return a value from an initializer.");
            }
            resolver_resolve_expr(resolver, stmt->as.return_statement.value);
            return;
        case STMT_BLOCK:
            resolver_begin_scope(resolver);
            resolver_resolve_statements(resolver, &stmt->as.block.statements);
            resolver_end_scope(resolver);
            return;
        case STMT_IF:
            resolver_resolve_expr(resolver, stmt->as.if_statement.condition);
            resolver_resolve_statement(resolver, stmt->as.if_statement.then_branch);
            resolver_resolve_statement(resolver, stmt->as.if_statement.else_branch);
            return;
        case STMT_WHILE:
            resolver_resolve_expr(resolver, stmt->as.while_statement.condition);
            resolver_resolve_statement(resolver, stmt->as.while_statement.body);
            return;
    }
}

void resolver_resolve_statements(Resolver *resolver, const StmtArray *statements) {
    for (size_t i = 0; i < statements->count; i++) {
        resolver_resolve_statement(resolver, statements->items[i]);
    }
}

void resolver_error(Resolver *resolver, Token token, const char *message) {
    if (token.type == TOKEN_EOF) {
        fprintf(stderr, "[line %d] Error at end: %s\n", token.line, message);
    } else {
        fprintf(stderr, "[line %d] Error at '%.*s': %s\n", token.line, (int) token.length, token.start, message);
    }

    resolver->had_error = 1;
}

Value evaluate_expr(const Expr *expr, Environment *environment, int *had_runtime_error) {
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
                        runtime_error(expr->as.unary.operator_token.line, "Operand must be a number.");
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
                        runtime_error(expr->as.binary.operator_token.line, "Operands must be numbers.");
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
                        runtime_error(expr->as.binary.operator_token.line, "Operands must be numbers.");
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
                        runtime_error(expr->as.binary.operator_token.line, "Operands must be numbers.");
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
                        runtime_error(expr->as.binary.operator_token.line, "Operands must be numbers.");
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
                        runtime_error(expr->as.binary.operator_token.line, "Operands must be numbers.");
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
                        runtime_error(expr->as.binary.operator_token.line, "Operands must be two numbers or two strings.");
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
                        runtime_error(expr->as.binary.operator_token.line, "Operands must be numbers.");
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
                        runtime_error(expr->as.binary.operator_token.line, "Operands must be numbers.");
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
            if (expr->as.variable.is_resolved) {
                return get_resolved_variable(environment, expr->as.variable.resolved_depth, expr->as.variable.name, had_runtime_error);
            }
            return get_variable(global_environment(environment), expr->as.variable.name, had_runtime_error);
        case EXPR_ASSIGN: {
            Value value = evaluate_expr(expr->as.assign.value, environment, had_runtime_error);
            if (*had_runtime_error) {
                free_value(&value);
                return make_nil_value();
            }

            int did_assign;
            if (expr->as.assign.is_resolved) {
                did_assign = assign_resolved_variable(
                    environment,
                    expr->as.assign.resolved_depth,
                    expr->as.assign.name,
                    value,
                    had_runtime_error);
            } else {
                did_assign = assign_variable(global_environment(environment), expr->as.assign.name, value, had_runtime_error);
            }

            if (!did_assign) {
                free_value(&value);
                return make_nil_value();
            }

            return value;
        }
        case EXPR_LOGICAL: {
            Value left = evaluate_expr(expr->as.logical.left, environment, had_runtime_error);
            if (*had_runtime_error) {
                free_value(&left);
                return make_nil_value();
            }

            switch (expr->as.logical.operator_token.type) {
                case TOKEN_OR:
                    if (is_truthy(left)) {
                        return left;
                    }
                    free_value(&left);
                    return evaluate_expr(expr->as.logical.right, environment, had_runtime_error);
                case TOKEN_AND:
                    if (!is_truthy(left)) {
                        return left;
                    }
                    free_value(&left);
                    return evaluate_expr(expr->as.logical.right, environment, had_runtime_error);
                default:
                    free_value(&left);
                    break;
            }
            break;
        }
        case EXPR_CALL: {
            Value callee = evaluate_expr(expr->as.call.callee, environment, had_runtime_error);
            if (*had_runtime_error) {
                free_value(&callee);
                return make_nil_value();
            }

            Value *arguments = NULL;
            if (expr->as.call.arguments.count > 0) {
                arguments = xmalloc(expr->as.call.arguments.count * sizeof(Value));
            }

            for (size_t i = 0; i < expr->as.call.arguments.count; i++) {
                arguments[i] = evaluate_expr(expr->as.call.arguments.items[i], environment, had_runtime_error);
                if (*had_runtime_error) {
                    for (size_t j = 0; j <= i; j++) {
                        free_value(&arguments[j]);
                    }
                    free(arguments);
                    free_value(&callee);
                    return make_nil_value();
                }
            }

            if (callee.type != VALUE_CLASS &&
                callee.type != VALUE_NATIVE_FUNCTION &&
                callee.type != VALUE_FUNCTION) {
                runtime_error(expr->as.call.paren.line, "Can only call functions and classes.");
                *had_runtime_error = 1;
                for (size_t i = 0; i < expr->as.call.arguments.count; i++) {
                    free_value(&arguments[i]);
                }
                free(arguments);
                free_value(&callee);
                return make_nil_value();
            }

            Value result;
            if (callee.type == VALUE_CLASS) {
                result = make_instance_value(new_instance_object(callee.as.class_object));

                const Value *initializer_method = find_inherited_class_method(callee.as.class_object, "init", strlen("init"));
                if (initializer_method != NULL) {
                    Value initializer = bind_method(*initializer_method, result.as.instance_object);
                    Value initializer_result = make_nil_value();
                    int call_succeeded = call_function_value(
                        initializer,
                        expr->as.call.arguments.count,
                        arguments,
                        expr->as.call.paren.line,
                        had_runtime_error,
                        &initializer_result);
                    free_value(&initializer);
                    free_value(&initializer_result);
                    if (!call_succeeded) {
                        for (size_t i = 0; i < expr->as.call.arguments.count; i++) {
                            free_value(&arguments[i]);
                        }
                        free(arguments);
                        free_value(&callee);
                        free_value(&result);
                        return make_nil_value();
                    }
                } else if (expr->as.call.arguments.count != 0) {
                    runtime_error(
                        expr->as.call.paren.line,
                        "Expected 0 arguments but got %zu.",
                        expr->as.call.arguments.count);
                    *had_runtime_error = 1;
                    for (size_t i = 0; i < expr->as.call.arguments.count; i++) {
                        free_value(&arguments[i]);
                    }
                    free(arguments);
                    free_value(&callee);
                    free_value(&result);
                    return make_nil_value();
                }
            } else if (callee.type == VALUE_NATIVE_FUNCTION) {
                const NativeFunction *native_function = callee.as.native_function;
                if ((size_t) native_function->arity != expr->as.call.arguments.count) {
                    runtime_error(
                        expr->as.call.paren.line,
                        "Expected %d arguments but got %zu.",
                        native_function->arity,
                        expr->as.call.arguments.count);
                    *had_runtime_error = 1;
                    for (size_t i = 0; i < expr->as.call.arguments.count; i++) {
                        free_value(&arguments[i]);
                    }
                    free(arguments);
                    free_value(&callee);
                    return make_nil_value();
                }

                result = native_function->function((int) expr->as.call.arguments.count, arguments);
            } else {
                if (!call_function_value(
                        callee,
                        expr->as.call.arguments.count,
                        arguments,
                        expr->as.call.paren.line,
                        had_runtime_error,
                        &result)) {
                    *had_runtime_error = 1;
                    for (size_t i = 0; i < expr->as.call.arguments.count; i++) {
                        free_value(&arguments[i]);
                    }
                    free(arguments);
                    free_value(&callee);
                    return make_nil_value();
                }
            }
            for (size_t i = 0; i < expr->as.call.arguments.count; i++) {
                free_value(&arguments[i]);
            }
            free(arguments);
            free_value(&callee);
            return result;
        }
        case EXPR_GET: {
            Value object = evaluate_expr(expr->as.get.object, environment, had_runtime_error);
            if (*had_runtime_error) {
                free_value(&object);
                return make_nil_value();
            }

            if (object.type != VALUE_INSTANCE) {
                runtime_error(expr->as.get.name.line, "Only instances have properties.");
                *had_runtime_error = 1;
                free_value(&object);
                return make_nil_value();
            }

            Value result = get_instance_field(object.as.instance_object, expr->as.get.name, had_runtime_error);
            free_value(&object);
            return result;
        }
        case EXPR_SET: {
            Value object = evaluate_expr(expr->as.set.object, environment, had_runtime_error);
            if (*had_runtime_error) {
                free_value(&object);
                return make_nil_value();
            }

            if (object.type != VALUE_INSTANCE) {
                runtime_error(expr->as.set.name.line, "Only instances have fields.");
                *had_runtime_error = 1;
                free_value(&object);
                return make_nil_value();
            }

            Value value = evaluate_expr(expr->as.set.value, environment, had_runtime_error);
            if (*had_runtime_error) {
                free_value(&object);
                free_value(&value);
                return make_nil_value();
            }

            set_instance_field(object.as.instance_object, expr->as.set.name, value);
            free_value(&object);
            return value;
        }
    }

    runtime_error(0, "Unsupported expression.");
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

ClassObject *new_class_object(const char *name, size_t length, ClassObject *superclass) {
    ClassObject *class_object = xmalloc(sizeof(ClassObject));
    class_object->name = copy_string_slice(name, length);
    class_object->superclass = retain_class_object(superclass);
    class_object->methods = NULL;
    class_object->method_count = 0;
    class_object->method_capacity = 0;
    class_object->ref_count = 1;
    return class_object;
}

ClassObject *retain_class_object(ClassObject *class_object) {
    if (class_object != NULL) {
        class_object->ref_count++;
    }

    return class_object;
}

void release_class_object(ClassObject *class_object) {
    if (class_object == NULL) {
        return;
    }

    if (class_object->ref_count > 1) {
        class_object->ref_count--;
        return;
    }

    for (size_t i = 0; i < class_object->method_count; i++) {
        free(class_object->methods[i].name);
        free_value(&class_object->methods[i].value);
    }

    free(class_object->methods);
    free(class_object->name);
    release_class_object(class_object->superclass);
    free(class_object);
}

Value make_class_value(ClassObject *class_object) {
    Value value = {
        .type = VALUE_CLASS,
        .as.class_object = class_object,
    };
    return value;
}

InstanceObject *new_instance_object(ClassObject *class_object) {
    InstanceObject *instance_object = xmalloc(sizeof(InstanceObject));
    instance_object->class_object = retain_class_object(class_object);
    instance_object->fields = NULL;
    instance_object->field_count = 0;
    instance_object->field_capacity = 0;
    instance_object->ref_count = 1;
    return instance_object;
}

InstanceObject *retain_instance_object(InstanceObject *instance_object) {
    if (instance_object != NULL) {
        instance_object->ref_count++;
    }

    return instance_object;
}

void release_instance_object(InstanceObject *instance_object) {
    if (instance_object == NULL) {
        return;
    }

    if (instance_object->ref_count > 1) {
        instance_object->ref_count--;
        return;
    }

    for (size_t i = 0; i < instance_object->field_count; i++) {
        free(instance_object->fields[i].name);
        free_value(&instance_object->fields[i].value);
    }

    free(instance_object->fields);
    release_class_object(instance_object->class_object);
    free(instance_object);
}

Value make_instance_value(InstanceObject *instance_object) {
    Value value = {
        .type = VALUE_INSTANCE,
        .as.instance_object = instance_object,
    };
    return value;
}

Value make_native_function_value(const NativeFunction *native_function) {
    Value value = {
        .type = VALUE_NATIVE_FUNCTION,
        .as.native_function = native_function,
    };
    return value;
}

Value make_function_value(const Stmt *declaration, Environment *closure, int is_initializer) {
    Value value = {
        .type = VALUE_FUNCTION,
        .as.function.declaration = declaration,
        .as.function.closure = retain_environment(closure),
        .as.function.is_initializer = is_initializer,
    };
    return value;
}

Value bind_method(Value method, InstanceObject *instance_object) {
    Environment *bound_environment = new_enclosed_environment(method.as.function.closure);
    Token this_token = {
        .type = TOKEN_THIS,
        .start = "this",
        .length = strlen("this"),
        .line = 0,
        .number = 0,
    };
    Value instance_value = make_instance_value(retain_instance_object(instance_object));
    define_variable(bound_environment, this_token, instance_value);
    free_value(&instance_value);

    Value bound_method = make_function_value(
        method.as.function.declaration,
        bound_environment,
        method.as.function.is_initializer);
    release_environment(bound_environment);
    return bound_method;
}

int is_initializer_name(Token name) {
    return name.length == strlen("init") && strncmp(name.start, "init", strlen("init")) == 0;
}

Value get_bound_this_value(Value function, int *had_runtime_error) {
    if (function.type != VALUE_FUNCTION || function.as.function.closure == NULL) {
        *had_runtime_error = 1;
        runtime_error(0, "Missing bound receiver.");
        return make_nil_value();
    }

    int index = find_environment_entry(function.as.function.closure, "this", strlen("this"));
    if (index >= 0) {
        return clone_value(function.as.function.closure->items[index].value);
    }

    *had_runtime_error = 1;
    runtime_error(0, "Missing bound receiver.");
    return make_nil_value();
}

int call_function_value(
    Value callee,
    size_t argument_count,
    const Value *arguments,
    int line,
    int *had_runtime_error,
    Value *result_out) {
    const Stmt *declaration = callee.as.function.declaration;
    if (declaration->as.function.parameters.count != argument_count) {
        runtime_error(
            line,
            "Expected %zu arguments but got %zu.",
            declaration->as.function.parameters.count,
            argument_count);
        *had_runtime_error = 1;
        return 0;
    }

    Environment *function_environment = new_enclosed_environment(callee.as.function.closure);
    for (size_t i = 0; i < declaration->as.function.parameters.count; i++) {
        define_variable(function_environment, declaration->as.function.parameters.items[i], arguments[i]);
    }

    int did_return = 0;
    Value return_value = make_nil_value();
    int exit_code = interpret_statements(
        &declaration->as.function.body,
        function_environment,
        &did_return,
        &return_value);
    release_environment(function_environment);
    if (exit_code != 0) {
        *had_runtime_error = 1;
        free_value(&return_value);
        return 0;
    }

    if (callee.as.function.is_initializer) {
        free_value(&return_value);
        *result_out = get_bound_this_value(callee, had_runtime_error);
        return !(*had_runtime_error);
    }

    if (did_return) {
        *result_out = return_value;
    } else {
        *result_out = make_nil_value();
        free_value(&return_value);
    }

    return 1;
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
        case VALUE_CLASS:
            return make_class_value(retain_class_object(value.as.class_object));
        case VALUE_INSTANCE:
            return make_instance_value(retain_instance_object(value.as.instance_object));
        case VALUE_NATIVE_FUNCTION:
            return make_native_function_value(value.as.native_function);
        case VALUE_FUNCTION:
            return make_function_value(
                value.as.function.declaration,
                value.as.function.closure,
                value.as.function.is_initializer);
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

    if (value->type == VALUE_CLASS) {
        release_class_object(value->as.class_object);
    }

    if (value->type == VALUE_INSTANCE) {
        release_instance_object(value->as.instance_object);
    }

    if (value->type == VALUE_FUNCTION) {
        release_environment(value->as.function.closure);
    }

    *value = make_nil_value();
}

Value native_clock(int argument_count, const Value *arguments) {
    (void) argument_count;
    (void) arguments;
    return make_number_value((double) time(NULL));
}

void runtime_error(int line, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fprintf(stderr, "\n[line %d]\n", line);
}

int find_instance_field(const InstanceObject *instance_object, const char *name, size_t length) {
    if (instance_object == NULL) {
        return -1;
    }

    for (size_t i = 0; i < instance_object->field_count; i++) {
        if (instance_object->fields[i].length == length &&
            strncmp(instance_object->fields[i].name, name, length) == 0) {
            return (int) i;
        }
    }

    return -1;
}

void set_instance_field(InstanceObject *instance_object, Token name, Value value) {
    int existing_index = find_instance_field(instance_object, name.start, name.length);
    Value stored_value = clone_value(value);

    if (existing_index >= 0) {
        free_value(&instance_object->fields[existing_index].value);
        instance_object->fields[existing_index].value = stored_value;
        return;
    }

    if (instance_object->field_count == instance_object->field_capacity) {
        size_t new_capacity = instance_object->field_capacity < 8 ? 8 : instance_object->field_capacity * 2;
        instance_object->fields = xrealloc(instance_object->fields, new_capacity * sizeof(EnvironmentEntry));
        instance_object->field_capacity = new_capacity;
    }

    instance_object->fields[instance_object->field_count].name = copy_string_slice(name.start, name.length);
    instance_object->fields[instance_object->field_count].length = name.length;
    instance_object->fields[instance_object->field_count].value = stored_value;
    instance_object->field_count++;
}

Value get_instance_field(const InstanceObject *instance_object, Token name, int *had_runtime_error) {
    int index = find_instance_field(instance_object, name.start, name.length);
    if (index >= 0) {
        return clone_value(instance_object->fields[index].value);
    }

    const Value *method = find_inherited_class_method(instance_object->class_object, name.start, name.length);
    if (method != NULL) {
        return bind_method(*method, (InstanceObject *) instance_object);
    }

    runtime_error(name.line, "Undefined property '%.*s'.", (int) name.length, name.start);
    *had_runtime_error = 1;
    return make_nil_value();
}

int find_class_method(const ClassObject *class_object, const char *name, size_t length) {
    if (class_object == NULL) {
        return -1;
    }

    for (size_t i = 0; i < class_object->method_count; i++) {
        if (class_object->methods[i].length == length &&
            strncmp(class_object->methods[i].name, name, length) == 0) {
            return (int) i;
        }
    }

    return -1;
}

const Value *find_inherited_class_method(const ClassObject *class_object, const char *name, size_t length) {
    for (const ClassObject *current = class_object; current != NULL; current = current->superclass) {
        int index = find_class_method(current, name, length);
        if (index >= 0) {
            return &current->methods[index].value;
        }
    }

    return NULL;
}

void set_class_method(ClassObject *class_object, Token name, Value value) {
    int existing_index = find_class_method(class_object, name.start, name.length);
    Value stored_value = clone_value(value);

    if (existing_index >= 0) {
        free_value(&class_object->methods[existing_index].value);
        class_object->methods[existing_index].value = stored_value;
        return;
    }

    if (class_object->method_count == class_object->method_capacity) {
        size_t new_capacity = class_object->method_capacity < 8 ? 8 : class_object->method_capacity * 2;
        class_object->methods = xrealloc(class_object->methods, new_capacity * sizeof(ClassMethodEntry));
        class_object->method_capacity = new_capacity;
    }

    class_object->methods[class_object->method_count].name = copy_string_slice(name.start, name.length);
    class_object->methods[class_object->method_count].length = name.length;
    class_object->methods[class_object->method_count].value = stored_value;
    class_object->method_count++;
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

Value get_resolved_variable(const Environment *environment, size_t distance, Token name, int *had_runtime_error) {
    Environment *target = ancestor_environment((Environment *) environment, distance);
    if (target != NULL) {
        int index = find_environment_entry(target, name.start, name.length);
        if (index >= 0) {
            return clone_value(target->items[index].value);
        }
    }

    runtime_error(name.line, "Undefined variable '%.*s'.", (int) name.length, name.start);
    *had_runtime_error = 1;
    return make_nil_value();
}

Value get_variable(const Environment *environment, Token name, int *had_runtime_error) {
    for (const Environment *current = environment; current != NULL; current = current->enclosing) {
        int index = find_environment_entry(current, name.start, name.length);
        if (index >= 0) {
            return clone_value(current->items[index].value);
        }
    }

    runtime_error(name.line, "Undefined variable '%.*s'.", (int) name.length, name.start);
    *had_runtime_error = 1;
    return make_nil_value();
}

int assign_resolved_variable(Environment *environment, size_t distance, Token name, Value value, int *had_runtime_error) {
    Environment *target = ancestor_environment(environment, distance);
    if (target != NULL) {
        int index = find_environment_entry(target, name.start, name.length);
        if (index >= 0) {
            free_value(&target->items[index].value);
            target->items[index].value = clone_value(value);
            return 1;
        }
    }

    runtime_error(name.line, "Undefined variable '%.*s'.", (int) name.length, name.start);
    *had_runtime_error = 1;
    return 0;
}

int assign_variable(Environment *environment, Token name, Value value, int *had_runtime_error) {
    for (Environment *current = environment; current != NULL; current = current->enclosing) {
        int index = find_environment_entry(current, name.start, name.length);
        if (index >= 0) {
            free_value(&current->items[index].value);
            current->items[index].value = clone_value(value);
            return 1;
        }
    }

    runtime_error(name.line, "Undefined variable '%.*s'.", (int) name.length, name.start);
    *had_runtime_error = 1;
    return 0;
}

int is_truthy(Value value) {
    switch (value.type) {
        case VALUE_NIL:
            return 0;
        case VALUE_BOOL:
            return value.as.boolean;
        case VALUE_NUMBER:
        case VALUE_STRING:
        case VALUE_CLASS:
        case VALUE_INSTANCE:
        case VALUE_NATIVE_FUNCTION:
        case VALUE_FUNCTION:
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
        case VALUE_CLASS:
            return left.as.class_object == right.as.class_object;
        case VALUE_INSTANCE:
            return left.as.instance_object == right.as.instance_object;
        case VALUE_NATIVE_FUNCTION:
            return left.as.native_function == right.as.native_function;
        case VALUE_FUNCTION:
            return left.as.function.declaration == right.as.function.declaration &&
                   left.as.function.closure == right.as.function.closure &&
                   left.as.function.is_initializer == right.as.function.is_initializer;
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
        case VALUE_CLASS:
            printf("%s", value.as.class_object->name);
            return;
        case VALUE_INSTANCE:
            printf("%s instance", value.as.instance_object->class_object->name);
            return;
        case VALUE_NATIVE_FUNCTION:
            printf("<native fn>");
            return;
        case VALUE_FUNCTION:
            printf(
                "<fn %.*s>",
                (int) value.as.function.declaration->as.function.name.length,
                value.as.function.declaration->as.function.name.start);
            return;
    }
}

int interpret_statement(const Stmt *stmt, Environment *environment, int *did_return, Value *return_value) {
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
        case STMT_CLASS:
            {
                ClassObject *superclass = NULL;
                Value superclass_value = make_nil_value();
                if (stmt->as.class_statement.superclass != NULL) {
                    superclass_value = evaluate_expr(stmt->as.class_statement.superclass, environment, &had_runtime_error);
                    if (had_runtime_error) {
                        free_value(&superclass_value);
                        return 70;
                    }

                    if (superclass_value.type != VALUE_CLASS) {
                        runtime_error(
                            stmt->as.class_statement.superclass->as.variable.name.line,
                            "Superclass must be a class.");
                        free_value(&superclass_value);
                        return 70;
                    }

                    superclass = superclass_value.as.class_object;
                }

                value = make_class_value(new_class_object(
                    stmt->as.class_statement.name.start,
                    stmt->as.class_statement.name.length,
                    superclass));
                free_value(&superclass_value);
            }
            for (size_t i = 0; i < stmt->as.class_statement.methods.count; i++) {
                const Stmt *method = stmt->as.class_statement.methods.items[i];
                Value method_value = make_function_value(method, environment, is_initializer_name(method->as.function.name));
                set_class_method(value.as.class_object, method->as.function.name, method_value);
                free_value(&method_value);
            }
            define_variable(environment, stmt->as.class_statement.name, value);
            free_value(&value);
            return 0;
        case STMT_FUNCTION:
            value = make_function_value(stmt, environment, 0);
            define_variable(environment, stmt->as.function.name, value);
            free_value(&value);
            return 0;
        case STMT_RETURN:
            if (stmt->as.return_statement.value == NULL) {
                value = make_nil_value();
            } else {
                value = evaluate_expr(stmt->as.return_statement.value, environment, &had_runtime_error);
                if (had_runtime_error) {
                    free_value(&value);
                    return 70;
                }
            }

            free_value(return_value);
            *return_value = value;
            *did_return = 1;
            return 0;
        case STMT_BLOCK: {
            Environment *block_environment = new_enclosed_environment(environment);
            int exit_code = interpret_statements(&stmt->as.block.statements, block_environment, did_return, return_value);
            release_environment(block_environment);
            return exit_code;
        }
        case STMT_IF:
            value = evaluate_expr(stmt->as.if_statement.condition, environment, &had_runtime_error);
            if (had_runtime_error) {
                free_value(&value);
                return 70;
            }

            if (is_truthy(value)) {
                free_value(&value);
                return interpret_statement(stmt->as.if_statement.then_branch, environment, did_return, return_value);
            }

            free_value(&value);
            if (stmt->as.if_statement.else_branch != NULL) {
                return interpret_statement(stmt->as.if_statement.else_branch, environment, did_return, return_value);
            }
            return 0;
        case STMT_WHILE:
            while (1) {
                value = evaluate_expr(stmt->as.while_statement.condition, environment, &had_runtime_error);
                if (had_runtime_error) {
                    free_value(&value);
                    return 70;
                }

                int condition_is_truthy = is_truthy(value);
                free_value(&value);
                if (!condition_is_truthy) {
                    return 0;
                }

                int exit_code = interpret_statement(stmt->as.while_statement.body, environment, did_return, return_value);
                if (exit_code != 0) {
                    return exit_code;
                }

                if (*did_return) {
                    return 0;
                }
            }
    }

    return 70;
}

int interpret_statements(const StmtArray *statements, Environment *environment, int *did_return, Value *return_value) {
    for (size_t i = 0; i < statements->count; i++) {
        int exit_code = interpret_statement(statements->items[i], environment, did_return, return_value);
        if (exit_code != 0) {
            return exit_code;
        }

        if (*did_return) {
            return 0;
        }
    }

    return 0;
}

int run_run_command(const char *filename) {
    char *source = NULL;
    TokenArray tokens = {0};
    StmtArray statements = {0};
    Environment *environment = new_environment();
    define_native_functions(environment);
    int exit_code = parse_file_to_statements(filename, &source, &tokens, &statements);
    if (exit_code != 0) {
        release_environment(environment);
        return exit_code;
    }

    Resolver resolver;
    init_resolver(&resolver);
    resolver_resolve_statements(&resolver, &statements);
    if (resolver.had_error) {
        free_resolver(&resolver);
        free_stmt_array(&statements);
        free_token_array(&tokens);
        free(source);
        release_environment(environment);
        return 65;
    }
    free_resolver(&resolver);

    int did_return = 0;
    Value return_value = make_nil_value();
    exit_code = interpret_statements(&statements, environment, &did_return, &return_value);
    free_value(&return_value);

    free_stmt_array(&statements);
    free_token_array(&tokens);
    free(source);
    release_environment(environment);
    return exit_code;
}
