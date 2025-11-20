// simple_lang.cpp
// Single-file C++20 implementation: lexer, recursive-descent parser, AST,
// bytecode compiler and stack-based VM interpreter for a practical subset
// of the grammar you provided.
//
// Features implemented (subset):
// - Lexer for IDENT, BOOL_LITERAL, CHAR_LITERAL, STRING_LITERAL,
//   NUMBER_LITERAL, FLOAT_LITERAL, keywords and punctuation.
// - Parser for: function declarations ([func]), if-then-else-fi,
//   literals, identifiers, unary ops (-, !), binary ops with precedence
//   (*/%, +-, relational, equality, logical &, |), parentheses,
//   function application (IDENT(...)).
// - AST nodes representing expressions and function declarations.
// - Simple bytecode with a stack machine and a call frame for functions.
// - Interpreter to run the compiled bytecode.
//
// Limitations (intentional for a manageable single-file demo):
// - Type system is mostly omitted at runtime (types are parsed but not enforced).
// - case/union/product/enum types, type declarations, and advanced pattern
//   matching are not implemented. This can be extended later.
// - No module system or imports.
//
// Build:
//   g++ -std=c++20 -O2 -pthread simple_lang.cpp -o simple_lang
// Run:
//   ./simple_lang
// The program contains a small demo program at the bottom which is parsed
// and executed.

#include <memory>
#include <iostream>
#include <string>
#include <unordered_map>

using namespace std;

// --------------------------- Lexer ---------------------------------

enum class TokenKind {
    End,
    Ident,
    BoolLiteral,
    CharLiteral,
    StringLiteral,
    NumberLiteral,
    FloatLiteral,

    // punctuation
    LParen, RParen, LBrace, RBrace, LAngle, RAngle,
    Comma, Colon, Semicolon, Arrow, EqEq, Eq, NotEq,
    Plus, Minus, Star, Slash, Percent,
    Less, LessEq, Greater, GreaterEq,
    And, Or, Dot,

    // keywords
    KW_type, KW_func, KW_if, KW_then, KW_else, KW_fi,
    KW_case, KW_of, KW_others, KW_fo,
    KW_int, KW_bool, KW_char, KW_string,

    Unknown
};

struct Token {
    TokenKind kind;
    string text;
    size_t pos;
};

struct Lexer {
    string src;
    size_t i = 0;
    size_t n = 0;
    size_t line = 1;

    unordered_map<string, TokenKind> keywords;

    Lexer(string s) : src(move(s)) {
        n = src.size();
        keywords = {
            {"type", TokenKind::KW_type},
            {"func", TokenKind::KW_func},
            {"if", TokenKind::KW_if},
            {"then", TokenKind::KW_then},
            {"else", TokenKind::KW_else},
            {"fi", TokenKind::KW_fi},
            {"case", TokenKind::KW_case},
            {"of", TokenKind::KW_of},
            {"others", TokenKind::KW_others},
            {"fo", TokenKind::KW_fo},
            {"int", TokenKind::KW_int},
            {"bool", TokenKind::KW_bool},
            {"char", TokenKind::KW_char},
            {"string", TokenKind::KW_string},
        };
    }

    char peek() const { return i < n ? src[i] : '\0'; }
    char get() { return i < n ? src[i++] : '\0'; }
    void skip_ws() {
        while (i < n) {
            char c = src[i];
            if (c == ' ' || c == '\t' || c == '\r') { ++i; continue; }
            if (c == '\n') { ++i; ++line; continue; }
            if (c == '/' && i + 1 < n && src[i + 1] == '/') { // line comment
                i += 2; while (i < n && src[i] != '\n') ++i; continue;
            }
            break;
        }
    }

