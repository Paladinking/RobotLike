#include "language.h"
#include "engine/engine.h"
#include <SDL3/SDL.h>
#include <chrono>
#include <sstream>

void Program::pause() { paused.store(true); }

void Program::resume() { paused.store(false); }

void Program::load_program(std::vector<std::unique_ptr<Statement>> statements,
                           std::vector<std::unique_ptr<Expression>> expressions,
                           std::vector<std::unique_ptr<Function>> all_funcs,
                           Statement *entry) {
    if (run_thread.joinable()) {
        stop();
    }
    all_exprs = std::move(expressions);
    all_statements = std::move(statements);
    all_functions = std::move(all_funcs);
    entrypoint = entry;
}

void entry(Program *program) {
    std::cout << "Run thread started" << std::endl;
    try {
        while (true) {
            program->entrypoint->evaluate(*program);
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(10ms);
        }
    } catch (RuntimeError &e) {
        SDL_Event evt;
        evt.type = program->EVT_PRINT;
        auto *s = new std::string("Runtime error: ");
        evt.user.data1 = s;
        *s += e.cause + " at line " + std::to_string(e.lineno + 1) + "\n";
        SDL_PushEvent(&evt);
    } catch (StopException &e) {
        std::cout << "Stopped" << std::endl;
    }
}

void Program::stop() {
    if (!run_thread.joinable()) {
        return;
    }
    running.store(false);
    run_thread.join();
    all_functions.clear();
    all_exprs.clear();
    all_statements.clear();

    globals.clear();
    scopes.clear();

    funcs.clear();

    tuples.clear();
}

void Program::start() {
    assert(!run_thread.joinable());
    paused.store(false);
    running.store(true);
    run_thread = std::thread{entry, this};
}

void Program::set_events(uint32_t print_evt) {
    EVT_PRINT = print_evt;
    EVT_MOVE = print_evt + 1;
    EVT_ROTL = print_evt + 2;
    EVT_ROTR = print_evt + 3;
    EVT_READ_TILE = print_evt + 4;
    EVT_MOVE_FORWARDS = print_evt + 5;
}

void Program::status(int32_t line) {
    using namespace std::chrono_literals;
    if (!running.load()) {
        throw StopException();
    }
    while (paused.load()) {
        if (!running.load()) {
            throw StopException();
        }
        std::this_thread::sleep_for(50ms);
    }
    this->lineno.store(line);
}

void Program::add_scope() { 
    if (scopes.size() >= 1024) {
        throw RuntimeError(lineno.load(), "Recursion limit hit");
    }
    scopes.emplace_back();
}

void Program::remove_scope() {
    scopes.pop_back();
}

void Program::set_var(int32_t id, Value val, bool param) {
    if (!scopes.empty()) {
        auto var = scopes.back().find(id);
        if (var != scopes.back().end()) {
            var->second = val;
            return;
        }
        if (param) {
            scopes.back().insert({id, val});
            return;
        }
    }
    auto global_var = globals.find(id);
    if (global_var != globals.end()) {
        global_var->second = val;
        return;
    }
    if (!scopes.empty()) {
        scopes.back().insert({id, val});
    } else {
        globals.insert({id, val});
    }
}

void Program::set_function(int32_t id, Function *f) { funcs.insert({id, f}); }

Function *Program::get_function(int32_t id) {
    auto it = funcs.find(id);
    if (it == funcs.end()) {
        throw RuntimeError(lineno.load(), "Tries to access undefined function");
    }
    return it->second;
}

Value Program::get_var(int32_t id) {
    if (scopes.size() > 0) {
        auto var = scopes.back().find(id);
        if (var != scopes.back().end()) {
            return var->second;
        }
    }
    auto global_var = globals.find(id);
    if (global_var != globals.end()) {
        return global_var->second;
    }
    throw RuntimeError(lineno.load(), "Tried to access undefined variable");
}

