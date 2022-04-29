#include <ast.h>

int BaseAST::id = 0;
int BaseAST::elf_id = 0;
int BaseAST::block_id = 0;
bool tmp_defined = false;

std::string GenVar(int num) {
    return "%" + std::to_string(num);
}

std::string CompUnitAST::GenIR(BlockInfo* b) {
    printf("decl @getint(): i32\n");
    printf("decl @getch(): i32\n");
    printf("decl @getarray(*i32): i32\n");
    printf("decl @putint(i32)\n");
    printf("decl @putch(i32)\n");
    printf("decl @putarray(i32, *i32)\n");
    printf("decl @starttime()\n");
    printf("decl @stoptime()\n\n");
    b->insert("getint", "getint", "int_func");
    b->insert("getch", "getch", "int_func");
    b->insert("getarray", "getarray", "int_func");
    b->insert("putint", "putint", "void_func");
    b->insert("putch", "putch", "void_func");
    b->insert("putarray", "putarray", "void_func");
    b->insert("starttime", "starttime", "void_func");
    b->insert("stoptime", "stoptime", "void_func");

    int n = son.size();
    for(int i = 0; i < n; ++i)
        son[i]->GenIR(b);
    return "";
}

std::string FuncDefAST::GenIR(BlockInfo* b) {
    std::string ret = "", type = func_type->GenIR(b);
    ret += "fun @" + ident + "(";
    if(son.size() > 0)
        ret += son[0]->GenIR(b);
    ret += ")";
    ret += type + " {\n";
    printf("%s", ret.c_str());
    if(!b->fa) printf("%%entry:\n");

    // alloc params for function
    BlockInfo* blk = new BlockInfo(++elf_id, b);
    blk->created_by_fa = true;
    if(son.size() > 0) {
        vector<std::string>params = son[0]->getson(b);
        for(int i = 0, n = params.size(); i < n; ++i) { 
            int pos = params[i].find(':', 0);
            std::string pname = params[i].substr(1, pos - 1);
            std::string ptype = params[i].substr(pos + 2);
            std::string value = "%" + pname + "_" + blk->id;
            auto name = value.c_str();
            printf("  %s = alloc %s\n", name, ptype.c_str());
            printf("  store @%s, %s\n", pname.c_str(), name);
            if(ptype == "i32") blk->insert(pname, value, "int_var");
            else {
                int dim = 1;
                for(int j = 0; j < ptype.length(); ++j)
                    dim += (ptype[j] == '[');
                blk->insert(pname, value, "int_ptr", dim);
            }
        }
    }

    // Insert function into global table
    b->insert(ident, ident, type == ": i32"? "int_func": "void_func");
    if(block->GenIR(blk) != "ret")
        printf("  ret\n");
    id = 0;
    printf("}\n");
    return "";
}

std::string FuncTypeAST::GenIR(BlockInfo* b) {
    if (type == "int")
        return ": i32";
    else if(type == "void")
        return " ";
    assert(false);
}

std::string FuncFParamsAST::GenIR(BlockInfo* b) {
    std::string ret = "";
    for(int i = 0, n = son.size(); i < n; ++i) {
        if(i > 0) ret += ", ";
        ret += son[i]->GenIR(b);
    }
    return ret;
}

vector<std::string> FuncFParamsAST::getson(BlockInfo* b) const {
    vector<std::string> ret;
    for(int i = 0, n = son.size(); i < n; ++i)
        ret.push_back(son[i]->GenIR(b));
    return ret;
}

std::string FuncFParamAST::GenIR(BlockInfo* b) {
    if(state == 1)
        return "@" + ident + ": i32";
    return "@" + ident + ": *" + son[0]->GenIR(b);
}

std::string FuncRParamsAST::GenIR(BlockInfo* b) {
    vector<std::string> params;
    int n = son.size();
    for(int i = 0; i < n; ++i)
        params.push_back(son[i]->GenIR(b));
    std::string ret = "";
    for(int i = 0; i < son.size(); ++i) {
        if(i > 0) ret += ", ";
        ret += params[i];
    }
    return ret;
}