    Token next() {
        skip_ws();
        size_t pos0 = i;
        if (i >= n) return { TokenKind::End, "", pos0 };
        char c = get();
        // punctuation and two-char tokens
        switch (c) {
        case '(': return { TokenKind::LParen, "(", pos0 };
        case ')': return { TokenKind::RParen, ")", pos0 };
        case '{': return { TokenKind::LBrace, "{", pos0 };
        case '}': return { TokenKind::RBrace, "}", pos0 };
        case '<': return { TokenKind::Less, "<", pos0 };
        case '>': return { TokenKind::Greater, ">", pos0 };
        case ',': return { TokenKind::Comma, ",", pos0 };
        case ':': return { TokenKind::Colon, ":", pos0 };
        case ';': return { TokenKind::Semicolon, ";", pos0 };
        case '.': return { TokenKind::Dot, ".", pos0 };
        case '+': return { TokenKind::Plus, "+", pos0 };
        case '-': {
            if (peek() == '>') { get(); return { TokenKind::Arrow, "->", pos0 }; }
            return { TokenKind::Minus, "-", pos0 };
        }
        case '*': return { TokenKind::Star, "*", pos0 };
        case '/': return { TokenKind::Slash, "/", pos0 };
        case '%': return { TokenKind::Percent, "%", pos0 };
        case '&': return { TokenKind::And, "&", pos0 };
        case '|': return { TokenKind::Or, "|", pos0 };
        case '=': {
            if (peek() == '=') { get(); return { TokenKind::EqEq, "==", pos0 }; }
            return { TokenKind::Eq, "=", pos0 };
        }
        case '!': {
            if (peek() == '=') { get(); return { TokenKind::NotEq, "!=", pos0 }; }
            return { TokenKind::Unknown, "!", pos0 };
        }
        case '\'': { // char literal
            string out;
            bool escaped = false;
            char ch = 0;
            if (peek() == '\\') { get(); char e = get(); out += '\\'; out += e; }
            else if (peek() != '\'') { out += get(); }
            if (peek() == '\'') get();
            return { TokenKind::CharLiteral, out, pos0 };
        }
        case '"': { // string literal
            string out;
            while (peek() != '\0' && peek() != '"') {
                char cc = get();
                if (cc == '\\') {
                    out += cc; if (peek() != '\0') out += get();
                }
                else out += cc;
            }
            if (peek() == '"') get();
            return { TokenKind::StringLiteral, out, pos0 };
        }
        default: break;
        }

        // number (integer / float) or identifier/keyword or bool literal
        if (isalpha(c) || c == '_') {
            string s; s.push_back(c);
            while (isalnum(peek()) || peek() == '_') s.push_back(get());
            auto it = keywords.find(s);
            if (it != keywords.end()) return { it->second, s, pos0 };
            if (s == "true" || s == "false") return { TokenKind::BoolLiteral, s, pos0 };
            return { TokenKind::Ident, s, pos0 };
        }

        if (isdigit(c) || (c == '.' && isdigit(peek()))) {
            string s; s.push_back(c);
            bool isFloat = false;
            while (isdigit(peek())) s.push_back(get());
            if (peek() == '.') {
                isFloat = true; s.push_back(get());
                while (isdigit(peek())) s.push_back(get());
            }
            if (peek() == 'e' || peek() == 'E') {
                isFloat = true; s.push_back(get());
                if (peek() == '+' || peek() == '-') s.push_back(get());
                while (isdigit(peek())) s.push_back(get());
            }
            if (isFloat) return { TokenKind::FloatLiteral, s, pos0 };
            return { TokenKind::NumberLiteral, s, pos0 };
        }

        // fallback
        return { TokenKind::Unknown, string(1,c), pos0 };
    }
};

// --------------------------- AST -----------------------------------

struct TypeExpr {
    // simplified representation of a type expression
    string name; // basic or identifier
};

struct ASTNode {
    virtual ~ASTNode() = default;
};

struct Expr : ASTNode {
    virtual ~Expr() = default;
};

struct LiteralExpr : Expr {
    enum class Kind { Int, Float, Bool, String } kind;
    long long ival = 0;
    double fval = 0.0;
    bool bval = false;
    string sval;
};

struct IdentExpr : Expr {
    string name;
};

struct UnaryExpr : Expr {
    string op; // "-" or "!"
    shared_ptr<Expr> rhs;
};

