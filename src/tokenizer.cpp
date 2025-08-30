#include "tokenizer.h"
#include <string>
#include <variant>
#include <unordered_map>


const static std::unordered_map<std::string, enum TokenType> KEYWORDS = {
    {"if", TOKEN_KWIF},
    {"else", TOKEN_KWELSE},
    {"elsif", TOKEN_KWELSIF},
    {"True", TOKEN_KWTRUE},
    {"False", TOKEN_KWFALSE},
    {"return", TOKEN_KWRETURN},
    {"while", TOKEN_KWWHILE},
    {"fn", TOKEN_KWFN},
    {"for", TOKEN_KWFOR},
    {"in", TOKEN_KWIN},
    {"None", TOKEN_KWIN},
    {"and", TOKEN_KWAND},
    {"or", TOKEN_KWOR}
};

Tokenizer::Tokenizer() : paren_count{0, 0, 0}, last_was_eol{false}, indent_level{0} {}

Token Tokenizer::get_token(const std::string& in, uint64_t& ix, uint64_t& start,
                            uint64_t& end, ErrList& errors) noexcept {
    bool in_paren = paren_count[0] + paren_count[1] + paren_count[2] > 0;

    start = ix;
    end = ix;
    if (last_was_eol) {
        assert(!in_paren);
        uint64_t indents;
        do {
            indents = parser_read_indent(in, ix);
        } while (parser_skip_spaces(in, ix, false));

        if (indents > indent_level) {
            errors.emplace_back("Bad indent", start);
            indents = indent_level;
        }
        if (indents < indent_level) {
            ix = start;
            --indent_level;
            Token t = {TOKEN_BLOCKEND};
            t.blockend = 0;
            return t;
        }
        last_was_eol = false;
    }

    if (parser_skip_spaces(in, ix, in_paren)) {
        last_was_eol = true;
        start = ix - 1;
        end = ix;
        Token t = {TOKEN_SEPARATOR};
        t.separator = '\n';
        return t;
    }

    start = ix;
    end = ix;
    if (ix >= in.size()) {
        if (indent_level > 0) {
            --indent_level;
            Token t = {TOKEN_BLOCKEND};
            t.blockend = 0;
            return t;
        }
        Token t = {TOKEN_END};
        return t;
    }

    char c = in[ix];
    if (is_identifier_start(c)) {
        std::string s = parser_read_identifier(in, ix, errors);
        auto kw = KEYWORDS.find(s);
        if (kw != KEYWORDS.end()) {
            return { kw->second };
        }
        end = ix;
        Token t = {TOKEN_IDENTIFIER};
        // TODO: free this memory
        char* str = new char[s.size()];
        t.identifier.str = str;
        t.identifier.size = s.size();
        std::memcpy(str, s.c_str(), s.size());
        return t;
    } else if (c >= '0' && c <= '9') {
        std::variant<uint64_t, double> n = parser_read_number(in, ix, errors);
        end = ix;
        if (std::holds_alternative<uint64_t>(n)) {
            uint64_t u = std::get<uint64_t>(n);
            Token t = {TOKEN_INTEGER};
            t.integer = u;
            return t;
        } else {
            double d = std::get<double>(n);
            Token t = {TOKEN_REAL};
            t.real = d;
            return t;
        }
    } else if (c == '"' || c == '\'') {
        std::string s = parser_read_string(in, ix, c, errors);
        end = ix;
        Token t = {TOKEN_STRING};
        char* str = new char[s.size()];
        t.string.str = str;
        t.string.size = s.size();
        std::memcpy(str, s.c_str(), s.size());
        return t;
    }

    Token t = literal_token(in.c_str(), in.size(), &ix);
    end = ix;
    return t;
}

std::string parser_read_identifier(const std::string& in, uint64_t& ix, ErrList& errors) noexcept {
    std::string dest;
    if (ix < in.size()) {
        errors.emplace_back("Unexpected end of input", ix);
        return "";
    }
    char c = in[ix];
    if (!is_identifier_start(c)) {
        errors.emplace_back(std::string{"Invalid character '"} + c + "'", ix);
        return "";
    }

    while (ix < in.size()) {
        c = in[ix];
        if (is_identifier(c)) {
            dest.push_back(c);
            ++ix;
            continue;
        }
        break;
    }
    return dest;
}

bool parser_skip_spaces(const std::string& in, uint64_t& ix, bool in_paren) noexcept {
    do {
        if (ix >= in.size()) {
            return false;
        }
        char c = in[ix];
        if ((c == '\r' || c == '\n') && !in_paren) {
            while (ix < in.size() && (in[ix] == '\r' || in[ix] == '\n')) {
                ++ix;
            }
            return true;
        }
        if (c != ' ' && c != '\t' && c != '\n' && c != '\r') {
            return false;
        }
        ++ix;
    } while (1);
}

