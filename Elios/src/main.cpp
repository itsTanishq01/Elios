#include <iostream>
#include <fstream>
#include <optional>
#include <sstream>
#include <vector>
#include <cctype> // for std::isdigit and std::isalpha

enum class TokenType {
    _return,
    int_lit,
    semi
};

struct Token {
    TokenType type;
    std::optional<std::string> value;
};

std::vector<Token> tokenize(const std::string& str) {
    std::vector<Token> tokens;
    std::string buffer;
    size_t i = 0;

    while (i < str.size()) {
        char c = str[i];
        if (std::isalpha(c)) {
            buffer.clear();
            while (i < str.size() && std::isalpha(str[i])) {
                buffer.push_back(str[i++]);
            }

            if (buffer == "return") {
                tokens.push_back({.type = TokenType::_return});
            } else {
                std::cerr << "Unrecognized token: " << buffer << std::endl;
                exit(EXIT_FAILURE);
            }
        } else if (std::isdigit(c)) {
            buffer.clear();
            while (i < str.size() && std::isdigit(str[i])) {
                buffer.push_back(str[i++]);
            }
            tokens.push_back({.type = TokenType::int_lit, .value = buffer});
        } else if (c == ';') {
            tokens.push_back({.type = TokenType::semi});
            i++;
        } else if (std::isspace(c)) {
            i++;
        } else {
            std::cerr << "Unrecognized character: " << c << std::endl;
            exit(EXIT_FAILURE);
        }
    }

    return tokens;
}

std::string tokens_to_asm(const std::vector<Token>& tokens) {
    std::stringstream output;
    output << "global _start\nstart:\n";

    for (size_t i = 0; i < tokens.size(); ++i) {
        const Token& token = tokens[i];
        if (token.type == TokenType::_return) {
            if (i + 1 < tokens.size() && tokens[i + 1].type == TokenType::int_lit) {
                if (i + 2 < tokens.size() && tokens[i + 2].type == TokenType::semi) {
                    output << "    mov rax, 60\n";
                    output << "    mov rdi, " << tokens[i + 1].value.value() << "\n";
                    output << "    syscall\n";
                } else {
                    std::cerr << "Expected ';' after integer literal" << std::endl;
                    exit(EXIT_FAILURE);
                }
            } else {
                std::cerr << "Expected integer literal after 'return'" << std::endl;
                exit(EXIT_FAILURE);
            }
        }
    }

    return output.str();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Incorrect Usage. Correct Usage is..." << std::endl;
        std::cerr << "Usage: Elios <input.el>" << std::endl;
        return EXIT_FAILURE;
    }

    std::string contents;
    {
        std::ifstream input(argv[1]);
        if (!input) {
            std::cerr << "Failed to open file: " << argv[1] << std::endl;
            return EXIT_FAILURE;
        }
        std::stringstream contents_stream;
        contents_stream << input.rdbuf();
        contents = contents_stream.str();
    }

    std::vector<Token> tokens = tokenize(contents); {
        std::fstream file("out.asm", std::ios::out);
        file << tokens_to_asm(tokens);
    }
    system("nasm -felf64 out.asm");
    system("ld -o out.o");

    return EXIT_SUCCESS;
}