std::string BlockAST::GenIR(BlockInfo* b) {
    if(b->created_by_fa) {
        b->created_by_fa = false;
        return item->GenIR(b);
    }
    BlockInfo* blk = new BlockInfo(++elf_id, b);
    return item->GenIR(blk);
}

std::string BlockItemStarAST::GenIR(BlockInfo* b) {
    int n = son.size();
    for(int i = 0; i < n; ++i)
        if(son[i]->GenIR(b) == "ret")
            return "ret";
    return "";
}

std::string BlockItemAST::GenIR(BlockInfo* b) {
    return item->GenIR(b);
}

std::string StmtAST::GenIR(BlockInfo* b) {
    if (state == 1) {
        std::string res = exp->GenIR(b);
        printf("  ret %s\n", res.c_str());
        return "ret";
    }
    else if(state == 2) {
        std::string var = son[0]->GenIR(b);
        std::string res = exp->GenIR(b);
        printf("  store %s, %s\n", res.c_str(), var.c_str());
    }
    else if(state == 4)
        return exp->GenIR(b);
    else if(state == 5) {
        printf("  ret\n");
        return "ret";
    }
    else if(state == 6)
        return exp->GenIR(b);
    else if(state == 7) {
        // IF '(' Exp ')' Stmt
        int block_if = block_id, block_out = block_id + 1;
        block_id += 2;
        std::string res = exp->GenIR(b);
        printf("  br %s, %%blk%d, %%blk%d\n", res.c_str(), block_if, block_out);
        printf("%%blk%d:\n", block_if);

        res = son[0]->GenIR(b);
        if(res != "ret")
            printf("  jump %%blk%d\n", block_out);
        printf("%%blk%d:\n", block_out);
    }
    else if (state == 8) {
        // IF '(' Exp ')' Stmt ELSE Stmt
        int block_if = block_id, block_else = block_id + 1, block_out = block_id + 2;
        block_id += 3;
        std::string res = exp->GenIR(b);
        printf("  br %s, %%blk%d, %%blk%d\n", res.c_str(), block_if, block_else);
        printf("%%blk%d:\n", block_if);
        std::string res0 = son[0]->GenIR(b);
        if(res0 != "ret") printf("  jump %%blk%d\n", block_out);
        printf("%%blk%d:\n", block_else);

        std::string res1 = son[1]->GenIR(b);
        if(res1 != "ret") printf("  jump %%blk%d\n", block_out);
        printf("%%blk%d:\n", block_out);
    }
    else if (state == 9) {
        // WHILE '(' Exp ')' Stmt
        int block_entry = block_id, block_body = block_id + 1, block_out = block_id + 2;
        block_id += 3;
        printf("  jump %%blk%d\n", block_entry);
        printf("%%blk%d:\n", block_entry);
        b->block_entry.push(block_entry), b->block_out.push(block_out);
        std::string res = exp->GenIR(b);
        printf("  br %s, %%blk%d, %%blk%d\n", res.c_str(), block_body, block_out);
        printf("%%blk%d:\n", block_body);
        res = son[0]->GenIR(b);
        if(res != "ret") printf("  jump %%blk%d\n", block_entry);
        printf("%%blk%d:\n", block_out);
        b->block_entry.pop(), b->block_out.pop();
    }
    else if (state == 10) {
        // BREAK;
        printf("  jump %%blk%d\n", b->block_out.top());
        printf("%%blk%d:\n", block_id++);
    }
    else if (state == 11) {
        // CONTINUE;
        printf("  jump %%blk%d\n", b->block_entry.top());
        printf("%%blk%d:\n", block_id++);
    }
    return "";
}

std::string ExpAST::GenIR(BlockInfo* b) {
    return tuple_exp->GenIR(b);
}

std::string PrimaryExpAST::GenIR(BlockInfo* b) {
    if (state == 1)
        return item->GenIR(b);
    if (state == 2)
        return var;
    std::string ret = son[0]->GenIR(b), ident = son[0]->ident;
    std::string type = b->qtype(ident);
    int n = son[0]->son.size(), dim = b->qdim(ident);

    if((type == "int_ptr" && n == 0) || type == "int_const")  
        return ret;

    std::string reg = GenVar(id++);
    if(dim == n) printf("  %s = load %s\n", reg.c_str(), ret.c_str());
    else printf("  %s = getelemptr %s, 0\n", reg.c_str(), ret.c_str());
    return reg;
}