uint64_t parser_read_indent(const std::string& in, uint64_t& ix) noexcept {
    uint64_t count = 0;
    while (ix + 3 < in.size()) {
        for (uint64_t i = 0; ix < 4; ++i) {
            if (in[ix + i] != ' ' && in[ix + i] != '\t') {
                return count;
            }
        }
        ix += 4;
        ++count;

    }
    return count;
}

uint64_t parser_read_uint(const std::string& in, uint64_t& ix, uint8_t base,
                          ErrList& errors) {
    uint64_t n = 0;
    if (ix >= in.size()) {
        errors.emplace_back("Unexpected end of input", ix);
        return 0;
    }
    char c = in[ix];
    uint64_t start = ix;
    bool overflow = false;

    if (base == 16) { // Hex
        if ((c < '0' || c > '9') && (c < 'a' || c > 'f') &&
            (c < 'A' && c > 'F')) {
            errors.emplace_back(std::string{"Invalid character '"} + c + "'", ix);
            return 0;
        }
        do {
            c = in[ix];
            if (n > 0xfffffffffffffff) {
                overflow = true;
                n = 0;
            }
            n = n << 4;
            if (c >= '0' && c <= '9') {
                n += c - '0';
            } else if (c >= 'a' && c <= 'f') {
                n += (c - 'a') + 10;
            } else if (c >= 'A' && c <= 'F') {
                n += (c - 'A') + 10;
            } else {
                break;
            }
            ++ix;
        } while (ix < in.size());
    } else if (base == 2) { // Binary
        if (c != '0' && c != '1') {
            errors.emplace_back(std::string{"Invalid character '"} + c + "'", ix);
            return 0;
        }
        do {
            c = in[ix];
            if (n > 0x7fffffffffffffff) {
                overflow = true;
                n = 0;
            }
            n = n << 1;
            if (c == '0' || c == '1') {
                n += c - '0';
            } else {
                break;
            }
            ++ix;
        } while (ix < in.size());
    } else if (base == 8) { // Octal
        if (c < '0' || c > '7') {
            errors.emplace_back(std::string{"Invalid character '"} + c + "'", ix);
            return 0;
        }
        do {
            c = in[ix];
            if (n > 0x1fffffffffffffff) {
                overflow = true;
                n = 0;
            }
            n = n << 3;
            if (c >= '0' && c <= '7') {
                n += c - '0';
            } else {
                break;
            }
            ++ix;
        } while (ix < in.size());
    } else { // Base 10
        if (c < '0' || c > '9') {
            errors.emplace_back(std::string{"Invalid character '"} + c + "'", ix);
            return 0;
        }
        do {
            c = in[ix];
            if (c < '0' || c > '9') {
                break;
            }
            uint64_t v = c - '0';
            if (n > 0x1999999999999999) {
                overflow = true;
                n = 0;
            } else if (n == 0x1999999999999999) {
                if (v > 5) {
                    overflow = true;
                    n = 0;
                }
            }
            n = n * 10;
            n += v;
            ++ix;
        } while (ix < in.size());
    }
    if (overflow) {
        errors.emplace_back("Integer literal overflows", ix);
        return 0;
    }

    return n;
}


