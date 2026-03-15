#include "Manager.h"
#include <iostream>

Manager::Manager() {
    calculator = std::make_unique<Calculator>(symbolTable);
}

void Manager::setInput(const std::string& expression) {
    reset();
    
    auto inputStream = std::make_unique<std::istringstream>(expression);
    
    lexer = std::make_unique<Lexer>(*inputStream);
    parser = std::make_unique<Parser>(*lexer, symbolTable); 
    
    ownedStreams.push_back(std::move(inputStream));
}

double Manager::evaluate() {
    if (!parser) return 0.0;

    auto ast = parser->parse();
    
    if (!ast) {
        std::cerr << "Error: Failed to parse expression." << std::endl;
        return 0.0;
    }

    return calculator->calculate(std::move(ast));
}

bool Manager::getVariable(const std::string& name, double& value) {
    return symbolTable.getValue(name, value);
}

void Manager::reset() {
    lexer.reset();
    parser.reset();
    ownedStreams.clear();
}