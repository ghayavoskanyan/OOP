#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include "Manager.h"

int main() {
    Manager manager;
    std::string input;
    
    std::cout << "Expression Calculator!\n";
    std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    std::cout << "Supported operations: +, -, *, /, parentheses, variables\n";
    std::cout << "Examples:\n";
    std::cout << "  w = -5 + 3\n";
    std::cout << "  n = w + 10\n";
    std::cout << "  x = (5 + 3) * 2\n";
    std::cout << "Type 'quit' to exit\n\n";
    
    while (true) {
        std::cout << "Enter expression: ";
        std::getline(std::cin, input);
        
        if (input == "quit" || input == "exit") {
            break;
        }
        
        if (input.empty()) {
            continue;
        }
        
        try {
            manager.setInput(input);
            double result = manager.evaluate();
            std::cout << "Result: " << result << "\n";
            
            // Check if it was an assignment and show variable value
            size_t eqPos = input.find('=');
            if (eqPos != std::string::npos) {
                std::string varName = input.substr(0, eqPos);
                // Trim whitespace
                size_t first = varName.find_first_not_of(" \t");
                if (first != std::string::npos) {
                    varName = varName.substr(first);
                    size_t last = varName.find_last_not_of(" \t");
                    if (last != std::string::npos) {
                        varName = varName.substr(0, last + 1);
                    }
                }
                double val;
                if (manager.getVariable(varName, val)) {
                    std::cout << "Variable " << varName << " = " << val << "\n";
                }
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
        
        std::cout << "\n";
    }
    return 0;
}