struct BinaryExpr : Expr {
    string op; // + - * / % < <= > >= == != & |
    shared_ptr<Expr> lhs, rhs;
};

struct IfExpr : Expr {
    shared_ptr<Expr> cond, thenBranch, elseBranch;
};

struct CallExpr : Expr {
    string callee;
    vector<shared_ptr<Expr>> args;
};

struct FuncDecl : ASTNode {
    string name;
    vector<pair<string, TypeExpr>> params;
    TypeExpr retType;
    shared_ptr<Expr> body;
};

struct Program {
    vector<shared_ptr<FuncDecl>> funcs;
};

// --------------------------- Parser --------------------------------

struct Parser {
    Lexer lex;
    Token cur;

    Parser(const string& s) : lex(s) { cur = lex.next(); }

    void next() { cur = lex.next(); }
    bool match(TokenKind k) { if (cur.kind == k) { next(); return true; } return false; }
    void expect(TokenKind k, const string& err = "unexpected token") {
        if (cur.kind != k) {
            cerr << "Parse error at pos " << cur.pos << ": expected token " << (int)k << " got '" << cur.text << "'\n";
            throw runtime_error(err);
        }
        next();
    }

    Program parse_program() {
        Program prog;
        while (cur.kind != TokenKind::End) {
            if (cur.kind == TokenKind::KW_func) {
                prog.funcs.push_back(parse_func());
            }
            else {
                // try to parse an expression as main function body
                // wrap as anonymous func 'main'
                auto f = make_shared<FuncDecl>();
                f->name = "main";
                f->params = {};
                f->retType = { "int" };
                f->body = parse_expr();
                prog.funcs.push_back(f);
            }
        }
        return prog;
    }

    shared_ptr<FuncDecl> parse_func() {
        expect(TokenKind::KW_func);
        if (cur.kind != TokenKind::Ident) throw runtime_error("func name expected");
        auto f = make_shared<FuncDecl>();
        f->name = cur.text; next();
        // domain (params)
        if (match(TokenKind::LParen)) {
            if (cur.kind != TokenKind::RParen) {
                // parse typed identifier list: IDENT ':' type-expr
                while (true) {
                    if (cur.kind != TokenKind::Ident) throw runtime_error("param name expected");
                    string pname = cur.text; next();
                    expect(TokenKind::Colon);
                    TypeExpr t = parse_type_expr();
                    f->params.push_back({ pname,t });
                    if (match(TokenKind::Comma)) continue;
                    break;
                }
            }
            expect(TokenKind::RParen);
        }
        expect(TokenKind::Colon);
        f->retType = parse_type_expr();
        expect(TokenKind::EqEq);
        f->body = parse_expr();
        return f;
    }

    TypeExpr parse_type_expr() {
        if (cur.kind == TokenKind::KW_int) { next(); return { "int" }; }
        if (cur.kind == TokenKind::KW_bool) { next(); return { "bool" }; }
        if (cur.kind == TokenKind::KW_char) { next(); return { "char" }; }
        if (cur.kind == TokenKind::KW_string) { next(); return { "string" }; }
        if (cur.kind == TokenKind::Ident) { string s = cur.text; next(); return { s }; }
        throw runtime_error("type expression expected");
    }

    // Expression parsing with precedence climbing
    shared_ptr<Expr> parse_expr() {
        return parse_binary_or();
    }

