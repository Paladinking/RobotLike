#ifndef PARSER_H
#define PARSER_H
#include "language.h"


class Parser {
    std::unordered_map<std::string, BuiltinCall::Type> builtins {};

    std::unordered_map<std::string, int32_t> names {};

    using Lines = const std::vector<std::string>&;

    int32_t get_var(const std::string& s);

    bool name_free(const std::string& s) const;

    bool read_ident(Lines lines, std::string& dest, bool allow_space = true);

    void expect_ident(Lines lines, std::string& dest, bool allow_space = true);

    bool has_indent(Lines, int32_t indent);

    void expect_char(Lines lines, char c, bool allow_space = true);

    void expect_eol(Lines lines);

    void parse_expression_list(Lines lines, std::vector<Expression*>& dest);

    void parse_name_list(Lines lines, std::vector<std::string>& dest);

    Expression* parse_expression(Lines lines);

    Statement* parse_statement(Lines lines, int32_t indent);

    void parse_statements(Lines lines, int32_t indents, std::vector<Statement*>& dest);

    int32_t ix;
    int32_t line;
    int32_t next_name;

    IfStatement* last_if = nullptr;

public:
    std::vector<std::unique_ptr<Statement>> all_statements;
    std::vector<std::unique_ptr<Expression>> all_expressions;
    std::vector<std::unique_ptr<Function>> all_functions;

    Statement* entry;
    

    Parser();

    std::vector<std::pair<std::string, int32_t>> errors;

    bool parse_lines(const std::vector<std::string>& lines);



};


#endif
