#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *read_file_contents(const char *filename);
int scan_tokens(const char *source);
void print_token(const char *type, const char *lexeme);

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
            case ' ':
            case '\r':
            case '\t':
                break;
            case '\n':
                line++;
                break;
            default:
                fprintf(stderr, "[line %d] Error: Unexpected character: %c\n", line, source[i]);
                had_error = 1;
                break;
        }
    }

    print_token("EOF", "");
    return had_error ? 65 : 0;
}

void print_token(const char *type, const char *lexeme) {
    printf("%s %s null\n", type, lexeme);
}