void Program::set_return(Value val) { return_val = std::move(val); }

Value Program::get_return() { return return_val; }

Value::Tuple Program::add_tuple(std::vector<Value> tuple) {
    auto *val = new std::vector<Value>(std::move(tuple));
    return Value::Tuple{val, tuples};
}

Value BinOp::evaluate(Program &p) const {
    p.status(lineno);
    Value left = lhs->evaluate(p);
    if (type == AND) {
        Value right = rhs->evaluate(p);
        if (left.boolean()) {
            return Value(right.boolean());
        }
        return Value(false);
    } else if (type == OR) {
        Value right = rhs->evaluate(p);
        if (!left.boolean()) {
            return Value(right.boolean());
        }
        return Value(true);
    }
    Value right = rhs->evaluate(p);
    auto dbl = [](const Value& v) {
        if (v.type == Value::DOUBLE) {
            return v.d;
        }
        assert(v.type == Value::INT64);
        return static_cast<double>(v.i);
    };

    if (left.type == Value::TUPLE || right.type == Value::TUPLE) {
        if (type != ADD) {
            throw RuntimeError(lineno, "Invalid binary operation for tuple");
        }
    }

    switch (type) {
    case ADD:
        if (left.type == Value::TUPLE && right.type == Value::TUPLE) {
            std::vector<Value> vals{*left.tuple};
            for (Value &v : *right.tuple) {
                vals.push_back(v);
            }
            return Value(p.add_tuple(std::move(vals)));
        }
        if (!left.numeric() || !right.numeric()) {
            throw RuntimeError(lineno, "Addition of non-numeric type");
        }
        if (left.type == Value::DOUBLE || right.type == Value::DOUBLE) {
            return Value(dbl(left) + dbl(right));
        }
        return Value(left.i + right.i);
    case SUB:
        if (!left.numeric() || !right.numeric()) {
            throw RuntimeError(lineno, "Subtraction of non-numeric type");
        }
        if (left.type == Value::DOUBLE || right.type == Value::DOUBLE) {
            return Value(dbl(left) - dbl(right));
        }
        return Value(left.i - right.i);
    case MUL:
        if (!left.numeric() || !right.numeric()) {
            throw RuntimeError(lineno, "Multplication of non-numeric type");
        }
        if (left.type == Value::DOUBLE || right.type == Value::DOUBLE) {
            return Value(dbl(left) * dbl(right));
        }
        return Value(left.i * right.i);
    case DIV:
        if (!left.numeric() || !right.numeric()) {
            throw RuntimeError(lineno, "Division of non-numeric type");
        }
        return Value(dbl(left) / dbl(right));
    case IDIV:
        if (!left.numeric() || !right.numeric()) {
            throw RuntimeError(lineno, "Division of non-numeric type");
        }
        if (left.type == Value::DOUBLE || right.type == Value::DOUBLE) {
            return Value(static_cast<int64_t>(dbl(left) / dbl(right)));
        }
        return Value(left.i / right.i);
    case MOD:
        if (left.type != Value::INT64 || right.type != Value::INT64) {
            throw RuntimeError(lineno, "Modulo of non-integer type");
        }
        return Value(left.i % right.i);
    case BITOR:
        if (left.type != Value::INT64 || right.type != Value::INT64) {
            throw RuntimeError(lineno, "Bitwise or of non-integer type");
        }
        return Value(left.i | right.i);
    case BITAND:
        if (left.type != Value::INT64 || right.type != Value::INT64) {
            throw RuntimeError(lineno, "Bitwise and of non-integer type");
        }
        return Value(left.i & right.i);
    case BITXOR:
        if (left.type != Value::INT64 || right.type != Value::INT64) {
            throw RuntimeError(lineno, "Bitwise xor of non-integer type");
        }
        return Value(left.i ^ right.i);
    case GT:
        if (!left.numeric() || !right.numeric()) {
            throw RuntimeError(lineno, "Comparison of non-numeric type");
        }
        if (left.type == Value::DOUBLE || right.type == Value::DOUBLE) {
            return Value(dbl(left) > dbl(right));
        }
        return Value(left.i > right.i);
    case LT:
        if (!left.numeric() || !right.numeric()) {
            throw RuntimeError(lineno, "Comparison of non-numeric type");
        }
        if (left.type == Value::DOUBLE || right.type == Value::DOUBLE) {
            return Value(dbl(left) < dbl(right));
        }
        return Value(left.i < right.i);
    case GTE:
        if (!left.numeric() || !right.numeric()) {
            throw RuntimeError(lineno, "Comparison of non-numeric type");
        }
        if (left.type == Value::DOUBLE || right.type == Value::DOUBLE) {
            return Value(dbl(left) >= dbl(right));
        }
        return Value(left.i > right.i);
    case LTE:
        if (!left.numeric() || !right.numeric()) {
            throw RuntimeError(lineno, "Comparison of non-numeric type");
        }
        if (left.type == Value::DOUBLE || right.type == Value::DOUBLE) {
            return Value(dbl(left) <= dbl(right));
        }
        return Value(left.i <= right.i);
    case EQ:
        if (left.type == Value::NONE || right.type == Value::NONE) {
            return Value(left.type == right.type);
        } else if (left.type == Value::BOOL || right.type == Value::BOOL) {
            return Value(right.type == left.type && left.b == right.b);
        } else if (left.type == Value::DOUBLE || right.type == Value::DOUBLE) {
            return Value(dbl(left) == dbl(right));
        } else {
            return Value(left.i == right.i);
        }
    case NEQ:
        if (left.type == Value::NONE || right.type == Value::NONE) {
            return Value(left.type != right.type);
        } else if (left.type == Value::BOOL || right.type == Value::BOOL) {
            return Value(right.type != left.type || left.b == right.b);
        } else if (left.type == Value::DOUBLE || right.type == Value::DOUBLE) {
            return Value(dbl(left) != dbl(right));
        } else {
            return Value(left.i == right.i);
        }
    case AND:
    case OR:
        return Value(); // Unreachable
    }
    return Value();
}

