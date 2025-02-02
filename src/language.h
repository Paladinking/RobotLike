#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <vector>
#include <unordered_map>
#include <memory>
#include <string>
#include <cassert>
#include <iostream>
#include <exception>
#include <thread>


class RuntimeError: std::exception {
public:
    int lineno;
    std::string cause;
    RuntimeError(int lineno, std::string cause) : lineno{lineno}, cause{std::move(cause)} {}
};

class StopException: std::exception {};


struct Value {
    enum Type {
        TUPLE, DOUBLE, INT64, BOOL, NONE
    } type;
    union {
        std::vector<Value>* tuple;
        double d;
        int64_t i;
        bool b;
    };

    explicit Value(bool b) : type{BOOL}, b{b} {}
    explicit Value(int64_t i): type{INT64}, i{i} {}
    explicit Value(double d): type{DOUBLE}, d{d} {}
    explicit Value(std::vector<Value>* tuple): type{TUPLE}, tuple{tuple} {}
    explicit Value() : type{NONE} {}

    bool numeric() const {
        return type == DOUBLE || type == INT64;
    }

    bool boolean() const {
        switch (type) {
            case TUPLE:
                return tuple->size() > 0;
            case DOUBLE:
                return d != 0.0;
            case INT64:
                return i != 0;
            case BOOL:
                return b;
            case NONE:
            default:
                return false;
        }
    }

    void write(std::ostream& o) {
        switch (type) {
            case TUPLE:
                o << "(";
                for (Value& v: *tuple) {
                    v.write(o);
                    o << ',';
                }
                o << ")";
                break;
            case DOUBLE:
                o << d;
                break;
            case BOOL:
                o << (b ? "True" : "False");
                break;
            case INT64:
                o << i;
                break;
            case NONE:
                o << "None";
                break;
        
        }
    }
};

class Expression;
class Statement;
class Function;

class Program {
    std::unordered_map<int32_t, Value> globals;
    std::vector<std::unordered_map<int32_t, Value>> scopes;

    std::unordered_map<int32_t, Function*> funcs;

    // Used for memory only
    std::vector<std::unique_ptr<Expression>> all_exprs;
    std::vector<std::unique_ptr<Statement>> all_statements;
    std::vector<std::unique_ptr<Function>> all_functions;

    // Reference counts
    std::unordered_map<std::vector<Value>*, int32_t> tuples;

    std::atomic_bool paused{false};
    std::atomic_bool running{false};
    std::atomic<int32_t> lineno{0};

    std::thread run_thread;

    Value return_val = Value();
public:
    uint32_t EVT_PRINT;

    Statement* entrypoint;

    ~Program() {
        stop();
    }

    void runtime_error(int32_t lineno, const std::string& cause);

    void status(int32_t lineno);

    void set_events(uint32_t print_evt);

    void add_scope();
    
    void load_program(std::vector<std::unique_ptr<Statement>> statements, std::vector<std::unique_ptr<Expression>> expressions, std::vector<std::unique_ptr<Function>> all_funcs, Statement* entry); // Outside thread

    void pause(); // Outside thread

    void resume(); // Outside thread

    void step(); // Outside thread

    void stop(); // Outside thread

    void start(); // Outside thread

    void remove_scope();

    // Implicit add_ref
    void set_var(int32_t id, Value val, bool param);

    Value get_var(int32_t id);

    void set_function(int32_t id, Function* f);

    Function* get_function(int32_t id);

    void set_return(Value val);

    Value get_return();

    // Implicit add_ref
    std::vector<Value>* add_tuple(std::vector<Value> tuple);

    void add_ref(Value val);

    void remove_ref(Value val);
};


class Expression {
public:
    Expression(int32_t lineno) : lineno{lineno} {}

    int32_t lineno;

    virtual ~Expression() {}

    virtual Value evaluate(Program& p) const = 0;
};

class Statement {
public:
    enum Status {
        RETURN, BREAK, CONTINUE, NEXT
    };

    Statement(int32_t lineno) : lineno{lineno} {}

    int32_t lineno;

    virtual ~Statement() {}

    virtual Status evaluate(Program& p) const = 0;
};

class Function {
public:
    Function(std::vector<int32_t> params, std::vector<Statement*> statements) : params{params}, statements{statements} {}

    std::vector<int32_t> params;

    std::vector<Statement*> statements;
};

class Literal {
public:
    enum Type {
        TUPLE, DOUBLE, INT64, BOOL, NONE
    } type;

    explicit Literal(double d) : type{DOUBLE}, d{d} {}
    explicit Literal(int64_t i) : type{INT64}, i{i} {}
    explicit Literal(bool b) : type{BOOL}, b{b} {}
    explicit Literal(std::vector<Expression*> tuple) : type{TUPLE}, tuple{(std::move(tuple))} {}
    Literal() : type{NONE} {}

    Literal(Literal&& other) : type {other.type} {
        other.type = NONE;
        switch(type) {
            case DOUBLE:
                d = other.d;
                break;
            case INT64:
                i = other.i;
                break;
            case BOOL:
                b = other.b;
                break;
            case TUPLE:
                new (&tuple)std::vector<Expression*>(std::move(other.tuple));
                break;
            case NONE:
                break;
        }
    }
    Literal(const Literal& other) = delete;
    Literal& operator=(const Literal& other) = delete;
    Literal& operator=(Literal&& other) = delete;

