#include "StmtInterpreter.h"
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

static bool astNeedsInterpreter(const ASTNode* n) {
    if (!n) return false;
    switch (n->type) {
        case NodeType::CallNode:
            return true;
        case NodeType::BinaryOpNode: {
            auto* b = static_cast<const BinaryOpNode*>(n);
            char op = b->getOp();
            if (op == '%' || op == '&' || op == '|' || op == 'X' || op == 'S' || op == 'T' || op == 'A' || op == 'O' ||
                op == 'P' || op == 'Q')
                return true;
            return astNeedsInterpreter(b->getLeft().get()) || astNeedsInterpreter(b->getRight().get());
        }
        case NodeType::UnaryOpNode: {
            auto* u = static_cast<const UnaryOpNode*>(n);
            if (u->getOp() == '!' || u->getOp() == '~') return true;
            return astNeedsInterpreter(u->getOperand().get());
        }
        case NodeType::AssignmentNode: {
            auto* a = static_cast<const AssignmentNode*>(n);
            return astNeedsInterpreter(a->getExpression().get());
        }
        case NodeType::CastNode: {
            auto* c = static_cast<const CastNode*>(n);
            return astNeedsInterpreter(c->getInner());
        }
        case NodeType::TernaryNode:
            return true;
        default:
            return false;
    }
}

static bool stmtNeedsInterpreter(const StatementNode* s) {
    if (!s) return false;
    switch (s->type) {
        case StatementType::FunctionDef:
        case StatementType::ReturnStmt:
        case StatementType::SwitchStmt:
        case StatementType::BreakStmt:
        case StatementType::ContinueStmt:
        case StatementType::GotoStmt:
        case StatementType::LabelStmt:
        case StatementType::DoWhileNode:
            return true;
        case StatementType::ExprStatement:
            return astNeedsInterpreter(static_cast<const ExprStatement*>(s)->getExpr());
        case StatementType::PrintNode:
            return astNeedsInterpreter(static_cast<const PrintNode*>(s)->getExpr());
        case StatementType::PrintStringNode:
            return true;
        case StatementType::DeclNode: {
            auto* d = static_cast<const DeclNode*>(s);
            return d->hasInitializer() && astNeedsInterpreter(d->getInitializer());
        }
        case StatementType::IfNode: {
            auto* i = static_cast<const IfNode*>(s);
            return astNeedsInterpreter(i->getCondition()) || stmtNeedsInterpreter(i->getThen()) ||
                   (i->getElse() != nullptr && stmtNeedsInterpreter(i->getElse()));
        }
        case StatementType::WhileNode: {
            auto* w = static_cast<const WhileNode*>(s);
            return astNeedsInterpreter(w->getCondition()) || stmtNeedsInterpreter(w->getBody());
        }
        case StatementType::ForNode: {
            auto* f = static_cast<const ForNode*>(s);
            return astNeedsInterpreter(f->getInit()) || astNeedsInterpreter(f->getCondition()) ||
                   astNeedsInterpreter(f->getUpdate()) || stmtNeedsInterpreter(f->getBody());
        }
        case StatementType::BlockNode: {
            for (const auto& c : static_cast<const BlockNode*>(s)->getStatements())
                if (stmtNeedsInterpreter(c.get())) return true;
            return false;
        }
        case StatementType::SeqNode: {
            for (const auto& c : static_cast<const SeqNode*>(s)->getStatements())
                if (stmtNeedsInterpreter(c.get())) return true;
            return false;
        }
        default:
            return false;
    }
}

bool programNeedsInterpreter(const StatementNode* root) { return stmtNeedsInterpreter(root); }

StmtInterpreter::StmtInterpreter(SymbolTable& globals)
    : globals_(globals), types_(nullptr), frameTop_(nullptr), debugTrace_(false) {
    const char* dbg = std::getenv("OOP_INTERP_DEBUG");
    debugTrace_ = (dbg && std::string(dbg) == "1");
}
StmtInterpreter::StmtInterpreter(SymbolTable& globals, TypeRegistry& types)
    : globals_(globals), types_(&types), frameTop_(nullptr), debugTrace_(false) {
    const char* dbg = std::getenv("OOP_INTERP_DEBUG");
    debugTrace_ = (dbg && std::string(dbg) == "1");
}

void StmtInterpreter::registerFunction(const FunctionDefNode* fn) { functions_[fn->getName()] = fn; }

