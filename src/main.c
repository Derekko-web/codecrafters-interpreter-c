#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_file_contents(const char *filename);
int scan_tokens(const char *source);
void print_token(const char *type, const char *lexeme);
void print_token_slices(const char *type, const char *lexeme, size_t lexeme_length, const char *literal, size_t literal_length);
int match_next(const char *source, size_t *index, char expected);
int scan_string(const char *source, size_t *index, int *line);
int scan_number(const char *source, size_t *index);
int is_digit(char c);
void format_number_literal(double value, char *buffer, size_t buffer_size);

int main(int argc, char *argv[]) {
    // Disable output buffering
    setbuf(stdout, NULL);
    setbuf(stderr, NULL);

    if (argc < 3) {
        fprintf(stderr, "Usage: ./your_program tokenize <filename>\n");
        return 1;
    }

    const char *command = argv[1];

    if (strcmp(command, "tokenize") == 0) {
        char *file_contents = read_file_contents(argv[2]);
        if (file_contents == NULL) {
            return 1;
        }

        int exit_code = scan_tokens(file_contents);
        free(file_contents);
        return exit_code;
    } else {
        fprintf(stderr, "Unknown command: %s\n", command);
        return 1;
    }

    return 0;
}

char *read_file_contents(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        fprintf(stderr, "Error reading file: %s\n", filename);
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long file_size_long = ftell(file);
    rewind(file);

    if (file_size_long < 0) {
        fprintf(stderr, "Error determining file size\n");
        fclose(file);
        return NULL;
    }

    size_t file_size = (size_t)file_size_long;

    char *file_contents = malloc(file_size + 1);
    if (file_contents == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        fclose(file);
        return NULL;
    }

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

int scan_tokens(const char *source) {
    int had_error = 0;
    int line = 1;

    for (size_t i = 0; source[i] != '\0'; i++) {
        switch (source[i]) {
            case '(':
                print_token("LEFT_PAREN", "(");
                break;
            case ')':
                print_token("RIGHT_PAREN", ")");
                break;
            case '{':
                print_token("LEFT_BRACE", "{");
                break;
            case '}':
                print_token("RIGHT_BRACE", "}");
                break;
            case ',':
                print_token("COMMA", ",");
                break;
            case '.':
                print_token("DOT", ".");
                break;
            case '-':
                print_token("MINUS", "-");
                break;
            case '+':
                print_token("PLUS", "+");
                break;
            case ';':
                print_token("SEMICOLON", ";");
                break;
            case '*':
                print_token("STAR", "*");
                break;
            case '=':
                if (match_next(source, &i, '=')) {
                    print_token("EQUAL_EQUAL", "==");
                } else {
                    print_token("EQUAL", "=");
                }
                break;
            case '!':
                if (match_next(source, &i, '=')) {
                    print_token("BANG_EQUAL", "!=");
                } else {
                    print_token("BANG", "!");
                }
                break;
            case '<':
                if (match_next(source, &i, '=')) {
                    print_token("LESS_EQUAL", "<=");
                } else {
                    print_token("LESS", "<");
                }
                break;
            case '>':
                if (match_next(source, &i, '=')) {
                    print_token("GREATER_EQUAL", ">=");
                } else {
                    print_token("GREATER", ">");
                }
                break;
            case '/':
                if (match_next(source, &i, '/')) {
                    while (source[i + 1] != '\0' && source[i + 1] != '\n') {
                        i++;
                    }
                } else {
                    print_token("SLASH", "/");
                }
                break;
            case '"':
                if (scan_string(source, &i, &line)) {
                    had_error = 1;
                }
                break;
            case ' ':
            case '\r':
            case '\t':
                break;
            case '\n':
                line++;
                break;
            default:
                if (is_digit(source[i])) {
                    scan_number(source, &i);
                } else {
                    fprintf(stderr, "[line %d] Error: Unexpected character: %c\n", line, source[i]);
                    had_error = 1;
                }
                break;
        }
    }

    print_token("EOF", "");
    return had_error ? 65 : 0;
}

void print_token(const char *type, const char *lexeme) {
    printf("%s %s null\n", type, lexeme);
}

void print_token_slices(const char *type, const char *lexeme, size_t lexeme_length, const char *literal, size_t literal_length) {
    printf("%s %.*s %.*s\n", type, (int)lexeme_length, lexeme, (int)literal_length, literal);
}

int match_next(const char *source, size_t *index, char expected) {
    if (source[*index + 1] != expected) {
        return 0;
    }

    (*index)++;
    return 1;
}

int scan_string(const char *source, size_t *index, int *line) {
    size_t start = *index;
    size_t current = start + 1;

    while (source[current] != '"' && source[current] != '\0') {
        if (source[current] == '\n') {
            (*line)++;
        }

        current++;
    }

    if (source[current] == '\0') {
        fprintf(stderr, "[line %d] Error: Unterminated string.\n", *line);
        *index = current - 1;
        return 1;
    }

    print_token_slices("STRING", source + start, current - start + 1, source + start + 1, current - start - 1);
    *index = current;
    return 0;
}

int scan_number(const char *source, size_t *index) {
    size_t start = *index;
    size_t current = start;

    while (is_digit(source[current + 1])) {
        current++;
    }

    if (source[current + 1] == '.' && is_digit(source[current + 2])) {
        current++;

        while (is_digit(source[current + 1])) {
            current++;
        }
    }

    double value = strtod(source + start, NULL);
    char literal[64];
    format_number_literal(value, literal, sizeof(literal));

    print_token_slices("NUMBER", source + start, current - start + 1, literal, strlen(literal));
    *index = current;
    return 0;
}

int is_digit(char c) {
    return c >= '0' && c <= '9';
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