std::variant<uint64_t, double> parser_read_number(const std::string& in, uint64_t& ix,
                                                  ErrList& errors) noexcept {
    if (ix >= in.size()) {
        errors.emplace_back("Unexpected end of input", ix);
        return {(uint64_t)0};
    }
    char c = in[ix];
    if ((c < '0' || c > '9') && c != '.') {
        errors.emplace_back(std::string{"Invalid character '"} + c + "'", ix);
        return {(uint64_t)0};
    }
    uint8_t base = 10;
    if (c == '0' && ix + 1 < in.size()) {
        char p = in[ix + 1];
        if (p == 'x' || p == 'X') {
            base = 16;
            ix += 2;
        } else if (p == 'b' || p == 'B') {
            base = 2;
            ix += 2;
        } else if (p == 'o' || p == 'O') {
            base = 8;
            ix += 2;
        }
        if (ix >= in.size()) {
            errors.emplace_back("Unexpected end of input", ix);
            return {(uint64_t)0};
        }
        c = in[ix];
    }
    bool is_int = true;

    if (base != 10) {
        return parser_read_uint(in, ix, base, errors);
    }
    for (uint64_t i = ix; i < in.size(); ++i) {
        if (in[i] >= '0' && in[i] <= '9') {
            continue;
        }
        if (in[i] == 'e' || in[i] == 'E' || in[i] == '.') {
            is_int = false;
        }
        break;
    }
    if (is_int) {
        return parser_read_uint(in, ix, 10, errors);
    }

    double d = 0.0;

    if (c == '.') {
        d = 0.0;
    } else {
        d = (c - '0');
        ++ix;
        if (ix >= in.size()) {
            return d;
        }
    }

    if (c != '0' && c != '.') {
        c = in[ix];
        while (c >= '0' && c <= '9') {
            d *= 10;
            d += c - '0';
            ++ix;
            if (ix >= in.size()) {
                return d;
            }
            c = in[ix];
        }
    } else if (c == '0'){
        c = in[ix];
        if (c >= '0' && c <= '9') {
            errors.emplace_back(std::string{"Invalid character '"} + c + "'", ix);
            return 0.0;
        }
    }
    if (c == '.') {
        ++ix;
        if (ix >= in.size()) {
            errors.emplace_back("Unexpected end of input", ix);
            return 0.0;
        }
        c = in[ix];
        if (c < '0' || c > '9') {
            errors.emplace_back(std::string{"Invalid character '"} + c + "'", ix);
            return 0.0;
        }
        double factor = 0.1;
        while (c >= '0' && c <= '9') {
            d += (c - '0') * factor;
            factor *= 0.1;
            ++ix;
            if (ix >= in.size()) {
                return d;
            }
            c = in[ix];
        }
    }
    if (c == 'e' || c == 'E') {
        ++ix;
        if (ix >= in.size()) {
            errors.emplace_back("Unexpected end of input", ix);
            return 0.0;
        }
        c = in[ix];
        int64_t exp = 0;
        bool neg_exp = false;
        if (c == '-' || c == '+') {
            neg_exp = c == '-';
            ++ix;
            if (ix >= in.size()) {
                errors.emplace_back("Unexpected end of input", ix);
                return 0.0;
            }
            c = in[ix];
        }
        if (c < '0' || c > '9') {
            errors.emplace_back(std::string{"Invalid character '"} + c + "'", ix);
            return 0.0;
        }
        while (c >= '0' && c <= '9') {
            exp = exp * 10;
            exp += c - '0';
            ++ix;
            if (ix >= in.size()) {
                break;
            }
            c = in[ix];
        }
        double factor = 1.0;
        while (exp > 15) {
            exp -= 15;
            // 10^15 is the largest power of 10 that can be fully represented by a double
            factor *= 1000000000000000;
        }
        int64_t exps[15] = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000, 
                            1000000000, 10000000000, 100000000000, 1000000000000, 10000000000000,
                            100000000000000};
        factor *= exps[exp];
        if (neg_exp) {
            d /= factor;
        } else {
            d *= factor;
        }
    }
    return d;
}

// Read a string literal
std::string parser_read_string(const std::string& in, uint64_t& ix, char end,
                               ErrList& errors) noexcept {
    // Skip leading '"'
    ix += 1;

    std::string dest;

    while (1) {
        if (ix >= in.size()) {
            errors.emplace_back("Unexpected end of input", ix);
            return "";
        }
        char c = in[ix];
        ++ix;
        if (c == end) {
            break;
        } else if (c == '\\') {
            if (ix >= in.size()) {
                errors.emplace_back("Unexpected end of input", ix);
                return "";
            } 
            c = in[ix];
            ++ix;
            if (c == '\\' || c == '"' || c == '\'') {
                dest.push_back(c);
            } else if (c == 'n') {
                dest.push_back('\n');
            } else if (c == 'r') {
                dest.push_back('\r');
            } else if (c == 'v') {
                dest.push_back('\v');
            } else if (c == '\f') {
                dest.push_back('\f');
            } else if (c == '\t') {
                dest.push_back('\t');
            } else if (c == '\0') {
                dest.push_back('\0');
            } else if (c == 'x') {
                if (ix >= in.size()) {
                    errors.emplace_back("Unexpected end of input", ix);
                    return "";
                }
                c = in[ix];
                ++ix;
                uint8_t hex = 0;
                if (c >= '0' && c <= '9') {
                    hex = c - '0';
                } else if (c >= 'a' && c <= 'f') {
                    hex = c - 'a' + 10;
                } else if (c >= 'A' && c <= 'F') {
                    hex = c - 'A' + 10;
                } else {
                    errors.emplace_back("Invalid escape sequence", ix);
                }
                if (ix >= in.size()) {
                    errors.emplace_back("Unexpected end of input", ix);
                    return "";
                }
                c = in[ix];
                if (c >= '0' && c <= '9') {
                    hex = (hex << 4) + (c - '0');
                    ++ix;
                } else if (c >= 'a' && c <= 'f') {
                    hex = (hex << 4) + (c - 'a' + 10);
                    ++ix;
                } else if (c >= 'A' && c <= 'F') {
                    hex = (hex << 4) + (c - 'A' + 10);
                    ++ix;
                }
                dest.push_back(hex);
            } else {
                errors.emplace_back("Invalid escape sequence", ix);
            }
        } else {
            dest.push_back(c);
        }
    }

    return dest;
}

