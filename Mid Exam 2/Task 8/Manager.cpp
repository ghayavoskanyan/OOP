/*Manager.cpp*/ #include "Manager.h"
#include <iostream>

Manager::Manager() {
    calculator = std::make_unique<InstructionCalculator>(symbolTable);
}

void Manager::setInput(const std::string& expression) {
    reset();
    auto inputStream = std::make_unique<std::istringstream>(expression);
    lexer = std::make_unique<Lexer>(*inputStream);
    stmtParser = std::make_unique<StatementParser>(*lexer, symbolTable);
    ownedStreams.push_back(std::move(inputStream));
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
    VirtualMachine vm(symbolTable, 32);
    return vm.execute(program);
}

bool Manager::getVariable(const std::string& name, double& value) {
    return symbolTable.getValue(name, value);
}

void Manager::reset() {
    lexer.reset();
    stmtParser.reset();
    ownedStreams.clear();
    calculator->clear();
}