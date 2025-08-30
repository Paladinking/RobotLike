#ifndef TOKENIZER_H
#define TOKENIZER_H

#include "parse.h"
#include <variant>
#include <vector>
#include <string>
#include <exception>

static inline bool is_identifier(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
             c == '_' || (c >= '0' && c <= '9');
}

static inline bool is_identifier_start(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') ||
            (c == '_');
}

class ParseError: std::exception {
public:
    uint64_t index;
    std::string cause;
    ParseError(uint64_t index, std::string cause) :
            index{index}, cause{std::move(cause)} {}
};

typedef std::vector<std::pair<std::string, uint64_t>> ErrList;

class Tokenizer {
    uint64_t paren_count[3];
    bool last_was_eol;
    uint64_t indent_level;
public:
    Tokenizer();

    Token get_token(const std::string& in, uint64_t& ix, uint64_t& start, uint64_t& end,
                    ErrList& errors) noexcept;
};

extern std::string parser_read_identifier(const std::string& in, uint64_t& ix,
                                          ErrList& errors) noexcept;

// true if line-ending
extern bool parser_skip_spaces(const std::string& in, uint64_t& ix, bool in_paren) noexcept;

extern uint64_t parser_read_indent(const std::string& in, uint64_t& ix) noexcept;

extern std::variant<uint64_t, double> parser_read_number(const std::string& in, uint64_t& ix,
                                                         ErrList& errors) noexcept;

extern std::string parser_read_string(const std::string& in, uint64_t& ix, char end,
                                      ErrList& errors) noexcept;

#endif
