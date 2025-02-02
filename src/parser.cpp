#include "parser.h"

// Global scope:
//      <statement>|<function_def>
// <statement>:
//      <call>|<assignment>|<if>|<while>|<for>|<return>|<break>|<continue>
// <expr>
//      <call>|<var>|<unop>|<binop>|<literal>
//
//

class ParseError: std::exception {
public:
    int lineno;
    std::string cause;
    ParseError(int lineno, std::string cause) : lineno{lineno}, cause{std::move(cause)} {}
};


Parser::Parser() {
    builtins.insert({"len", BuiltinCall::LENGTH});
    builtins.insert({"print", BuiltinCall::PRINT});
    builtins.insert({"elem", BuiltinCall::ELEM});
    builtins.insert({"tuple", BuiltinCall::TUPLE});
    builtins.insert({"move", BuiltinCall::MOVE});
    builtins.insert({"rotate_left", BuiltinCall::ROTL});
    builtins.insert({"rotate_right", BuiltinCall::ROTR});
    builtins.insert({"forward", BuiltinCall::FORWARDS});
    builtins.insert({"read_front", BuiltinCall::READ_FRONT});
    builtins.insert({"rand", BuiltinCall::RANDOM});
}

// Advances ix to point to first non-space in string
// if line[ix] == L' ', does nothing
// true iff not eol
bool skip_spaces(int32_t& ix, const std::string& line) {
    while (ix < line.size() && line[ix] == ' ') {
        ++ix;
    }
    return ix < line.size();
}

bool is_empty(const std::string& line) {
    for (char c: line) {
        if (c == '#') {
            return true;
        } else if (c != ' ') {
            return false;
        }
    }
    return true;
}

static inline bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline bool is_number(char c) {
    return (c >= '0' && c <= '9');
}

static inline bool is_alphanum(char c) {
    return is_alpha(c) || is_number(c);
}

// read identifier
bool Parser::read_ident(Lines lines, std::string& dest, bool allow_space) {
    const std::string& s = lines[line];
    dest.clear();
    if (allow_space) {
        skip_spaces(ix, s);
    }
    if (ix >= s.size() || (s[ix] != '_' && !is_alpha(s[ix]))) {
        return false;
    }
    dest.push_back(s[ix]);
    ++ix;
    while (ix < s.size() && (s[ix] == '_' || is_alphanum(s[ix]))) {
        dest.push_back(s[ix]);
        ++ix;
    }
    skip_spaces(ix, s);
    return true;
}

void Parser::expect_ident(Lines lines, std::string& dest, bool allow_space) {
    if (!read_ident(lines, dest, allow_space)) {
        throw ParseError(line, "Expected identifier");
    }
}

void Parser::expect_eol(Lines lines) {
    skip_spaces(ix, lines[line]);
    if (ix < lines[line].size()) {
        throw ParseError(line, "Expected eol");
    }
}

void Parser::expect_char(Lines lines, char c, bool allow_space) {
    if (allow_space) {
        skip_spaces(ix, lines[line]);
    }
    if (ix >= lines[line].size() || lines[line][ix] != c) {
        throw ParseError(line, std::string("Expected '") + c  + "'");
    }
    ++ix;
}

bool Parser::has_indent(Lines lines, int32_t indent) {
    for (int32_t i = 0; i < indent * 4; ++i) {
        if (ix + i >= lines.size() || lines[line][ix + i] != ' ') {
            return false;
        }
    }
    ix += 4 * indent;
    return true;
}

int32_t Parser::get_var(const std::string& name) {
    auto it = names.find(name);
    if (it != names.end()) {
        return it->second;
    }
    names[name] = next_name;
    ++next_name;
    return next_name - 1;
}