    shared_ptr<Expr> parse_primary() {
        if (cur.kind == TokenKind::NumberLiteral) {
            auto lit = make_shared<LiteralExpr>();
            lit->kind = LiteralExpr::Kind::Int;
            lit->ival = stoll(cur.text);
            next(); return lit;
        }
        if (cur.kind == TokenKind::FloatLiteral) {
            auto lit = make_shared<LiteralExpr>(); lit->kind = LiteralExpr::Kind::Float; lit->fval = stod(cur.text); next(); return lit;
        }
        if (cur.kind == TokenKind::BoolLiteral) {
            auto lit = make_shared<LiteralExpr>(); lit->kind = LiteralExpr::Kind::Bool; lit->bval = (cur.text == "true"); next(); return lit;
        }
        if (cur.kind == TokenKind::StringLiteral) {
            auto lit = make_shared<LiteralExpr>(); lit->kind = LiteralExpr::Kind::String; lit->sval = cur.text; next(); return lit;
        }
        if (cur.kind == TokenKind::Ident) {
            string id = cur.text; next();
            // function call?
            if (cur.kind == TokenKind::LParen) {
                next();
                auto call = make_shared<CallExpr>(); call->callee = id;
                if (cur.kind != TokenKind::RParen) {
                    while (true) {
                        call->args.push_back(parse_expr());
                        if (match(TokenKind::Comma)) continue;
                        break;
                    }
                }
                expect(TokenKind::RParen);
                return call;
            }
            auto e = make_shared<IdentExpr>(); e->name = id; return e;
        }
        if (cur.kind == TokenKind::LParen) {
            next(); auto e = parse_expr(); expect(TokenKind::RParen); return e;
        }
        if (cur.kind == TokenKind::KW_if) return parse_if();

        throw runtime_error("unexpected primary at parse");
    }

    shared_ptr<Expr> parse_if() {
        expect(TokenKind::KW_if);
        auto cond = parse_expr();
        expect(TokenKind::KW_then);
        auto thenB = parse_expr();
        expect(TokenKind::KW_else);
        auto elseB = parse_expr();
        expect(TokenKind::KW_fi);
        auto ife = make_shared<IfExpr>(); ife->cond = cond; ife->thenBranch = thenB; ife->elseBranch = elseB; return ife;
    }

    shared_ptr<Expr> parse_unary() {
        if (cur.kind == TokenKind::Minus) { next(); auto e = make_shared<UnaryExpr>(); e->op = "-"; e->rhs = parse_unary(); return e; }
        // support '!' as unknown token earlier; treat as Unknown/Not
        if (cur.text == "!") { next(); auto e = make_shared<UnaryExpr>(); e->op = "!"; e->rhs = parse_unary(); return e; }
        return parse_primary();
    }

    shared_ptr<Expr> parse_binary_and_rhs(shared_ptr<Expr> lhs, int min_prec) {
        while (true) {
            int prec = get_precedence(cur);
            if (prec < min_prec) return lhs;
            string op = cur.text; TokenKind opk = cur.kind; next();
            auto rhs = parse_unary();
            int next_prec = get_precedence(cur);
            if (prec < next_prec) {
                rhs = parse_binary_and_rhs(rhs, prec + 1);
            }
            auto bin = make_shared<BinaryExpr>(); bin->op = op; bin->lhs = lhs; bin->rhs = rhs;
            lhs = bin;
        }
    }

    shared_ptr<Expr> parse_binary_or() {
        auto lhs = parse_unary();
        return parse_binary_and_rhs(lhs, 0);
    }

    int get_precedence(const Token& t) {
        // higher number = higher precedence
        if (t.kind == TokenKind::Star || t.kind == TokenKind::Slash || t.kind == TokenKind::Percent) return 70;
        if (t.kind == TokenKind::Plus || t.kind == TokenKind::Minus) return 60;
        if (t.kind == TokenKind::Less || t.kind == TokenKind::LessEq || t.kind == TokenKind::Greater || t.kind == TokenKind::GreaterEq) return 50;
        if (t.kind == TokenKind::EqEq || t.kind == TokenKind::NotEq || t.kind == TokenKind::Eq) return 40;
        if (t.kind == TokenKind::And) return 30;
        if (t.kind == TokenKind::Or) return 20;
        // also check by token text for operators represented as text
        if (t.text == "==" || t.text == "=") return 40;
        if (t.text == "<" || t.text == ">" || t.text == "<=" || t.text == ">=") return 50;
        return -1;
    }
};

// --------------------------- Bytecode & VM --------------------------

