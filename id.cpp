#include <algorithm>
#include <cctype>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

enum Group { STR_CONST = 20, UNKNOWN_GROUP };

int last_rand = -1;
int get_group(char c)
{
    if (std::isalpha(c) || c == '_') {
        return 0;
    }
    if (std::ispunct(c)) {
        int cur_rand;
        while ((cur_rand = rand() % 10000 + 10000) == last_rand) {
        }
        last_rand = cur_rand;
        return cur_rand;
    }
    if (std::isdigit(c)) {
        return 2;
    }
    if (std::isspace(c)) {
        return 3;
    }
    return UNKNOWN_GROUP;
}

struct Token {
    std::string s;
    int group;
};

int main(int argc, char *argv[])
{
    if (argc < 2) {
        std::cout << "usage: id <files>...";
        return EXIT_FAILURE;
    }

    argv++;
    std::ifstream ifs(*argv++);
    std::ostringstream oss;

    oss << ifs.rdbuf();
    std::string buf = oss.str();

    std::set<std::string> const types{"void", "char", "int"};
    std::vector<std::string> const keywords{
        "for",  "struct",   "if",    "while", "do",   "return",
        "else", "continue", "break", "true",  "false"};

    std::vector<Token> tokens;
    for (char c : buf) {
        int group = get_group(c);
        if (tokens.empty() || group != tokens.back().group) {
            tokens.push_back(Token{"", group});
        }
        tokens.back().s += c;
    }

    auto end = std::remove_if(tokens.begin(), tokens.end(), [](Token const &t) {
        return t.group == 3; // space
    });
    tokens.erase(end, tokens.end());

    // str constant fix
    for (auto it = tokens.begin(); it != tokens.end(); ++it) {
        if (it->s == "\"") {
            auto jt = std::find_if(std::next(it), tokens.end(),
                                   [](Token const &t) { return t.s == "\""; });
            if (jt == tokens.end()) {
                std::cout << "Unmatched brackets\n";
                std::exit(EXIT_FAILURE);
            }
            for (auto kt = std::next(it); kt != jt; ++kt) {
                it->s += kt->s;
            }
            it->s += jt->s;
            tokens.erase(std::next(it), std::next(jt));
            it->group = STR_CONST;
        }
    }

    std::set<std::string> ids;

    int i = 0;
    bool is_id = 0;
    for (auto const &token : tokens) {
        if (is_id || ids.find(token.s) != ids.end()) {
            is_id = false;
            std::cout << "Identifier: ";
            ids.insert(token.s);
        }
        else if (types.find(token.s) != types.end()) {
            is_id = true;
            std::cout << "Type: ";
        }
        else if (token.group == 2) {
            std::cout << "Constant: ";
        }
        else if (token.group == STR_CONST) {
            std::cout << "String Constant: ";
        }

        std::cout << token.s << '\n';
        ++i;
    }
}