Value UniOp::evaluate(Program &p) const {
    if (type == PAREN) {
        return e->evaluate(p);
    }
    p.status(lineno);
    Value inner = e->evaluate(p);
    if (type == NEGATE) {
        if (!inner.numeric()) {
            throw RuntimeError(lineno, "Negation of non-numeric type");
        }
        if (inner.type == Value::DOUBLE) {
            return Value(-inner.d);
        }
        return Value(-inner.i);
    } else {
        if (inner.type != Value::BOOL) {
            throw RuntimeError(lineno, "Boolean not of non-boolean type");
        }
        return Value(!inner.b);
    }
}

Value BuiltinCall::evaluate(Program &p) const {
    p.status(lineno);
    auto intv = [&p, this](const Value& v) -> int64_t {
        if (!v.numeric()) {
            throw RuntimeError(lineno, "Invalid integer");
        }
        if (v.type == Value::DOUBLE) {
            auto i = static_cast<int64_t>(v.d);
            if (static_cast<double>(i) != v.d) {
                throw RuntimeError(lineno, "Invalid integer");
            }
            return i;
        } else {
            return v.i;
        }
    };

    if (type == RANDOM) {
        Value v = Value(engine::random<int64_t>(0, 100));
        return v;
    } else if (type == FORWARDS) {
        SDL_Event e;
        e.type = p.EVT_MOVE_FORWARDS;
        if (!args.empty()) {
            throw RuntimeError(lineno, "Wrong number of arguments");
        }
        SDL_PushEvent(&e);
        p.pause();
    } else if (type == READ_FRONT) {
        SDL_Event e;
        e.type = p.EVT_READ_TILE;
        if (!args.empty()) {
            throw RuntimeError(lineno, "Wrong number of arguments");
        }
        auto *t = new ReadTile();
        e.user.data1 = t;
        SDL_PushEvent(&e);
        p.pause();
        p.status(lineno);

        t->m.lock();
        Value v = Value(t->is_open);
        t->m.unlock();
        delete t;
        return v;
    } else if (type == ROTR) {
        SDL_Event e;
        e.type = p.EVT_ROTR;
        if (args.size() != 0) {
            throw RuntimeError(lineno, "Wrong number of arguments");
        }
        SDL_PushEvent(&e);
        p.pause();
        return Value();
    } else if (type == ROTL) {
        SDL_Event e;
        e.type = p.EVT_ROTL;
        if (!args.empty()) {
            throw RuntimeError(lineno, "Wrong number of arguments");
        }
        SDL_PushEvent(&e);
        p.pause();
        return Value();
    } else if (type == MOVE) {
        SDL_Event e;
        e.type = p.EVT_MOVE;
        if (args.size() != 2) {
            throw RuntimeError(lineno, "Wrong number of arguments");
        }
        int64_t x = intv(args[0]->evaluate(p));
        int64_t y = intv(args[1]->evaluate(p));
        if (x < 0) {
            x = 0b11;
        } else if (x > 0) {
            x = 0b01;
        }
        if (y < 0) {
            y = 0b11;
        } else if (y > 0) {
            y = 0b01;
        }
        // 4-bit value two x, two y.
        e.user.data1 = (void *)(uintptr_t)(x << 2 | y);
        SDL_PushEvent(&e);
        p.pause();
    } else if (type == ELEM) {
        if (args.size() != 2) {
            throw RuntimeError(lineno, "Wrong number of arguments");
        }
        Value t = args[0]->evaluate(p);
        Value ix = args[1]->evaluate(p);
        if (t.type != Value::TUPLE || !ix.numeric()) {
            throw RuntimeError(lineno, "Invalid argument");
        }
        int64_t i;
        if (ix.type == Value::DOUBLE) {
            i = static_cast<int64_t>(ix.d);
            if (static_cast<double>(i) != ix.d) {
                throw RuntimeError(lineno, "Non-integer index");
            }
        } else {
            i = ix.i;
        }
        if (i >= t.tuple->size()) {
            throw RuntimeError(lineno, "Index out of bounds");
        }
        Value res = (*t.tuple)[i];
        return res;
    } else if (type == TUPLE) {
        std::vector<Value> v;
        v.reserve(args.size());
        for (auto &a : args) {
            v.push_back(a->evaluate(p));
        }
        return Value(p.add_tuple(v));
    } else if (type == LENGTH) {
        if (args.size() != 1) {
            throw RuntimeError(lineno, "Wrong number of arguments");
        }
        Value v = args[0]->evaluate(p);
        if (v.type != Value::TUPLE) {
            throw RuntimeError(lineno, "len requires tuple");
        }
        return Value(static_cast<int64_t>(v.tuple->size()));
    } else if (type == PRINT) {
        std::stringstream ss{};
        SDL_Event e;
        e.type = p.EVT_PRINT;
        if (args.empty()) {
            ss << "\n";
            e.user.data1 = new std::string{ss.str()};
            SDL_PushEvent(&e);
            return Value();
        }
        Value first = args[0]->evaluate(p);
        first.write(ss);
        for (size_t ix = 1; ix < args.size(); ++ix) {
            ss << ", ";
            Value v = args[ix]->evaluate(p);
            v.write(ss);
        }
        ss << "\n";
        e.user.data1 = new std::string{ss.str()};
        SDL_PushEvent(&e);
        p.pause();
        return Value();
    }
    // Unreachable
    return Value();
}

