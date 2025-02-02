#include "language.h"
#include <chrono>
#include <sstream>
#include <SDL.h>


void Program::runtime_error(int32_t lineno, const std::string& cause) {
    throw RuntimeError(lineno, cause);
}

void Program::load_program(std::vector<std::unique_ptr<Statement>> statements, std::vector<std::unique_ptr<Expression>> expressions, std::vector<std::unique_ptr<Function>> all_funcs, Statement* entry) {
    if (run_thread.joinable()) {
        stop();
    }
    all_exprs = std::move(expressions);
    all_statements = std::move(statements);
    all_functions = std::move(all_funcs);
    entrypoint = entry;
}

void entry(Program* program) {
    std::cout << "Run thread started" << std::endl;
    try {
        while (1) {
            program->entrypoint->evaluate(*program);
            using namespace std::chrono_literals;
            std::this_thread::sleep_for(10ms);
        }
    } catch (RuntimeError& e) {
        SDL_Event evt;
        evt.type = program->EVT_PRINT;
        auto* s = new std::string("Runtime error: ");
        evt.user.data1 = s;
        *s += e.cause + " at line " + std::to_string(e.lineno + 1);
        SDL_PushEvent(&evt);
    } catch (StopException& e) {
        std::cout << "Stopped" << std::endl;
    } catch (...) {
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
}

void Program::start() {
    assert(!run_thread.joinable());
    paused.store(false);
    running.store(true);
    run_thread = std::thread{entry, this};
}

void Program::set_events(uint32_t print_evt) {
    EVT_PRINT = print_evt;
}

void Program::status(int32_t lineno) {
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
    this->lineno.store(lineno);
}

void Program::add_scope() {
    scopes.emplace_back();
}

void Program::remove_scope() {
    std::unordered_map<int32_t, Value>& top = scopes.back();

    for (const auto& p: top) {
        remove_ref(p.second);
    }
    scopes.pop_back();
}

void Program::set_var(int32_t id, Value val, bool param) {
    add_ref(val);
    if (scopes.size() > 0) {
        auto var = scopes.back().find(id);
        if (var != scopes.back().end()) {
            remove_ref(var->second);
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
        remove_ref(global_var->second);
        global_var->second = val;
        return;
    }
    if (scopes.size() > 0) {
        scopes.back().insert({id, val});
    } else {
        globals.insert({id, val});
    }
}

void Program::set_function(int32_t id, Function* f) {
    funcs.insert({id, f});
}

Function* Program::get_function(int32_t id) {
    auto it = funcs.find(id);
    if (it == funcs.end()) {
        runtime_error(lineno.load(), "Tries to access undefined function");
        return nullptr;
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
    runtime_error(lineno.load(), "Tried to access undefined variable");
    return Value();
}

void Program::set_return(Value val) {
    return_val = val;
}

Value Program::get_return() {
    return return_val;
}

// Implicit add_ref
std::vector<Value>* Program::add_tuple(std::vector<Value> tuple) {
    std::vector<Value>* val = new std::vector<Value>(std::move(tuple));
    tuples.insert({val, 1});
    return val;
}

void Program::add_ref(Value val) {
    if (val.type != Value::TUPLE) {
        return;
    }
    std::vector<Value>* tuple = val.tuple;
    auto it = tuples.find(tuple);
    assert(it != tuples.end());
    it->second += 1;
}

void Program::remove_ref(Value val) {
    if (val.type != Value::TUPLE) {
        return;
    }
    std::vector<Value>* tuple = val.tuple;
    auto it = tuples.find(tuple);
    assert(it != tuples.end());
    it->second -= 1;
    if (it->second == 0) {
        delete it->first;
        tuples.erase(it);
    }

}


Value BinOp::evaluate(Program& p) const{
    struct Defer {
        Value v;
        Program& p;

        ~Defer() {
            p.remove_ref(v);
        }
    };

    p.status(lineno);
    Value left = lhs->evaluate(p);
    Defer ref_left{left, p};
    if (type == AND) {
        Value right = rhs->evaluate(p);
        Defer ref_right{right, p};
        if (left.boolean()) {
            return Value(right.boolean());
        }
        return Value(false);
    } else if (type == OR) {
        Value right = rhs->evaluate(p);
        Defer ref_right{right, p};
        if (!left.boolean()) {
            return Value(right.boolean());
        }
        return Value(true);
    }
    Value right = rhs->evaluate(p);
    Defer ref_right{right, p};
    auto dbl = [] (Value v) {
        if (v.type == Value::DOUBLE) {
            return v.d;
        }
        assert(v.type == Value::INT64);
        return static_cast<double>(v.i);
    };

    if (left.type == Value::TUPLE || right.type == Value::TUPLE) {
        if (type != ADD) {
            p.runtime_error(lineno, "Invalid binary operation for tuple");
            return Value();
        }
    }

    switch (type) {
        case ADD:
            if (left.type == Value::TUPLE && right.type == Value::TUPLE) {
                std::vector<Value> vals{*left.tuple};
                for (Value& v: *right.tuple) {
                    vals.push_back(v);
                }
                return Value(p.add_tuple(std::move(vals)));
            }
            if (!left.numeric() || !right.numeric()) {
                p.runtime_error(lineno, "Addition of non-numeric type");
                return Value();
            }
            if (left.type == Value::DOUBLE || right.type == Value::DOUBLE) {
                return Value(dbl(left) + dbl(right));
            }
            return Value(left.i + right.i);
        case SUB:
            if (!left.numeric() || !right.numeric()) {
                p.runtime_error(lineno, "Subtraction of non-numeric type");
                return Value();
            }
            if (left.type == Value::DOUBLE || right.type == Value::DOUBLE) {
                return Value(dbl(left) - dbl(right));
            }
            return Value(left.i - right.i);
        case MUL:
            if (!left.numeric() || !right.numeric()) {
                p.runtime_error(lineno, "Multplication of non-numeric type");
                return Value();
            }
            if (left.type == Value::DOUBLE || right.type == Value::DOUBLE) {
                return Value(dbl(left) * dbl(right));
            }
            return Value(left.i * right.i);
        case DIV:
            if (!left.numeric() || !right.numeric()) {
                p.runtime_error(lineno, "Division of non-numeric type");
                return Value();
            }
            return Value(dbl(left) / dbl(right));
        case IDIV:
            if (!left.numeric() || !right.numeric()) {
                p.runtime_error(lineno, "Division of non-numeric type");
                return Value();
            }
            if (left.type == Value::DOUBLE || right.type == Value::DOUBLE) {
                return Value(static_cast<int64_t>(dbl(left) / dbl(right)));
            }
            return Value(left.i / right.i);
        case MOD:
            if (!(left.type == Value::INT64) || !(right.type == Value::INT64)) {
                p.runtime_error(lineno, "Modulo of non-integer type");
                return Value();
            }
            return Value(left.i % right.i);
        case BITOR:
            if (!(left.type == Value::INT64) || !(right.type == Value::INT64)) {
                p.runtime_error(lineno, "Bitwise or of non-integer type");
                return Value();
            }
            return Value(left.i | right.i);
        case BITAND:
            if (!(left.type == Value::INT64) || !(right.type == Value::INT64)) {
                p.runtime_error(lineno, "Bitwise and of non-integer type");
                return Value();
            }
            return Value(left.i & right.i);
        case BITXOR:
            if (!(left.type == Value::INT64) || !(right.type == Value::INT64)) {
                p.runtime_error(lineno, "Bitwise xor of non-integer type");
                return Value();
            }
            return Value(left.i ^ right.i);
        case GT:
            if (!left.numeric() || !right.numeric()) {
                p.runtime_error(lineno, "Comparison of non-numeric type");
                return Value();
            }
            if (left.type == Value::DOUBLE || right.type == Value::DOUBLE) {
                return Value(dbl(left) > dbl(right));
            }
            return Value(left.i > right.i);
        case LT:
            if (!left.numeric() || !right.numeric()) {
                p.runtime_error(lineno, "Comparison of non-numeric type");
                return Value();
            }
            if (left.type == Value::DOUBLE || right.type == Value::DOUBLE) {
                return Value(dbl(left) < dbl(right));
            }
            return Value(left.i < right.i);
        case GTE:
            if (!left.numeric() || !right.numeric()) {
                p.runtime_error(lineno, "Comparison of non-numeric type");
                return Value();
            }
            if (left.type == Value::DOUBLE || right.type == Value::DOUBLE) {
                return Value(dbl(left) >= dbl(right));
            }
            return Value(left.i > right.i);
        case LTE:
            if (!left.numeric() || !right.numeric()) {
                p.runtime_error(lineno, "Comparison of non-numeric type");
                return Value();
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
            } else if (left.type == Value::DOUBLE || 
                       right.type == Value::DOUBLE) {
                return Value(dbl(left) == dbl(right));
            } else {
                return Value(left.i == right.i);
            }
        case NEQ:
            if (left.type == Value::NONE || right.type == Value::NONE) {
                return Value(left.type != right.type);
            } else if (left.type == Value::BOOL || right.type == Value::BOOL) {
                return Value(right.type != left.type || left.b == right.b);
            } else if (left.type == Value::DOUBLE ||
                       right.type == Value::DOUBLE) {
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


Value UniOp::evaluate(Program& p) const {
    if (type == PAREN) {
        return e->evaluate(p);
    }
    p.status(lineno);
    Value inner = e->evaluate(p);
    if (type == NEGATE) {
        if (!inner.numeric()) {
            p.runtime_error(lineno, "Negation of non-numeric type");
            return Value();
        }
        if (inner.type == Value::DOUBLE) {
            return Value(-inner.d);
        }
        return Value(-inner.i);
    } else {
        if (inner.type != Value::BOOL) {
            p.runtime_error(lineno, "Boolean not of non-boolean type");
            return Value();
        }
        return Value(!inner.b);
    }
}


Value BuiltinCall::evaluate(Program& p) const {
    p.status(lineno);
    if (type == ELEM) {
        if (args.size() != 2) {
            p.runtime_error(lineno, "Wrong number of arguments");
            return Value();
        }
        Value t = args[0]->evaluate(p);
        Value ix = args[1]->evaluate(p);
        if (t.type != Value::TUPLE || !ix.numeric()) {
            p.runtime_error(lineno, "Invalid argument");
        }
        int64_t i;
        if (ix.type == Value::DOUBLE) {
            i = static_cast<int64_t>(ix.d);
            if (static_cast<double>(i) != ix.d) {
                p.runtime_error(lineno, "Non-integer index");
                return Value();
            }
        } else {
            i = ix.i;
        }
        p.remove_ref(ix);
        if (i >= t.tuple->size()) {
            p.runtime_error(lineno, "Index out of bounds");
        }
        Value res = (*t.tuple)[i];
        p.add_ref(res);
        p.remove_ref(t);
        return res;
    } else if (type == TUPLE) {
        std::vector<Value> v;
        for (auto& a: args) {
            v.push_back(a->evaluate(p));
        }
        return Value(p.add_tuple(v));
    } else if (type == LENGTH) {
        if (args.size() != 1) {
            p.runtime_error(lineno, "Wrong number of arguments");
            return Value();
        }
        Value v = args[0]->evaluate(p);
        if (v.type != Value::TUPLE) {
            p.runtime_error(lineno, "len requires tuple");
            return Value();
        }
        int64_t len = v.tuple->size();
        p.remove_ref(v);
        return Value(len);
    } else if (type == PRINT) {
        std::stringstream ss{};
        SDL_Event e;
        e.type = p.EVT_PRINT;
        if (args.size() == 0) {
            ss << "\n";
            e.user.data1 = new std::string{ss.str()};
            SDL_PushEvent(&e);
            return Value();
        }
        Value first = args[0]->evaluate(p);
        first.write(ss);
        p.remove_ref(first);
        for (size_t ix = 1; ix < args.size(); ++ix) {
            ss << ", ";
            Value v = args[ix]->evaluate(p);
            v.write(ss);
            p.remove_ref(v);
        }
        ss << "\n";
        e.user.data1 = new std::string{ss.str()};
        SDL_PushEvent(&e);
        return Value();
    }
    // Unreachable
    return Value();
}

Value FuncCall::evaluate(Program& p) const {
    p.status(lineno);
    Function* f = p.get_function(name_id);

    if (args.size() != f->params.size()) {
        p.runtime_error(lineno, "Wrong number of arguments");
        return Value();
    }

    p.add_scope();
    for (size_t ix = 0; ix < args.size(); ++ix) {
        Value v = args[ix]->evaluate(p);
        p.set_var(f->params[ix], v, true);
        p.remove_ref(v);
    }
    for (Statement* s: f->statements) {
        Statement::Status status = s->evaluate(p);
        if (status == Statement::RETURN) {
            Value v = p.get_return();
            p.remove_scope();
            return v;
        } else if (status != Statement::NEXT) {
            p.runtime_error(lineno, "Invalid placement of break / continue");
            p.remove_scope();
            return Value();
        }
    }
    p.remove_scope();
    return Value();
}


Value VariableExpr::evaluate(Program& p) const {
    p.status(lineno);
    Value v = p.get_var(id);
    p.add_ref(v);
    return v;
}

Statement::Status Assignment::evaluate(Program& p) const {
    p.status(lineno);
    Value v = val->evaluate(p);
    p.set_var(id, v, false);
    p.remove_ref(v);
    return Statement::NEXT;
}


Statement::Status ExpressionStatement::evaluate(Program& p) const {
    Value v = expr->evaluate(p);
    p.remove_ref(v);
    return Statement::NEXT;
}

Statement::Status ReturnStatement::evaluate(Program& p) const {
    p.status(lineno);
    if (expr == nullptr) {
        p.set_return(Value());
    } else {
        Value v = expr->evaluate(p);
        p.set_return(v);
    }
    return Statement::RETURN;
}

Statement::Status FlowStatement::evaluate(Program& p) const {
    p.status(lineno);
    if (is_break) {
        return Statement::BREAK;
    } else {
        return Statement::CONTINUE;
    }
}


Statement::Status IfStatement::evaluate(Program& p) const {
    bool c = true;
    p.status(lineno);
    if (cond != nullptr) {
        Value v = cond->evaluate(p);
        c = v.boolean();
        p.remove_ref(v);
    }
    if (c) {
        for (Statement* s: on_if) {
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


Statement::Status WhileStatement::evaluate(Program& p) const {
    p.status(lineno);
    while (1) {
        Value v = cond->evaluate(p);
        bool c = v.boolean();
        p.remove_ref(v);
        if (!c) {
            break;
        }
        for (Statement* s: statements) {
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


Statement::Status ForStatement::evaluate(Program& p) const {
    p.status(lineno);
    Value v = expr->evaluate(p);
    if (v.type != Value::TUPLE) {
        p.runtime_error(lineno, "For loop requires tuple");
        return Statement::NEXT;
    }

    for (int64_t ix = 0; ix < v.tuple->size(); ++ix) {
        p.set_var(var_id, v.tuple->at(ix), false);
        for (Statement* s : statements) {
            Statement::Status status = s->evaluate(p);
            if (status == Statement::RETURN) {
                p.remove_ref(v);
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


Statement::Status GlobalStatement::evaluate(Program& p) const {
    p.status(lineno);
    for (Statement* s: statements) {
        Statement::Status status = s->evaluate(p);
        if (status != Statement::NEXT) {
            p.runtime_error(lineno, "Illegal statement");
            return Statement::NEXT;
        }
    }
    return Statement::NEXT;
}

Statement::Status FuncDef::evaluate(Program& p) const {
    p.status(lineno);
    p.set_function(name_id, function);

    return Statement::NEXT;
}
