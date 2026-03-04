#include "Manager.h"
#include <iostream>
#include <sstream>
#include <memory>

Manager::Manager() : currentInput(nullptr) {
    calculator = std::make_unique<Calculator>(symbolTable);
}

void Manager::setInput(const std::string& expression) {
    auto inputStream = std::make_unique<std::istringstream>(expression);
    currentInput = inputStream.get();
    lexer = std::make_unique<Lexer>(*inputStream);
    parser = std::make_unique<Parser>(*lexer, symbolTable);
    ownedStreams.push_back(std::move(inputStream));
}

void Manager::setInput(std::istream& is) {
    currentInput = &is;
    lexer = std::make_unique<Lexer>(is);
    parser = std::make_unique<Parser>(*lexer, symbolTable);
}

double Manager::evaluate() {
    if (!parser) {
        throw std::runtime_error("No input set");
    }
    
    auto ast = parser->parse();
    return calculator->calculate(std::move(ast));
}

void Manager::setVariable(const std::string& name, double value) {
    symbolTable.setValue(name, value);
}

bool Manager::getVariable(const std::string& name, double& value) {
    return symbolTable.getValue(name, value);
}

void Manager::reset() {
    lexer.reset();
    parser.reset();
    ownedStreams.clear();
}