Value FuncCall::evaluate(Program &p) const {
    p.status(lineno);
    Function *f = p.get_function(name_id);

    if (args.size() != f->params.size()) {
        throw RuntimeError(lineno, "Wrong number of arguments");
    }

    p.add_scope();
    for (size_t ix = 0; ix < args.size(); ++ix) {
        Value v = args[ix]->evaluate(p);
        p.set_var(f->params[ix], v, true);
    }
    for (Statement *s : f->statements) {
        Statement::Status status = s->evaluate(p);
        if (status == Statement::RETURN) {
            Value v = p.get_return();
            p.remove_scope();
            return v;
        } else if (status != Statement::NEXT) {
            p.remove_scope();
            throw RuntimeError(lineno, "Invalid placement of break / continue");
        }
    }
    p.remove_scope();
    return Value();
}

Value VariableExpr::evaluate(Program &p) const {
    p.status(lineno);
    Value v = p.get_var(id);
    return v;
}

Statement::Status Assignment::evaluate(Program &p) const {
    p.status(lineno);
    Value v = val->evaluate(p);
    p.set_var(id, v, false);
    return Statement::NEXT;
}

Statement::Status ExpressionStatement::evaluate(Program &p) const {
    Value v = expr->evaluate(p);
    return Statement::NEXT;
}