std::string UnaryExpAST::GenIR(BlockInfo* b) {
    if(state == 1)
        return primary_exp->GenIR(b);
    if(state == 2) {
        std::string ret = unary_exp->GenIR(b), inst = "";
        if(op == "!") {
            inst += "  %" + std::to_string(id) + " = ";
            inst += "eq " + ret + ", 0" + "\n";
        }
        else if(op == "-") {
            inst += "  %" + std::to_string(id) + " = ";
            inst += "sub 0, " + ret + "\n";
        } 
        else if (op == "+")
            return ret;
        printf("%s", inst.c_str());
        return GenVar(id++);
    }
    std::string ret = "", params = "";
    if(state == 4) 
        params = unary_exp->GenIR(b);
    if(b->qtype(op) == "int_func") {
        ret = GenVar(id++);
        printf("  %s = ", ret.c_str());
    }
    else printf("  ");
    printf("call @%s(%s)\n", op.c_str(), params.c_str());
    return ret;
}

std::string AndOrAST::GenIR(BlockInfo* b) {
    if(state == 1)
        return dst->GenIR(b);

    int exp_true = block_id++, exp_false = block_id++;

    std::string ls = src->GenIR(b);
    printf("  %s = ne %s, 0\n", GenVar(id++).c_str(), ls.c_str());
    if(!tmp_defined)
        printf("  @__tmpres = alloc i32\n"), tmp_defined = true;
    printf("  store %%%d, @__tmpres\n", id - 1);
    ls = GenVar(id - 1);
    if(op == "||") {
        printf("  br %s, %%blk%d, %%blk%d\n", ls.c_str(), exp_true, exp_false);
    }
    if(op == "&&") {
        printf("  %%%d = eq %s, 0\n", id++, ls.c_str());
        printf("  br %%%d, %%blk%d, %%blk%d\n", id - 1, exp_true, exp_false);
    }

    printf("%%blk%d:\n", exp_false);
    std::string rs = dst->GenIR(b);
    printf("  %s = ne %s, 0\n", GenVar(id++).c_str(), rs.c_str());
    rs = GenVar(id - 1);
    printf("  %s", GenVar(id++).c_str());
    if (op == "&&")
        printf(" = and ");
    else if (op == "||")
        printf(" = or ");

    printf("%s, %s\n", ls.c_str(), rs.c_str());
    printf("  store %%%d, @__tmpres\n", id - 1);
    printf("  jump %%blk%d\n", exp_true);
    printf("%%blk%d:\n", exp_true);
    printf("  %s = load @__tmpres\n", GenVar(id++).c_str());
    return GenVar(id - 1);
}

int AndOrAST::calc(BlockInfo* blk) const {
    if(state == 1)
        return dst->calc(blk);
    int a = src->calc(blk);
    int b = dst->calc(blk);
    if (op == "&&") return a && b;
    if (op == "||") return a || b;
    else assert(false);
}

std::string TupleExpAST::GenIR(BlockInfo* b) {
    if(state == 1)
        return dst->GenIR(b);
    std::string ls = src->GenIR(b);
    std::string rs = dst->GenIR(b);

    std::string inst = "  " + GenVar(id++);
    if(op == "*")
        inst += " = mul ";
    else if (op == "/")
        inst += " = div ";
    else if (op == "%")
        inst += " = mod ";
    else if(op == "+")
        inst += " = add ";
    else if (op == "-")
        inst += " = sub ";
    else if (op == "==")
        inst += " = eq ";
    else if (op == "!=")
        inst += " = ne ";
    else if (op == ">")
        inst += " = gt ";
    else if (op == ">=")
        inst += " = ge ";
    else if (op == "<")
        inst += " = lt ";
    else if (op == "<=")
        inst += " = le ";
    else 
        assert(false);

    inst += ls + ", " + rs + "\n";
    printf("%s", inst.c_str());
    return GenVar(id - 1);
}