void StmtInterpreter::collectFunctions(const StatementNode* stmt) {
    if (!stmt) return;
    if (stmt->type == StatementType::FunctionDef) {
        registerFunction(static_cast<const FunctionDefNode*>(stmt));
        return;
    }
    if (stmt->type == StatementType::SeqNode) {
        for (const auto& c : static_cast<const SeqNode*>(stmt)->getStatements()) collectFunctions(c.get());
        return;
    }
    if (stmt->type == StatementType::BlockNode) {
        for (const auto& c : static_cast<const BlockNode*>(stmt)->getStatements()) collectFunctions(c.get());
    }
}

static int32_t getVar(SymbolTable& g, const std::unordered_map<std::string, int32_t>* locals, const std::string& name) {
    if (locals && locals->count(name)) return locals->at(name);
    int32_t v = 0;
    if (g.getValue(name, v)) return v;
    throw std::runtime_error("Undefined variable: " + name);
}

static void setVar(SymbolTable& g, std::unordered_map<std::string, int32_t>* locals, const std::string& name, int32_t val) {
    if (locals) {
        auto it = locals->find(name);
        if (it != locals->end()) {
            it->second = val;
            return;
        }
        if (g.hasSymbol(name)) {
            g.setValue(name, val);
            return;
        }
        (*locals)[name] = val;
        return;
    }
    if (!g.hasSymbol(name)) g.addSymbol(name);
    g.setValue(name, val);
}

static std::string resolveName(const TypeRegistry* tr, const std::string& n) {
    if (!tr) return n;
    return tr->resolveStorageName(n);
}

void StmtInterpreter::pushFrame(StackFrame* frame) {
    frame->prev = frameTop_;
    frameTop_ = frame;
    if (debugTrace_) {
        std::cout << "[DBG] enter " << frame->functionName << "\n";
        printStackTrace("[DBG] stack");
    }
}

void StmtInterpreter::popFrame() {
    if (!frameTop_) return;
    if (debugTrace_) {
        std::cout << "[DBG] leave " << frameTop_->functionName << "\n";
    }
    frameTop_ = frameTop_->prev;
    if (debugTrace_) {
        printStackTrace("[DBG] stack");
    }
}

void StmtInterpreter::printStackTrace(const std::string& prefix) const {
    std::cout << prefix << ": ";
    const StackFrame* cur = frameTop_;
    if (!cur) {
        std::cout << "<empty>\n";
        return;
    }
    bool first = true;
    while (cur) {
        if (!first) std::cout << " <- ";
        std::cout << cur->functionName;
        first = false;
        cur = cur->prev;
    }
    std::cout << "\n";
}