Statement::Status ReturnStatement::evaluate(Program &p) const {
    p.status(lineno);
    if (expr == nullptr) {
        p.set_return(Value());
    } else {
        Value v = expr->evaluate(p);
        p.set_return(v);
    }
    return Statement::RETURN;
}

Statement::Status FlowStatement::evaluate(Program &p) const {
    p.status(lineno);
    if (is_break) {
        return Statement::BREAK;
    } else {
        return Statement::CONTINUE;
    }
}

Statement::Status IfStatement::evaluate(Program &p) const {
    bool c = true;
    p.status(lineno);
    if (cond != nullptr) {
        Value v = cond->evaluate(p);
        c = v.boolean();
    }
    if (c) {
        for (Statement *s : on_if) {
            Statement::Status status = s->evaluate(p);
            if (status != Statement::NEXT) {
                return status;
            }
        }
    } else if (next != nullptr) {
        return next->evaluate(p);
    }
    return Statement::NEXT;
}

Statement::Status WhileStatement::evaluate(Program &p) const {
    p.status(lineno);
    while (true) {
        Value v = cond->evaluate(p);
        bool c = v.boolean();
        if (!c) {
            break;
        }
        for (Statement *s : statements) {
            Statement::Status status = s->evaluate(p);
            if (status == Statement::RETURN) {
                return Statement::RETURN;
            } else if (status == Statement::BREAK) {
                goto end;
            } else if (status == Statement::CONTINUE) {
                break;
            }
        }
    }
end:
    return Statement::NEXT;
}

Statement::Status ForStatement::evaluate(Program &p) const {
    p.status(lineno);
    Value v = expr->evaluate(p);
    if (v.type != Value::TUPLE) {
        throw RuntimeError(lineno, "For loop requires tuple");
    }

    for (const auto & ix : *v.tuple) {
        p.set_var(var_id, ix, false);
        for (Statement *s : statements) {
            Statement::Status status = s->evaluate(p);
            if (status == Statement::RETURN) {
                return Statement::RETURN;
            } else if (status == Statement::BREAK) {
                goto end;
            } else if (status == Statement::CONTINUE) {
                break;
            }
        }
    }
end:
    return Statement::NEXT;
}

Statement::Status GlobalStatement::evaluate(Program &p) const {
    p.status(lineno);
    for (Statement *s : statements) {
        Statement::Status status = s->evaluate(p);
        if (status != Statement::NEXT) {
            throw RuntimeError(lineno, "Illegal statement");
        }
    }
    return Statement::NEXT;
}

Statement::Status FuncDef::evaluate(Program &p) const {
    p.status(lineno);
    p.set_function(name_id, function);

    return Statement::NEXT;
}