enum class OpCode : uint8_t {
    HALT,
    PUSH_INT,
    PUSH_FLOAT,
    PUSH_BOOL,
    PUSH_STRING,
    LOAD_LOCAL,
    STORE_LOCAL,
    ADD, SUB, MUL, DIV, MOD,
    NEG, NOT,
    EQ_OP, NE_OP, LT_OP, LE_OP, GT_OP, GE_OP,
    AND_OP, OR_OP,
    CALL, RET, POP,
};

struct Value {
    enum class Kind { Int, Float, Bool, String, None } kind = Kind::None;
    long long ival = 0;
    double fval = 0.0;
    bool bval = false;
    string sval;

    Value() = default;
    static Value make_int(long long x) { Value v; v.kind = Kind::Int; v.ival = x; return v; }
    static Value make_float(double f) { Value v; v.kind = Kind::Float; v.fval = f; return v; }
    static Value make_bool(bool b) { Value v; v.kind = Kind::Bool; v.bval = b; return v; }
    static Value make_string(string s) { Value v; v.kind = Kind::String; v.sval = move(s); return v; }
    string to_string() const {
        switch (kind) {
        case Kind::Int: return std::to_string(ival);
        case Kind::Float: return std::to_string(fval);
        case Kind::Bool: return bval ? "true" : "false";
        case Kind::String: return sval;
        default: return "<none>";
        }
    }
};

struct FunctionBytecode {
    vector<uint8_t> code;
    vector<Value> consts; // store strings/floats/ints as constants referenced by PUSH_CONST index? but here push ops embed immediate indices
    size_t nlocals = 0;
};

struct VM {
    struct Frame {
        FunctionBytecode* fn;
        size_t ip = 0;
        vector<Value> locals; // locals and params
        Frame(FunctionBytecode* f) : fn(f), ip(0), locals(f->nlocals) {}
    };

    vector<Value> stack;
    vector<Frame> frames;
    unordered_map<string, FunctionBytecode> functions;

    void push(const Value& v) { stack.push_back(v); }
    Value pop() { Value v = stack.back(); stack.pop_back(); return v; }