Expression* Parser::parse_expression(Lines lines) {
    skip_spaces(ix, lines[line]);
    if (ix >= lines[line].size()) {
        throw ParseError(line, "Invalid expression");
    }
    int32_t lineno = line;
    Expression* var = nullptr;
    if (lines[line][ix] == '(') {
        ++ix;
        Expression* e = parse_expression(lines);
        expect_char(lines, ')');
        var = new UniOp(lineno, UniOp::PAREN, e);
    } else if (lines[line][ix] == '-') {
        ++ix;
        Expression* e = parse_expression(lines);
        var = new UniOp(lineno, UniOp::NEGATE, e);
    } else if (lines[line][ix] == '!') {
        ++ix;
        Expression* e = parse_expression(lines);
        var = new UniOp(lineno, UniOp::NOT, e);
    } else if (lines[line][ix] >= '0' && lines[line][ix] <= '9') {
        std::string s;
        double d;
        do {
            s.push_back(lines[line][ix]);
            ++ix;
        } while (ix < lines[line].size() && (lines[line][ix] == '.' ||
                 is_number(lines[line][ix])));
        try {
            d = std::stod(s);
        } catch (std::invalid_argument& e) {
            throw ParseError(lineno, "Invalid number");
        }
        var = new LiteralExpr(Literal(d), lineno);
        // literal
    } else {
        std::string id;
        expect_ident(lines, id);
        if (id == "None") {
            var = new LiteralExpr(Literal(), lineno);
        } else if (id == "True") {
            var = new LiteralExpr(Literal(true), lineno);
        } else if (id == "False") {
            var = new LiteralExpr(Literal(false), lineno);
        } else if (ix < lines[line].size() && lines[line][ix] == '(') {
            // Call
            std::vector<Expression*> args{};
            parse_expression_list(lines, args);
            auto it = builtins.find(id);
            if (it != builtins.end()) {
                var = new BuiltinCall(lineno, it->second, args);
            } else {
                int32_t var_id = get_var(id);
                var = new FuncCall(lineno, var_id, args);
            }
        } else {
            // Variable
            int32_t var_id = get_var(id);
            var = new VariableExpr(lineno, var_id);
        }
    }
    all_expressions.emplace_back(var);
    skip_spaces(ix, lines[line]);
    if (ix >= lines[line].size()) {
        return var;
    }
    std::string op;
    BinOp::Type type;
    if (read_ident(lines, op)) {
        if (op == "and") {
            type = BinOp::AND;
        } else if (op == "or") {
            type = BinOp::OR;
        } else {
            throw ParseError(line, "Invalid expression");
        }
    } else {
        if (lines[line][ix] == '+') {
            ++ix;
            type = BinOp::ADD;
        } else if (lines[line][ix] == '-') {
            ++ix;
            type = BinOp::SUB;
        } else if (lines[line][ix] == '*') {
            ++ix;
            type = BinOp::MUL;
        } else if (lines[line][ix] == '/') {
            ++ix;
            if (ix < lines[line].size() && lines[line][ix] == '/') {
                ++ix;
                type = BinOp::IDIV;
            } else {
                type = BinOp::DIV;
            }
        } else if (lines[line][ix] == '%') {
            ++ix;
            type = BinOp::MOD;
        } else if (lines[line][ix] == '&') {
            ++ix;
            type = BinOp::BITAND;
        } else if (lines[line][ix] == '|') {
            ++ix;
            type = BinOp::BITOR;
        } else if (lines[line][ix] == '^') {
            ++ix;
            type = BinOp::BITXOR;
        } else if (lines[line][ix] == '>') {
            ++ix;
            if (ix < lines[line].size() && lines[line][ix] == '=') {
                ++ix;
                type = BinOp::GTE;
            } else {
                type = BinOp::GT;
            }
        } else if (lines[line][ix] == '<') {
            ++ix;
            if (ix < lines[line].size() && lines[line][ix] == '=') {
                ++ix;
                type = BinOp::LTE;
            } else {
                type = BinOp::LT;
            }
        } else if (lines[line][ix] == '!') {
            if (ix < lines[line].size() && lines[line][ix] == '=') {
                ++ix;
                type = BinOp::NEQ;
            } else {
                throw ParseError(lineno, "Invalid expression");
            }
        } else if (lines[line][ix] == '=') {
            if (ix < lines[line].size() && lines[line][ix] == '=') {
                ++ix;
                type = BinOp::EQ;
            } else {
                throw ParseError(lineno, "Invalid expression");
            }
        } else {
            return var;
        }
    }
    Expression* next = parse_expression(lines);
    Expression* bin = new BinOp(type, var, next, line);
    all_expressions.emplace_back(bin);
    return bin;
}

