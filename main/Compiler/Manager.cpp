#include "Manager.h"
#include "CompileRegs.h"
#include "ExeImage.h"
#include "IrFile.h"
#include "IrToRiscv.h"
#include "StatementNode.h"
#include "StmtInterpreter.h"
#include "VmMonitor.h"
#include <fstream>
#include <filesystem>
#include <cctype>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <unordered_map>

static std::string stripComments(const std::string& src) {
    std::string out;
    bool inLine = false, inBlock = false, inString = false;
    for (size_t i = 0; i < src.size(); ++i) {
        char c = src[i];
        char n = (i + 1 < src.size()) ? src[i + 1] : '\0';
        if (inLine) {
            if (c == '\n') {
                inLine = false;
                out.push_back(c);
            }
            continue;
        }
        if (inBlock) {
            if (c == '*' && n == '/') {
                inBlock = false;
                ++i;
            } else if (c == '\n') {
                out.push_back('\n');
            }
            continue;
        }
        if (!inString && c == '/' && n == '/') {
            inLine = true;
            ++i;
            continue;
        }
        if (!inString && c == '/' && n == '*') {
            inBlock = true;
            ++i;
            continue;
        }
        if (c == '"' && (i == 0 || src[i - 1] != '\\')) inString = !inString;
        out.push_back(c);
    }
    return out;
}

static std::string applyDefines(const std::string& line, const std::unordered_map<std::string, std::string>& defs) {
    std::string out;
    for (size_t i = 0; i < line.size();) {
        if (std::isalpha(static_cast<unsigned char>(line[i])) || line[i] == '_') {
            size_t j = i + 1;
            while (j < line.size() && (std::isalnum(static_cast<unsigned char>(line[j])) || line[j] == '_')) ++j;
            std::string tok = line.substr(i, j - i);
            auto it = defs.find(tok);
            if (it != defs.end()) out += it->second;
            else out += tok;
            i = j;
        } else {
            out.push_back(line[i++]);
        }
    }
    return out;
}

static std::string preprocessSource(const std::string& src, const std::filesystem::path& baseDir,
                                   std::unordered_map<std::string, std::string>& defs, int depth = 0) {
    if (depth > 16) throw std::runtime_error("Import nesting too deep");
    std::istringstream iss(stripComments(src));
    std::ostringstream out;
    std::string line;
    while (std::getline(iss, line)) {
        std::string trimmed = line;
        size_t p = trimmed.find_first_not_of(" \t");
        trimmed = (p == std::string::npos) ? "" : trimmed.substr(p);
        if (trimmed.rfind("#define", 0) == 0) {
            std::istringstream ds(trimmed);
            std::string hash, name, value;
            ds >> hash >> name;
            std::getline(ds, value);
            size_t vp = value.find_first_not_of(" \t");
            if (!name.empty() && vp != std::string::npos) defs[name] = value.substr(vp);
            continue;
        }
        if (trimmed.rfind("import", 0) == 0) {
            size_t q1 = trimmed.find('"');
            size_t q2 = (q1 == std::string::npos) ? std::string::npos : trimmed.find('"', q1 + 1);
            if (q1 != std::string::npos && q2 != std::string::npos && q2 > q1 + 1) {
                std::filesystem::path imp = baseDir / trimmed.substr(q1 + 1, q2 - q1 - 1);
                std::ifstream f(imp.string());
                if (!f.is_open()) throw std::runtime_error("Cannot open imported file: " + imp.string());
                std::ostringstream buf;
                buf << f.rdbuf();
                out << preprocessSource(buf.str(), imp.parent_path(), defs, depth + 1) << "\n";
                continue;
            }
        }
        out << applyDefines(line, defs) << "\n";
    }
    return out.str();
}

Manager::Manager() { calculator = std::make_unique<InstructionCalculator>(symbolTable); }

void Manager::setInput(const std::string& expression) {
    lexer.reset();
    stmtParser.reset();
    ownedStreams.clear();
    calculator->clear();

    auto stream = std::make_unique<std::istringstream>(expression);
    lexer = std::make_unique<Lexer>(*stream);
    stmtParser = std::make_unique<StatementParser>(*lexer, symbolTable, typeRegistry);
    ownedStreams.push_back(std::move(stream));
}