    ~Literal() {
        if (type == TUPLE) {
            using namespace std;
            tuple.~vector<Expression*>();
        }
    }

    Value to_value(Program& p) const {
        if (type == DOUBLE) {
            return Value(d);
        }
        if (type == INT64) {
            return Value(i);
        }
        if (type == BOOL) {
            return Value(b);
        }
        if (type == NONE) {
            return Value();
        }
        std::vector<Value> vals;
        vals.reserve(tuple.size());
        for (auto& lit: tuple) {
            vals.push_back(lit->evaluate(p));
        }
        auto* v = p.add_tuple(std::move(vals));
        return Value(v);
    }

    union {
        std::vector<Expression*> tuple;
        double d;
        int64_t i;
        bool b;
    };
};

class LiteralExpr : public Expression {
    Literal val;
public:
    LiteralExpr(Literal&& val, int32_t lineno) :  Expression(lineno), val{std::move(val)} {}
    
    virtual Value evaluate(Program& p) const override {
        p.status(lineno);
        Value v = val.to_value(p);
        return v;
    }
};

class BinOp : public Expression {
    Expression* lhs;
    Expression* rhs;
public:
    enum Type {
        ADD, SUB, MUL, DIV, IDIV, MOD, BITOR, BITAND, BITXOR, AND, OR,
        GT, LT, GTE, LTE, EQ, NEQ
    } type;
    BinOp(Type type, Expression* lhs, Expression* rhs, int32_t lineno) : 
          Expression(lineno), type{type}, lhs{lhs}, rhs{rhs} {}

    virtual Value evaluate(Program& program) const override;

};

class UniOp : public Expression {
    Expression* e;
public:
    enum Type {
        NEGATE, NOT, PAREN
    } type;
    UniOp(int32_t lineno, Type type, Expression* e) : Expression{lineno}, type{type}, e{e} {}

    virtual Value evaluate(Program& p) const override;
};

class FuncCall : public Expression {
    int32_t name_id;
    std::vector<Expression*> args;
public:
    FuncCall(int32_t lineno, int32_t name_id, std::vector<Expression*> args) : Expression{lineno}, name_id{name_id}, args{args} {}

    virtual Value evaluate(Program& p) const override;
};

class BuiltinCall : public Expression {
    std::vector<Expression*> args;
public:
    enum Type {
        LENGTH, PRINT, ELEM, TUPLE
    } type;
    BuiltinCall(int32_t lineno, Type type , std::vector<Expression*> args) : Expression{lineno}, type{type}, args{args} {}

    virtual Value evaluate(Program& p) const override;
};

class VariableExpr : public Expression {
    int32_t id;
public:
    VariableExpr(int32_t lineno, int32_t id) : Expression{lineno}, id{id} {}

    virtual Value evaluate(Program& p) const override;
};


class Assignment: public Statement {
    int32_t id;
    Expression* val;
public:
    Assignment(int32_t lineno, int32_t id, Expression* expr) : Statement{lineno}, id{id}, val{expr} {}

    virtual Status evaluate(Program& p) const override;
};

class ExpressionStatement: public Statement {
    Expression* expr;
public:
    ExpressionStatement(int32_t lineno, Expression* expr) : Statement{lineno}, expr{expr} {}

    virtual Status evaluate(Program& p) const override;
};

class ReturnStatement: public Statement {
    Expression* expr;
public:
    ReturnStatement(int32_t lineno, Expression* expr) : Statement{lineno}, expr{expr} {}

    virtual Status evaluate(Program& p) const override;
};


class FlowStatement: public Statement {
    bool is_break;
public:
    FlowStatement(int32_t lineno, bool is_break) : Statement{lineno}, is_break{is_break} {}

    virtual Status evaluate(Program& p) const override;
};

class IfStatement: public Statement {
    Expression* cond;
    std::vector<Statement*> on_if;
public:
    IfStatement* next;
    IfStatement(int32_t lineno, Expression* cond, std::vector<Statement*> on_if, IfStatement* next) : Statement(lineno), cond{cond}, on_if{std::move(on_if)}, next{next} {}


    virtual Status evaluate(Program& p) const override;
};

class WhileStatement: public Statement {
    Expression* cond;
    std::vector<Statement*> statements;
public:
    WhileStatement(int32_t lineno, Expression* cond, std::vector<Statement*> statements):  Statement{lineno}, cond{cond}, statements{std::move(statements)} {}
    virtual Status evaluate(Program& p) const override;
};

class ForStatement: public Statement {
    Expression* expr;
    int32_t var_id;
    std::vector<Statement*> statements;
public:
    ForStatement(int32_t lineno, Expression* expr, int32_t var_id, std::vector<Statement*> statements) : Statement{lineno}, expr{expr}, var_id{var_id}, statements{std::move(statements)} {}
    virtual Status evaluate(Program& p) const override;
};

class GlobalStatement: public Statement {
    std::vector<Statement*> statements;
public:
    GlobalStatement(std::vector<Statement*> statements) : Statement{0}, statements{std::move(statements)} {}
    virtual Status evaluate(Program& p) const override;
};

class FuncDef: public Statement {
    Function* function;
    int32_t name_id;
public:
    FuncDef(int32_t lineno, Function* f, int32_t name_id) : Statement{lineno}, function{f}, name_id{name_id} {}
    
    virtual Status evaluate(Program& p) const override;
};

#endif