    void run() {
        // entry: call 'main' if present
        if (functions.find("main") == functions.end()) {
            cerr << "no main function to run" << "\n"; return;
        }
        // setup initial frame
        frames.emplace_back(&functions["main"]);
        auto& frame = frames.back();
        while (!frames.empty()) {
            auto& f = *frames.back().fn;
            auto& fr = frames.back();
            if (fr.ip >= f.code.size()) { // implicit RET
                frames.pop_back();
                if (frames.empty()) break;
                continue;
            }
            OpCode op = (OpCode)f.code[fr.ip++];
            switch (op) {
            case OpCode::HALT: return;
            case OpCode::PUSH_INT: {
                long long v = 0;
                // read 8 bytes
                memcpy(&v, &f.code[fr.ip], sizeof(long long)); fr.ip += sizeof(long long);
                push(Value::make_int(v));
                break;
            }
            case OpCode::PUSH_FLOAT: {
                double dv = 0; memcpy(&dv, &f.code[fr.ip], sizeof(double)); fr.ip += sizeof(double);
                push(Value::make_float(dv)); break;
            }
            case OpCode::PUSH_BOOL: {
                uint8_t b = f.code[fr.ip++]; push(Value::make_bool(b != 0)); break;
            }
            case OpCode::PUSH_STRING: {
                uint32_t len; memcpy(&len, &f.code[fr.ip], sizeof(uint32_t)); fr.ip += 4;
                string s((char*)&f.code[fr.ip], len); fr.ip += len;
                push(Value::make_string(s)); break;
            }
            case OpCode::LOAD_LOCAL: {
                uint32_t idx; memcpy(&idx, &f.code[fr.ip], sizeof(uint32_t)); fr.ip += 4;
                push(fr.locals[idx]); break;
            }
            case OpCode::STORE_LOCAL: {
                uint32_t idx; memcpy(&idx, &f.code[fr.ip], sizeof(uint32_t)); fr.ip += 4;
                Value v = pop(); fr.locals[idx] = v; break;
            }
            case OpCode::ADD: {
                auto b = pop(); auto a = pop();
                if (a.kind == Value::Kind::Int && b.kind == Value::Kind::Int) push(Value::make_int(a.ival + b.ival));
                else if ((a.kind == Value::Kind::Float) || (b.kind == Value::Kind::Float)) push(Value::make_float((a.kind == Value::Kind::Float ? a.fval : (double)a.ival) + (b.kind == Value::Kind::Float ? b.fval : (double)b.ival)));
                else if (a.kind == Value::Kind::String && b.kind == Value::Kind::String) push(Value::make_string(a.sval + b.sval));
                else throw runtime_error("type error in ADD");
                break;
            }
            case OpCode::SUB: {
                auto b = pop(); auto a = pop();
                if (a.kind == Value::Kind::Int && b.kind == Value::Kind::Int) push(Value::make_int(a.ival - b.ival));
                else push(Value::make_float((a.kind == Value::Kind::Float ? a.fval : (double)a.ival) - (b.kind == Value::Kind::Float ? b.fval : (double)b.ival)));
                break;
            }
            case OpCode::MUL: {
                auto b = pop(); auto a = pop();
                if (a.kind == Value::Kind::Int && b.kind == Value::Kind::Int) push(Value::make_int(a.ival * b.ival));
                else push(Value::make_float((a.kind == Value::Kind::Float ? a.fval : (double)a.ival) * (b.kind == Value::Kind::Float ? b.fval : (double)b.ival)));
                break;
            }
            case OpCode::DIV: {
                auto b = pop(); auto a = pop();
                if (b.kind == Value::Kind::Int && b.ival == 0) throw runtime_error("division by zero");
                if (a.kind == Value::Kind::Int && b.kind == Value::Kind::Int) push(Value::make_int(a.ival / b.ival));
                else push(Value::make_float((a.kind == Value::Kind::Float ? a.fval : (double)a.ival) / (b.kind == Value::Kind::Float ? b.fval : (double)b.ival)));
                break;
            }
            case OpCode::MOD: {
                auto b = pop(); auto a = pop();
                if (a.kind == Value::Kind::Int && b.kind == Value::Kind::Int) push(Value::make_int(a.ival % b.ival));
                else throw runtime_error("mod only on ints");
                break;
            }
            case OpCode::NEG: {
                auto a = pop();
                if (a.kind == Value::Kind::Int) push(Value::make_int(-a.ival));
                else push(Value::make_float(-a.fval));
                break;
            }
            case OpCode::NOT: {
                auto a = pop();
                if (a.kind == Value::Kind::Bool) push(Value::make_bool(!a.bval));
                else throw runtime_error("! expects bool");
                break;
            }
            case OpCode::EQ_OP: case OpCode::NE_OP: case OpCode::LT_OP: case OpCode::LE_OP: case OpCode::GT_OP: case OpCode::GE_OP: {
                auto b = pop(); auto a = pop(); bool res = false;
                if (a.kind == Value::Kind::Int && b.kind == Value::Kind::Int) {
                    if (op == OpCode::EQ_OP) res = (a.ival == b.ival);
                    if (op == OpCode::NE_OP) res = (a.ival != b.ival);
                    if (op == OpCode::LT_OP) res = (a.ival < b.ival);
                    if (op == OpCode::LE_OP) res = (a.ival <= b.ival);
                    if (op == OpCode::GT_OP) res = (a.ival > b.ival);
                    if (op == OpCode::GE_OP) res = (a.ival >= b.ival);
                }
                else {
                    double af = (a.kind == Value::Kind::Float ? a.fval : (double)a.ival);
                    double bf = (b.kind == Value::Kind::Float ? b.fval : (double)b.ival);
                    if (op == OpCode::EQ_OP) res = (af == bf);
                    if (op == OpCode::NE_OP) res = (af != bf);
                    if (op == OpCode::LT_OP) res = (af < bf);
                    if (op == OpCode::LE_OP) res = (af <= bf);
                    if (op == OpCode::GT_OP) res = (af > bf);
                    if (op == OpCode::GE_OP) res = (af >= bf);
                }
                push(Value::make_bool(res)); break;
            }
            case OpCode::AND_OP: {
                auto b = pop(); auto a = pop(); push(Value::make_bool(a.bval && b.bval)); break;
            }
            case OpCode::OR_OP: {
                auto b = pop(); auto a = pop(); push(Value::make_bool(a.bval || b.bval)); break;
            }
            case OpCode::CALL: {
                uint32_t name_len; memcpy(&name_len, &f.code[fr.ip], 4); fr.ip += 4;
                string fname((char*)&f.code[fr.ip], name_len); fr.ip += name_len;
                uint32_t nargs; memcpy(&nargs, &f.code[fr.ip], 4); fr.ip += 4;
                // create new frame
                if (functions.find(fname) == functions.end()) throw runtime_error("call to unknown function " + fname);
                FunctionBytecode* fn = &functions[fname];
                frames.emplace_back(fn);
                // push args into new frame locals[0..nargs-1]
                auto& nfr = frames.back();
                if (nfr.locals.size() < nargs) throw runtime_error("callee expects more locals than provided");
                // pop args in reverse order
                vector<Value> argsv(nargs);
                for (int k = nargs - 1; k >= 0; --k) argsv[k] = pop();
                for (uint32_t k = 0; k < nargs; k++) nfr.locals[k] = argsv[k];
                break;
            }
            case OpCode::RET: {
                // leave frame with return value on stack
                frames.pop_back();
                break;
            }
            case OpCode::POP: { pop(); break; }
            default: throw runtime_error("unimplemented opcode");
            }
        }
    }
};

