#include "Manager.h"
#include "StatementNode.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdexcept>

Manager::Manager() {
    calculator = std::make_unique<InstructionCalculator>(symbolTable);
}

void Manager::setInput(const std::string& expression) {
    lexer.reset();
    stmtParser.reset();
    ownedStreams.clear();
    calculator->clear();

    auto stream = std::make_unique<std::istringstream>(expression);
    lexer      = std::make_unique<Lexer>(*stream);
    stmtParser = std::make_unique<StatementParser>(*lexer, symbolTable);
    ownedStreams.push_back(std::move(stream));
}

double Manager::evaluate() {
    if (!stmtParser) return 0.0;

    auto stmt = stmtParser->parse();
    if (!stmt) {
        std::cerr << "Error: Failed to parse statement.\n";
        return 0.0;
    }

    std::vector<Instruction> program;
    stmt->compile(program);

    VirtualMachine vm(symbolTable, 64);
    double result = vm.execute(program);

    if (stmt->type == StatementType::PrintNode) {
        std::cout << ">> " << result << "\n";
    }

    return result;
}

bool Manager::getVariable(const std::string& name, double& value) {
    return symbolTable.getValue(name, value);
}

void Manager::printAllVariables() {
    std::cout << "--- Variables ---\n";
    std::vector<double>& vals = symbolTable.getValuesVector();
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

    std::vector<Instruction> program;
    stmt->compile(program);

    VirtualMachine vm(symbolTable, 64);

    vm.execute(program);

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