std::string DeclAST::GenIR(BlockInfo* b) {
    return sub_decl->GenIR(b);
}

std::string ConstDefStarAST::GenIR(BlockInfo* b) {
    int n = son.size();
    for(int i = 0; i < n; ++i)
        son[i]->GenIR(b);
    return "";
}

std::string VarDefStarAST::GenIR(BlockInfo* b) {
    int n = son.size();
    for(int i = 0; i < n; ++i)
        son[i]->GenIR(b);
    return "";
}

std::string DefAST::GenAggragate(int l, int r, int dep) {
    if(l == r) return items[l];
    int tot = 1;
    for(int i = dep + 1; i < shape.size(); ++i)
        tot *= shape[i];
    std::string ret = "";
    for(int i = l; i + tot - 1 <= r; i += tot) {
        if(i > l) ret += ", ";
        ret += GenAggragate(i, i + tot - 1, dep + 1);
    }
    return "{" + ret + "}";
}

void DefAST::ArrayInit(int l, int r, int dep, std::string last) {
    if(l == r) {
        printf("  store %s, %s\n", items[l].c_str(), last.c_str());
        return ;
    }
    int tot = 1;
    for(int i = dep + 1; i < shape.size(); ++i)
        tot *= shape[i];

    for(int i = l, k = 0; i + tot - 1 <= r; i += tot, ++k) {
        std::string reg = GenVar(id++);
        printf("  %s = getelemptr %s, %d\n", reg.c_str(), last.c_str(), k);
        ArrayInit(i, i + tot - 1, dep + 1, reg);
    }
}

std::string ConstDefAST::GenIR(BlockInfo* b) {
    assert(b->table.find(var) == b->table.end());
    std::string type = son[0]->GenIR(b);
    std::string name = "@" + var + "_" + b->id;
    if(type == "i32") {
        b->insert(var, exp->GenIR(b, 0, 0), "int_const");
        return "";
    }

    for(int i = 0, n = son[0]->son.size(); i < n; ++i)
        shape.push_back(son[0]->son[i]->calc(b));
    
    b->insert(var, name, "int_array", shape.size());
    exp->shape = shape;
    exp->GenIR(b, 0, 0);
    items = exp->items;
    std::string res = GenAggragate(0, items.size() - 1, 0);
    if(!b->fa)
        printf("global %s = alloc %s, %s\n", name.c_str(), type.c_str(), res.c_str());
    else {
        printf("  %s = alloc %s\n", name.c_str(), type.c_str());
        ArrayInit(0, items.size() - 1, 0, name);
    }
    return "";
}

std::string VarDefAST::GenIR(BlockInfo* b) {
    assert(b->table.find(var) == b->table.end());
    std::string type = son[0]->GenIR(b), res;
    std::string value = "@" + var + "_" + b->id;
    for(int i = 0, n = son[0]->son.size(); i < n; ++i)
        shape.push_back(son[0]->son[i]->calc(b));
    if(state == 1) exp->shape = shape;
    if(type == "i32") b->insert(var, value, "int_var");
    else b->insert(var, value, "int_array", shape.size());

    if(b->fa) {
        printf("  %s = alloc %s\n", value.c_str(), type.c_str());
        if(state == 1) {
            if(type == "i32") {
                res = exp->GenIR(b, 0, 1);
                printf("  store %s, %s\n", res.c_str(), value.c_str());
            }
            else {
                exp->GenIR(b, 0, 1);
                items = exp->items;
                res = GenAggragate(0, items.size() - 1, 0);
                ArrayInit(0, items.size() - 1, 0, value);
            }
        }
        return "";
    }
    res = "zeroinit";
    if(state == 1) {
        if(type != "i32") {
            exp->GenIR(b, 0, 0);
            items = exp->items;
            res = GenAggragate(0, items.size() - 1, 0);
        }
        else res = exp->GenIR(b, 0, 0);
    }
    printf("global %s = alloc %s, %s\n", value.c_str(), type.c_str(), res.c_str());
    return "";
}