int32_t StmtInterpreter::evalExpr(const ASTNode* node, std::unordered_map<std::string, int32_t>* locals) {
    if (!node) return 0;
    switch (node->type) {
        case NodeType::NumberNode:
            return static_cast<int32_t>(static_cast<const NumberNode*>(node)->getValue());
        case NodeType::VariableNode: {
            std::string nm = static_cast<const VariableNode*>(node)->getName();
            nm = resolveName(types_, nm);
            return getVar(globals_, locals, nm);
        }
        case NodeType::UnaryOpNode: {
            auto* u = static_cast<const UnaryOpNode*>(node);
            int32_t v = evalExpr(u->getOperand().get(), locals);
            if (u->getOp() == '-') return -v;
            if (u->getOp() == '~') return ~v;
            if (u->getOp() == '!') return (v == 0) ? 1 : 0;
            return v;
        }
        case NodeType::BinaryOpNode: {
            auto* b = static_cast<const BinaryOpNode*>(node);
            char op = b->getOp();
            if (op == 'A') {
                int32_t L = evalExpr(b->getLeft().get(), locals);
                if (!L) return 0;
                int32_t R = evalExpr(b->getRight().get(), locals);
                return (R != 0) ? 1 : 0;
            }
            if (op == 'O') {
                int32_t L = evalExpr(b->getLeft().get(), locals);
                if (L) return 1;
                int32_t R = evalExpr(b->getRight().get(), locals);
                return (R != 0) ? 1 : 0;
            }
            int32_t L = evalExpr(b->getLeft().get(), locals);
            int32_t R = evalExpr(b->getRight().get(), locals);
            switch (op) {
                case '+': return L + R;
                case '-': return L - R;
                case '*': return L * R;
                case '/':
                    if (R == 0) throw std::runtime_error("Division by zero");
                    return L / R;
                case '%':
                    if (R == 0) throw std::runtime_error("Modulo by zero");
                    return L % R;
                case 'Q':
                    if (R == 0) throw std::runtime_error("Floor division by zero");
                    return L / R;
                case 'P': return static_cast<int32_t>(std::pow(static_cast<double>(L), static_cast<double>(R)));
                case '&': return L & R;
                case '|': return L | R;
                case 'X': return L ^ R;
                case 'S': return L << R;
                case 'T': return L >> R;
                case '>': return L > R ? 1 : 0;
                case '<': return L < R ? 1 : 0;
                case 'G': return L >= R ? 1 : 0;
                case 'L': return L <= R ? 1 : 0;
                case 'E': return L == R ? 1 : 0;
                case 'N': return L != R ? 1 : 0;
                default: throw std::runtime_error("Unknown binary op");
            }
        }
        case NodeType::AssignmentNode: {
            auto* a = static_cast<const AssignmentNode*>(node);
            std::string target = resolveName(types_, a->getVarName());
            int32_t v = evalExpr(a->getExpression().get(), locals);
            setVar(globals_, locals, target, v);
            return v;
        }
        case NodeType::CastNode:
            return evalExpr(static_cast<const CastNode*>(node)->getInner(), locals);
        case NodeType::CallNode: {
            auto* c = static_cast<const CallNode*>(node);
            if (c->getFuncName() == "sqrt") {
                const auto& args = c->getArgs();
                if (args.size() != 1) throw std::runtime_error("sqrt expects 1 arg");
                int32_t v = evalExpr(args[0].get(), locals);
                if (v < 0) throw std::runtime_error("sqrt of negative value");
                return static_cast<int32_t>(std::sqrt(static_cast<double>(v)));
            }
            if (c->getFuncName() == "abs") {
                const auto& args = c->getArgs();
                if (args.size() != 1) throw std::runtime_error("abs expects 1 arg");
                int32_t v = evalExpr(args[0].get(), locals);
                return v < 0 ? -v : v;
            }
            auto callMath1 = [&](double (*fn)(double), const char* name) -> int32_t {
                const auto& args = c->getArgs();
                if (args.size() != 1) throw std::runtime_error(std::string(name) + " expects 1 arg");
                return static_cast<int32_t>(fn(static_cast<double>(evalExpr(args[0].get(), locals))));
            };
            if (c->getFuncName() == "sin") return callMath1(std::sin, "sin");
            if (c->getFuncName() == "cos") return callMath1(std::cos, "cos");
            if (c->getFuncName() == "tan") return callMath1(std::tan, "tan");
            if (c->getFuncName() == "asin") return callMath1(std::asin, "asin");
            if (c->getFuncName() == "acos") return callMath1(std::acos, "acos");
            if (c->getFuncName() == "atan") return callMath1(std::atan, "atan");
            if (c->getFuncName() == "cbrt") return callMath1(std::cbrt, "cbrt");
            if (c->getFuncName() == "exp") return callMath1(std::exp, "exp");
            if (c->getFuncName() == "log") return callMath1(std::log, "log");
            if (c->getFuncName() == "ln") return callMath1(std::log, "ln");
            if (c->getFuncName() == "log10") return callMath1(std::log10, "log10");
            if (c->getFuncName() == "log2") return callMath1(std::log2, "log2");
            if (c->getFuncName() == "ceil") return callMath1(std::ceil, "ceil");
            if (c->getFuncName() == "atan2" || c->getFuncName() == "pow" || c->getFuncName() == "fmod" || c->getFuncName() == "log_ab") {
                const auto& args = c->getArgs();
                if (args.size() != 2) throw std::runtime_error(c->getFuncName() + " expects 2 args");
                double a0 = static_cast<double>(evalExpr(args[0].get(), locals));
                double a1 = static_cast<double>(evalExpr(args[1].get(), locals));
                if (c->getFuncName() == "atan2") return static_cast<int32_t>(std::atan2(a0, a1));
                if (c->getFuncName() == "pow") return static_cast<int32_t>(std::pow(a0, a1));
                if (c->getFuncName() == "fmod") return static_cast<int32_t>(std::fmod(a0, a1));
                return static_cast<int32_t>(std::log(a1) / std::log(a0));
            }
            auto it = functions_.find(c->getFuncName());
            if (it == functions_.end()) throw std::runtime_error("Undefined function: " + c->getFuncName());
            const FunctionDefNode* fn = it->second;
            StackFrame frame;
            frame.functionName = c->getFuncName();
            const auto& args = c->getArgs();
            const auto& ps = fn->getParams();
            if (args.size() != ps.size())
                throw std::runtime_error("Wrong argument count for " + c->getFuncName());
            for (size_t i = 0; i < args.size(); ++i) frame.locals[ps[i].second] = evalExpr(args[i].get(), locals);
            int32_t ret = 0;
            bool returned = false, brk = false, cont = false;
            std::string go;
            pushFrame(&frame);
            execStmt(fn->getBody(), &frame.locals, ret, returned, brk, cont, go);
            popFrame();
            if (fn->isVoid()) return 0;
            if (!returned) return 0;
            return ret;
        }
        case NodeType::TernaryNode: {
            auto* t = static_cast<const TernaryNode*>(node);
            int32_t c = evalExpr(t->getCondition(), locals);
            return c ? evalExpr(t->getThenExpr(), locals) : evalExpr(t->getElseExpr(), locals);
        }
        default:
            throw std::runtime_error("Unsupported expression in interpreter");
    }
}

