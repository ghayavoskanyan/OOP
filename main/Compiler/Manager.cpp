#include "Manager.h"
#include "CompileRegs.h"
#include "ExeImage.h"
#include "IrFile.h"
#include "IrToRiscv.h"
#include "StatementNode.h"
#include "StmtInterpreter.h"
#include "VmMonitor.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

Manager::Manager() { calculator = std::make_unique<InstructionCalculator>(symbolTable); }

void Manager::setInput(const std::string& expression) {
    lexer.reset();
    stmtParser.reset();
    ownedStreams.clear();
    calculator->clear();

    auto stream = std::make_unique<std::istringstream>(expression);
    lexer = std::make_unique<Lexer>(*stream);
    stmtParser = std::make_unique<StatementParser>(*lexer, symbolTable);
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
        StmtInterpreter interp(symbolTable);
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
    std::string code = oss.str();
    file.close();

    std::cout << "Running file: " << filepath << "\n";
    std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";

    auto stream = std::make_unique<std::istringstream>(code);
    Lexer fileLexer(*stream);
    StatementParser fileParser(fileLexer, symbolTable);

    auto stmt = fileParser.parse();
    if (!stmt) {
        std::cerr << "Error: Failed to parse file.\n";
        return;
    }

    if (programNeedsInterpreter(stmt.get())) {
        StmtInterpreter interp(symbolTable);
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
    std::string code = oss.str();

    std::istringstream iss(code);
    Lexer lex(iss);
    StatementParser parser(lex, symbolTable);
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
    std::string code = oss.str();

    std::istringstream iss(code);
    Lexer lex(iss);
    StatementParser parser(lex, symbolTable);
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