int ExpAST::calc(BlockInfo* b) const {
    return tuple_exp->calc(b);
}

int PrimaryExpAST::calc(BlockInfo* b) const {
    if(state == 1) return item->calc(b);
    std::string num = (state == 2? var: son[0]->GenIR(b));
    int ret = 0;
    for(int i = 0, n = num.length(); i < n; ++i)
        ret = ret * 10 + num[i] - 48;
    return ret;
}

int UnaryExpAST::calc(BlockInfo* b) const {
    if(state == 1)
        return primary_exp->calc(b);
    int val = unary_exp->calc(b);
    if(op == "-") return -val;
    if(op == "!") return !val;
    return val;
}

int TupleExpAST::calc(BlockInfo* blk) const {
    if(state == 1)
        return dst->calc(blk);
    int a = src->calc(blk);
    int b = dst->calc(blk);
    if (op == "*") return a * b;
    if (op == "/") return a / b;
    if (op == "%") return a % b;
    if (op == "+") return a + b;
    if (op == "-") return a - b;
    if (op == "==") return a == b;
    if (op == "!=") return a != b;
    if (op == ">") return a > b;
    if (op == ">=") return a >= b;
    if (op == "<") return a < b;
    if (op == "<=") return a <= b;
    else assert(false);
}

std::string LValAST::GenIR(BlockInfo* b) {
    std::string last = b->query(ident), type = b->qtype(ident);
    if(type == "int_const" || type == "int_var")
        return last;

    int i = 0, n = son.size();
    if(type == "int_ptr") {
        std::string index = "0", tmp = GenVar(id++);
        printf("  %s = load %s\n", tmp.c_str(), last.c_str());
        last = GenVar(id++);
        if(n > 0) i = 1, index = son[0]->GenIR(b);
        printf("  %s = getptr %s, %s\n", last.c_str(), tmp.c_str(), index.c_str());
    }
    for(; i < n; ++i) {
        std::string now = GenVar(id++), index = son[i]->GenIR(b);
        printf("  %s = getelemptr %s, %s\n", now.c_str(), last.c_str(), index.c_str());
        last = now;
    }
    return last;
}

std::string InitValStarAST::GenIR(BlockInfo* b, int dep, int op) {
    std::string res, ret = "";
    int tot = 1, m = shape.size();
    for(int i = dep; i < m; ++i)
        tot *= shape[i];

    int next_dep = -1;
    for(int i = 0, n = son.size(); i < n; ++i) {
        if(son[i]->state == 1) {
            res = son[i]->GenIR(b, next_dep, op);
            items.push_back(res);
        }
        else {
            son[i]->shape = shape;
            if(next_dep == -1) {
                int tmp = tot / shape[dep], sz = items.size();
                for(int j = dep + 1; j < m; ++j) {
                    if(sz % tmp == 0) {
                        next_dep = j;
                        break;
                    }
                    tmp /= shape[j];
                }
            }
            assert(next_dep != -1);
            res = son[i]->GenIR(b, next_dep, op);
            for(int j = 0; j < son[i]->items.size(); ++j)
                items.push_back(son[i]->items[j]);
        }
    }
    while(items.size() < tot)
        items.push_back("0");
    return "";
}

std::string InitValAST::GenIR(BlockInfo* b, int dep, int op) {
    if(state == 1)
        return op? son[0]->GenIR(b): to_string(son[0]->calc(b));
    if(state == 2) {
        int tot = 1, m = shape.size();
        for(int i = dep; i < m; ++i)
            tot *= shape[i];
        while(items.size() < tot)
            items.push_back("0");
        return "{}";
    }
    if(state == 3) {
        son[0]->shape = shape;
        son[0]->GenIR(b, dep, op);
        items = son[0]->items;
        return "";
    }
    assert(false);
}

std::string ConstExpAST::GenArrayDef(BlockInfo* b, int dep) const {
    if(dep >= son.size())
        return "i32";
    return "[" + GenArrayDef(b, dep + 1) + ", " + to_string(son[dep]->calc(b)) + "]";
}

std::string ConstExpAST::GenIR(BlockInfo* b) {
    return GenArrayDef(b, 0);
}