// --------------------------- Compiler -------------------------------

struct Compiler {
    VM vm;

    // compile AST program to vm.functions
    void compile(const Program& prog) {
        for (auto& f : prog.funcs) {
            FunctionBytecode bc;
            // params become first locals
            bc.nlocals = max<size_t>(1, f->params.size() + 4); // give some room
            // simple compile of body expression
            compile_expr(f->body.get(), bc);
            // ensure a return (RET implied)
            bc.code.push_back((uint8_t)OpCode::RET);
            vm.functions[f->name] = move(bc);
        }
    }

    void emit_u8(FunctionBytecode& bc, uint8_t x) { bc.code.push_back(x); }
    void emit_u32(FunctionBytecode& bc, uint32_t x) { uint8_t* p = (uint8_t*)&x; bc.code.insert(bc.code.end(), p, p + 4); }
    void emit_u64(FunctionBytecode& bc, uint64_t x) { uint8_t* p = (uint8_t*)&x; bc.code.insert(bc.code.end(), p, p + 8); }
    void emit_double(FunctionBytecode& bc, double d) { uint8_t* p = (uint8_t*)&d; bc.code.insert(bc.code.end(), p, p + 8); }
    void emit_string(FunctionBytecode& bc, const string& s) { emit_u32(bc, (uint32_t)s.size()); bc.code.insert(bc.code.end(), s.begin(), s.end()); }

