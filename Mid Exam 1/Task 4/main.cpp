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
    std::cout << "An example: x = 5 + 3\n";
    std::cout << "Type 'quit' to exit\n\n";
    
    std::cout << "Test variables: a = 10, b = 20\n";
    try {
        manager.setInput("a = 10");
        manager.evaluate();
        manager.setInput("b = 20");
        manager.evaluate();
        std::cout << "Done.\n\n";
    } catch (const std::exception& e) {
        std::cerr << "Error setting variables: " << e.what() << "\n\n";
    }
    
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
            
            if (input.find('=') != std::string::npos) {
                size_t eqPos = input.find('=');
                std::string varName = input.substr(0, eqPos);
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
            
            double a_val, b_val;
            if (manager.getVariable("a", a_val) && manager.getVariable("b", b_val)) {
                std::cout << "Current: a = " << a_val << ", b = " << b_val << "\n";
            }
        }
        catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
        
        std::cout << "\n";
    }
    return 0;
}