Statement* Parser::parse_statement(Lines lines, int32_t indent) {
    std::string id;
    expect_ident(lines, id, false);
    std::vector<Statement*> statements;
    int32_t lineno = line;
    if (id == "elsif") {
        if (last_if == nullptr) {
            throw ParseError(lineno, "elsif without if");
        }
        IfStatement* target = last_if;
        Expression* cond = parse_expression(lines);
        expect_char(lines, ':');
        expect_eol(lines);
        ++line;
        ix = 0;
        parse_statements(lines, indent + 1, statements);
        auto* ifs = new IfStatement(lineno, cond, statements, nullptr);
        all_statements.emplace_back(ifs);
        target->next = ifs;
        last_if = ifs;
        return nullptr;
    } else if (id == "else") {
        if (last_if == nullptr) {
            throw ParseError(lineno, "else without if");
        }
        IfStatement* target = last_if;
        expect_char(lines, ':');
        expect_eol(lines);
        ++line;
        ix = 0;
        parse_statements(lines, indent + 1, statements);
        auto* ifs = new IfStatement(lineno, nullptr, statements, nullptr);
        all_statements.emplace_back(ifs);
        target->next = ifs;
        last_if = nullptr;
        return nullptr;
    } else {
        last_if = nullptr;
    }

    if (id == "if") {
        Expression* cond = parse_expression(lines);
        expect_char(lines, ':');
        expect_eol(lines);
        ++line;
        ix = 0;
        parse_statements(lines, indent + 1, statements);
        auto* ifs = new IfStatement(lineno, cond, statements, nullptr);
        all_statements.emplace_back(ifs);
        last_if = ifs;
        return ifs;
    } else if (id == "for") {
        std::string var;
        expect_ident(lines, var);
        int32_t var_id = get_var(var);
        expect_char(lines, 'i');
        expect_char(lines, 'n', false);
        Expression* iter = parse_expression(lines);
        expect_char(lines, ':');
        expect_eol(lines);
        ++line;
        ix = 0;
        parse_statements(lines, indent + 1, statements);
        auto* fors = new ForStatement(lineno, iter, var_id, statements);
        all_statements.emplace_back(fors);
        return fors;
    } else if (id == "while") {
        Expression* cond = parse_expression(lines);
        expect_char(lines, ':');
        expect_eol(lines);
        ++line;
        ix = 0;
        parse_statements(lines, indent + 1, statements);
        auto* whiles = new WhileStatement(lineno, cond, statements);
        all_statements.emplace_back(whiles);
        return whiles;
    } else if (id == "return") {
        Expression* expr = parse_expression(lines);
        expect_eol(lines);
        auto* ret = new ReturnStatement(lineno, expr);
        all_statements.emplace_back(ret);
        return ret;
    } else if (id == "break") {
        expect_eol(lines);
        auto* flow = new FlowStatement(lineno, true);
        all_statements.emplace_back(flow);
        return flow;
    } else if (id == "continue") {
        expect_eol(lines);
        auto* flow = new FlowStatement(lineno, false);
        all_statements.emplace_back(flow);
        return flow;
    } else { // FuncCall or assignment
        if (ix >= lines[line].size()) {
            throw ParseError(lineno, "Not a statement");
        }
        if (lines[line][ix] != '(') {
            // assignment
            int32_t var_id = get_var(id);
            expect_char(lines, '=');
            Expression* source = parse_expression(lines);
            expect_eol(lines);
            auto* assign = new Assignment(lineno, var_id, source);
            all_statements.emplace_back(assign);
            return assign;
        } else {
            std::vector<Expression*> args{};
            int32_t lineno = line;
            parse_expression_list(lines, args);
            expect_eol(lines);
            auto it = builtins.find(id);
            Expression* call;
            if (it != builtins.end()) {
                call = new BuiltinCall(lineno, it->second, args);
            } else {
                int32_t name_id = get_var(id);
                call = new FuncCall(lineno, name_id, args);
            }
            all_expressions.emplace_back(call);
            auto* stat = new ExpressionStatement(lineno, call);
            all_statements.emplace_back(stat);
            return stat;
        }
    }
}