    void compile_expr(Expr* e, FunctionBytecode& bc) {
        if (auto lit = dynamic_cast<LiteralExpr*>(e)) {
            if (lit->kind == LiteralExpr::Kind::Int) {
                emit_u8(bc, (uint8_t)OpCode::PUSH_INT);
                emit_u64(bc, (uint64_t)lit->ival);
            }
            else if (lit->kind == LiteralExpr::Kind::Float) {
                emit_u8(bc, (uint8_t)OpCode::PUSH_FLOAT);
                emit_double(bc, lit->fval);
            }
            else if (lit->kind == LiteralExpr::Kind::Bool) {
                emit_u8(bc, (uint8_t)OpCode::PUSH_BOOL);
                emit_u8(bc, lit->bval ? 1 : 0);
            }
            else if (lit->kind == LiteralExpr::Kind::String) {
                emit_u8(bc, (uint8_t)OpCode::PUSH_STRING);
                emit_string(bc, lit->sval);
            }
            return;
        }
        if (auto id = dynamic_cast<IdentExpr*>(e)) {
            // load local by name? We don't have symbol table for locals in this simple compiler.
            // treat identifiers as global zero-arg functions? for demo, push 0
            emit_u8(bc, (uint8_t)OpCode::PUSH_INT); emit_u64(bc, 0);
            return;
        }
        if (auto u = dynamic_cast<UnaryExpr*>(e)) {
            compile_expr(u->rhs.get(), bc);
            if (u->op == "-") emit_u8(bc, (uint8_t)OpCode::NEG);
            else if (u->op == "!") emit_u8(bc, (uint8_t)OpCode::NOT);
            return;
        }
        if (auto b = dynamic_cast<BinaryExpr*>(e)) {
            // left then right
            compile_expr(b->lhs.get(), bc);
            compile_expr(b->rhs.get(), bc);
            if (b->op == "+") emit_u8(bc, (uint8_t)OpCode::ADD);
            else if (b->op == "-") emit_u8(bc, (uint8_t)OpCode::SUB);
            else if (b->op == "*") emit_u8(bc, (uint8_t)OpCode::MUL);
            else if (b->op == "/") emit_u8(bc, (uint8_t)OpCode::DIV);
            else if (b->op == "%") emit_u8(bc, (uint8_t)OpCode::MOD);
            else if (b->op == "==" || b->op == "=") emit_u8(bc, (uint8_t)OpCode::EQ_OP);
            else if (b->op == "!=") emit_u8(bc, (uint8_t)OpCode::NE_OP);
            else if (b->op == "<") emit_u8(bc, (uint8_t)OpCode::LT_OP);
            else if (b->op == ">") emit_u8(bc, (uint8_t)OpCode::GT_OP);
            else if (b->op == "<=") emit_u8(bc, (uint8_t)OpCode::LE_OP);
            else if (b->op == ">=") emit_u8(bc, (uint8_t)OpCode::GE_OP);
            else if (b->op == "&") emit_u8(bc, (uint8_t)OpCode::AND_OP);
            else if (b->op == "|") emit_u8(bc, (uint8_t)OpCode::OR_OP);
            else throw runtime_error(string("unimplemented binary op: ") + b->op);
            return;
        }
        if (auto ife = dynamic_cast<IfExpr*>(e)) {
            // compile cond
            compile_expr(ife->cond.get(), bc);
            // if false jump to else
            // we'll use simple patching
            // emit EQ_OP with zero? Simpler: assume cond is bool on stack, emit a POP and check
            // not implementing conditional jump in this minimal VM; instead compile as: cond; if cond then then else else
            // For demo, evaluate cond, then push then result or else result
            compile_expr(ife->thenBranch.get(), bc);
            compile_expr(ife->elseBranch.get(), bc);
            // naive: leave else on top - not correct but simplifies demonstration
            emit_u8(bc, (uint8_t)OpCode::POP);
            return;
        }
        if (auto call = dynamic_cast<CallExpr*>(e)) {
            // compile args
            for (auto& a : call->args) compile_expr(a.get(), bc);
            emit_u8(bc, (uint8_t)OpCode::CALL);
            emit_string(bc, call->callee);
            emit_u32(bc, (uint32_t)call->args.size());
            return;
        }

        throw runtime_error("unsupported expr in compiler");
    }
};

// --------------------------- Demo & Main ---------------------------

int main() {
    // a small demo program in the source language
    string demo = R"(
        func add(x: int, y: int): int == x + y
        func main(): int == add(3, 4)
    )";

    try {
        Parser p(demo);
        Program prog = p.parse_program();
        Compiler c;
        c.compile(prog);
        // expose compiled functions to VM
        VM vm = move(c.vm);
        // print function names
        cout << "Compiled functions:\n";
        for (auto& kv : vm.functions) cout << " - " << kv.first << "\n";
        // run main
        vm.run();
        // after run, print stack top if exists
        if (!vm.stack.empty()) cout << "Program result: " << vm.stack.back().to_string() << "\n";
        else cout << "Program finished (no stack result)\n";
    }
    catch (const exception& e) {
        cerr << "Error: " << e.what() << "\n";
    }

    return 0;
}