void StmtInterpreter::execStmt(const StatementNode* stmt, std::unordered_map<std::string, int32_t>* locals,
                                 int32_t& returnValue, bool& returned, bool& breakFlag, bool& continueFlag,
                                 std::string& gotoLabel) {
    if (!stmt || returned || breakFlag || continueFlag || !gotoLabel.empty()) return;

    switch (stmt->type) {
        case StatementType::ExprStatement: {
            evalExpr(static_cast<const ExprStatement*>(stmt)->getExpr(), locals);
            return;
        }
        case StatementType::PrintNode: {
            int32_t v = evalExpr(static_cast<const PrintNode*>(stmt)->getExpr(), locals);
            std::cout << v << std::endl;
            return;
        }
        case StatementType::PrintStringNode: {
            std::cout << static_cast<const PrintStringNode*>(stmt)->getText();
            return;
        }
        case StatementType::DeclNode: {
            auto* d = static_cast<const DeclNode*>(stmt);
            int32_t v = 0;
            if (d->hasInitializer()) v = evalExpr(d->getInitializer(), locals);
            if (locals) {
                (*locals)[d->getVarName()] = v;
            } else {
                if (!globals_.hasSymbol(d->getVarName())) globals_.addSymbol(d->getVarName());
                globals_.setValue(d->getVarName(), v);
            }
            return;
        }
        case StatementType::ReturnStmt: {
            auto* r = static_cast<const ReturnNode*>(stmt);
            if (!r->isVoidReturn()) returnValue = evalExpr(r->getExpr(), locals);
            returned = true;
            return;
        }
        case StatementType::BreakStmt: {
            breakFlag = true;
            return;
        }
        case StatementType::ContinueStmt: {
            continueFlag = true;
            return;
        }
        case StatementType::GotoStmt: {
            gotoLabel = static_cast<const GotoNode*>(stmt)->getLabel();
            return;
        }
        case StatementType::LabelStmt:
            return;
        case StatementType::IfNode: {
            auto* i = static_cast<const IfNode*>(stmt);
            int32_t c = evalExpr(i->getCondition(), locals);
            if (c) execStmt(i->getThen(), locals, returnValue, returned, breakFlag, continueFlag, gotoLabel);
            else if (i->getElse()) execStmt(i->getElse(), locals, returnValue, returned, breakFlag, continueFlag, gotoLabel);
            return;
        }
        case StatementType::WhileNode: {
            auto* w = static_cast<const WhileNode*>(stmt);
            while (!returned && !breakFlag && gotoLabel.empty()) {
                if (!evalExpr(w->getCondition(), locals)) break;
                execStmt(w->getBody(), locals, returnValue, returned, breakFlag, continueFlag, gotoLabel);
                if (continueFlag) continueFlag = false;
            }
            return;
        }
        case StatementType::DoWhileNode: {
            auto* d = static_cast<const DoWhileNode*>(stmt);
            do {
                execStmt(d->getBody(), locals, returnValue, returned, breakFlag, continueFlag, gotoLabel);
                if (continueFlag) continueFlag = false;
                if (returned || breakFlag || !gotoLabel.empty()) break;
            } while (evalExpr(d->getCondition(), locals));
            return;
        }
        case StatementType::ForNode: {
            auto* f = static_cast<const ForNode*>(stmt);
            if (f->getInit()) evalExpr(f->getInit(), locals);
            while (!returned && !breakFlag && gotoLabel.empty()) {
                if (f->getCondition() && !evalExpr(f->getCondition(), locals)) break;
                execStmt(f->getBody(), locals, returnValue, returned, breakFlag, continueFlag, gotoLabel);
                if (returned || breakFlag || !gotoLabel.empty()) break;
                if (f->getUpdate()) evalExpr(f->getUpdate(), locals);
                if (continueFlag) continueFlag = false;
            }
            return;
        }
        case StatementType::BlockNode: {
            const auto& list = static_cast<const BlockNode*>(stmt)->getStatements();
            std::unordered_map<std::string, size_t> labels;
            for (size_t i = 0; i < list.size(); ++i) {
                if (list[i]->type == StatementType::LabelStmt)
                    labels[static_cast<const LabelNode*>(list[i].get())->getLabel()] = i;
            }
            size_t pc = 0;
            while (pc < list.size()) {
                execStmt(list[pc].get(), locals, returnValue, returned, breakFlag, continueFlag, gotoLabel);
                if (returned || breakFlag || continueFlag) return;
                if (!gotoLabel.empty()) {
                    auto it = labels.find(gotoLabel);
                    if (it == labels.end()) throw std::runtime_error("Unknown label for goto: " + gotoLabel);
                    pc = it->second + 1;
                    gotoLabel.clear();
                    continue;
                }
                ++pc;
            }
            return;
        }
        case StatementType::SeqNode: {
            const auto& list = static_cast<const SeqNode*>(stmt)->getStatements();
            std::unordered_map<std::string, size_t> labels;
            for (size_t i = 0; i < list.size(); ++i) {
                if (list[i]->type == StatementType::LabelStmt)
                    labels[static_cast<const LabelNode*>(list[i].get())->getLabel()] = i;
            }
            size_t pc = 0;
            while (pc < list.size()) {
                execStmt(list[pc].get(), locals, returnValue, returned, breakFlag, continueFlag, gotoLabel);
                if (returned || breakFlag || continueFlag) return;
                if (!gotoLabel.empty()) {
                    auto it = labels.find(gotoLabel);
                    if (it == labels.end()) throw std::runtime_error("Unknown label for goto: " + gotoLabel);
                    pc = it->second + 1;
                    gotoLabel.clear();
                    continue;
                }
                ++pc;
            }
            return;
        }
        case StatementType::SwitchStmt: {
            auto* sw = static_cast<const SwitchNode*>(stmt);
            int32_t d = evalExpr(sw->getDiscriminant(), locals);
            bool matched = false;
            for (const auto& arm : sw->getArms()) {
                if (arm.isDefault) continue;
                if (arm.caseValue != d) continue;
                matched = true;
                for (const auto& st : arm.statements) {
                    execStmt(st.get(), locals, returnValue, returned, breakFlag, continueFlag, gotoLabel);
                    if (breakFlag) {
                        breakFlag = false;
                        return;
                    }
                    if (returned || continueFlag || !gotoLabel.empty()) return;
                }
                return;
            }
            if (!matched) {
                for (const auto& arm : sw->getArms()) {
                    if (!arm.isDefault) continue;
                    for (const auto& st : arm.statements) {
                        execStmt(st.get(), locals, returnValue, returned, breakFlag, continueFlag, gotoLabel);
                        if (breakFlag) {
                            breakFlag = false;
                            return;
                        }
                        if (returned || continueFlag || !gotoLabel.empty()) return;
                    }
                    return;
                }
            }
            return;
        }
        case StatementType::FunctionDef:
            return;
        default:
            throw std::runtime_error("Unsupported statement in interpreter");
    }
}

int32_t StmtInterpreter::run(const StatementNode* root) {
    functions_.clear();
    collectFunctions(root);
    int32_t ret = 0;
    bool returned = false, brk = false, cont = false;
    std::string go;
    execStmt(root, nullptr, ret, returned, brk, cont, go);

    returned = false;
    brk = false;
    cont = false;
    go.clear();
    auto it = functions_.find("main");
    if (it != functions_.end()) {
        StackFrame frame;
        frame.functionName = "main";
        pushFrame(&frame);
        execStmt(it->second->getBody(), &frame.locals, ret, returned, brk, cont, go);
        popFrame();
    }
    return ret;
}