int32_t Manager::evaluate() {
    if (!stmtParser) return 0;

    auto stmt = stmtParser->parse();
    if (!stmt) {
        std::cerr << "Error: Failed to parse statement.\n";
        return 0;
    }

    int32_t result = 0;
    if (programNeedsInterpreter(stmt.get())) {
        StmtInterpreter interp(symbolTable, typeRegistry);
        result = interp.run(stmt.get());
    } else {
        std::vector<Instruction> program;
        compile_regs::reset();
        stmt->compile(program);
        VirtualMachine vm(symbolTable, 64);
        result = vm.execute(program);
    }

    if (stmt->type == StatementType::PrintNode) {
        std::cout << ">> " << result << "\n";
    }

    return result;
}

bool Manager::getVariable(const std::string& name, int32_t& value) { return symbolTable.getValue(name, value); }

void Manager::printAllVariables() {
    std::cout << "--- Variables ---\n";
    std::vector<int32_t>& vals = symbolTable.getValuesVector();
    if (vals.empty()) {
        std::cout << "(none)\n";
        return;
    }
    for (size_t i = 0; i < vals.size(); i++) {
        std::cout << "  var[" << i << "] = " << vals[i] << "\n";
    }
}

void Manager::runFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filepath);
    }

    std::ostringstream oss;
    oss << file.rdbuf();
    std::unordered_map<std::string, std::string> defs;
    std::string code = preprocessSource(oss.str(), std::filesystem::path(filepath).parent_path(), defs);
    file.close();

    std::cout << "Running file: " << filepath << "\n";
    std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";

    auto stream = std::make_unique<std::istringstream>(code);
    Lexer fileLexer(*stream);
    StatementParser fileParser(fileLexer, symbolTable, typeRegistry);

    auto stmt = fileParser.parse();
    if (!stmt) {
        std::cerr << "Error: Failed to parse file.\n";
        return;
    }

    if (programNeedsInterpreter(stmt.get())) {
        StmtInterpreter interp(symbolTable, typeRegistry);
        interp.run(stmt.get());
    } else {
        std::vector<Instruction> program;
        compile_regs::reset();
        stmt->compile(program);
        VirtualMachine vm(symbolTable, 64);
        vm.execute(program);
    }

    std::cout << "\n--- Results ---\n";
    printAllVariables();
}

bool Manager::setInputFromFile(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) return false;

    std::ostringstream oss;
    oss << file.rdbuf();
    setInput(oss.str());
    return true;
}

void Manager::reset() {
    lexer.reset();
    stmtParser.reset();
    ownedStreams.clear();
    calculator->clear();
}

bool Manager::compileFileToIr(const std::string& sourcePath, const std::string& irPath) {
    std::ifstream file(sourcePath);
    if (!file.is_open()) return false;
    std::ostringstream oss;
    oss << file.rdbuf();
    std::unordered_map<std::string, std::string> defs;
    std::string code = preprocessSource(oss.str(), std::filesystem::path(sourcePath).parent_path(), defs);

    std::istringstream iss(code);
    Lexer lex(iss);
    StatementParser parser(lex, symbolTable, typeRegistry);
    auto stmt = parser.parse();
    if (!stmt) return false;

    if (programNeedsInterpreter(stmt.get())) {
        std::cerr << "IR file: program uses interpreter-only constructs (functions, switch, calls, ...).\n";
        return false;
    }
    std::vector<Instruction> program;
    compile_regs::reset();
    stmt->compile(program);
    return writeIrFile(irPath, program);
}

bool Manager::compileFileToExe(const std::string& sourcePath, const std::string& exePath) {
    std::ifstream file(sourcePath);
    if (!file.is_open()) return false;
    std::ostringstream oss;
    oss << file.rdbuf();
    std::unordered_map<std::string, std::string> defs;
    std::string code = preprocessSource(oss.str(), std::filesystem::path(sourcePath).parent_path(), defs);

    std::istringstream iss(code);
    Lexer lex(iss);
    StatementParser parser(lex, symbolTable, typeRegistry);
    auto stmt = parser.parse();
    if (!stmt) return false;

    if (programNeedsInterpreter(stmt.get())) {
        std::cerr << "Executable: program uses interpreter-only constructs (functions, switch, calls, ...).\n";
        return false;
    }
    std::vector<Instruction> program;
    compile_regs::reset();
    stmt->compile(program);

    auto tr = ir_to_riscv::translate(program, static_cast<uint32_t>(symbolTable.getValuesVector().size()));
    std::vector<int32_t> data = symbolTable.getValuesVector();
    return writeExeFile(exePath, tr.code, data, tr.maxVReg, kDefaultEntryAddr);
}

bool Manager::runRiscvExe(const std::string& exePath, std::string& errOut) { return VmMonitor::runExeFile(exePath, errOut); }