void Parser::parse_statements(Lines lines, int32_t indents, std::vector<Statement*>& dest) {
    while (1) {
        if (line >= lines.size() || !has_indent(lines, indents)) {
            --line;
            ix = lines[line].size();
            return;
        }
        Statement* s = parse_statement(lines, indents);
        if (s != nullptr) {
            dest.push_back(s);
        }
        ++line;
        ix = 0;
    }
}


void Parser::parse_expression_list(Lines lines, std::vector<Expression*>& dest) {
    expect_char(lines, '(');
    while (ix >= lines[line].size()) {
        ix = 0;
        do {
            ++line;
            if (line >= lines.size()) {
                throw ParseError(line - 1, "Missing closing ')'");
            }
        } while (is_empty(lines[line]));
        skip_spaces(ix, lines[line]);
    }
    if (lines[line][ix] == ')') {
        ++ix;
        return;
    }
    dest.push_back(parse_expression(lines));
    while (1) {
        if (ix >= lines[line].size()) {
            ix = 0;
            do {
                ++line;
                if (line >= lines.size()) {
                    throw ParseError(line - 1, "Missing closing ')'");
                }
            } while (is_empty(lines[line]));
            skip_spaces(ix, lines[line]);
            continue;
        }
        if (lines[line][ix] == ')') {
            ++ix;
            return;
        }
        expect_char(lines, ',');
        dest.push_back(parse_expression(lines));
    }
}

void Parser::parse_name_list(Lines lines, std::vector<std::string>& dest) {
    expect_char(lines, '(');
    std::string s;
    if (!read_ident(lines, s)) {
        expect_char(lines, ')');
        return;
    }
    dest.push_back(s);
    while (1) {
        if (ix >= lines[line].size()) {
            ix = 0;
            do {
                ++line;
                if (line >= lines.size()) {
                    throw ParseError(line - 1, "Missing closing ')'");
                }
            } while (is_empty(lines[line]));
            skip_spaces(ix, lines[line]);
            continue;
        }
        if (lines[line][ix] == ')') {
            ++ix;
            return;
        }
        expect_char(lines, ',');
        expect_ident(lines, s);
        dest.push_back(s);
    }
}

bool Parser::parse_lines(const std::vector<std::string>& lines) {
    errors.clear();
    all_statements.clear();
    all_expressions.clear();
    all_functions.clear();
    entry = nullptr;

    ix = 0;
    line = 0;
    next_name = 0;
    std::string ident;

    std::vector<Statement*> main{};

    while (line < lines.size()) {
        if (is_empty(lines[line])) {
            ++line;
            ix = 0;
            continue;
        }
        try {
            int32_t old_ix = ix;
            int32_t old_line = line;
            expect_ident(lines, ident, false);
            if (ident == "fn") {
                last_if = nullptr;

                std::string name;
                expect_ident(lines, name);

                std::vector<std::string> args{};
                parse_name_list(lines, args);
                expect_char(lines, ':');
                expect_eol(lines);
                ++line;
                ix = 0;
                std::vector<Statement*> statements;
                parse_statements(lines, 1, statements);
                std::vector<int32_t> params {};
                for (auto& arg: args) {
                    params.push_back(get_var(arg));
                }
                int32_t func_id = get_var(name);
                auto* f = new Function(params, statements);
                all_functions.emplace_back(f);
                auto *def = new FuncDef(old_line, f, func_id);
                all_statements.emplace_back(def);
                main.push_back(def);
                ++line;
                ix = 0;
            } else {
                ix = old_ix;
                line = old_line;
                Statement* s = parse_statement(lines, 0);
                if (s != nullptr) {
                    main.push_back(s);
                }
                ++line;
                ix = 0;
            }
        } catch (ParseError& e) {
            errors.push_back({e.cause, e.lineno});
            ++line;
            last_if = nullptr;
            // Skip indentet
        }
    }
    if (errors.size() > 0) {
        return false;
    }
    entry = new GlobalStatement(main);
    return true;
}
