#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include "Manager.h"

int main() {
    Manager manager; //կանչում ենք դիրիժորին
    std::string input;
    
    std::cout << "Expression Calculator!\n";
    std::cout << "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n";
    std::cout << "Supported operations: +, -, *, /, parentheses, variables\n";
    std::cout << "Examples:\n";
    std::cout << "  w = -5 + 3\n";
    std::cout << "  n = w + 10\n";
    std::cout << "  x = (5 + 3) * 2\n";
    std::cout << "Type 'quit' to exit\n\n";
    
    while (true) { //մտնում ենք անվերջ ցիկլ, մինչև user-ը quit/exit չանի
        std::cout << "Enter expression: ";
        std::getline(std::cin, input);
        
        if (input == "quit" || input == "exit") {
            break;
        }
        
        if (input.empty()) {
            continue;
        }
        
        try {
            manager.setInput(input); //մուտքագրած տեքստը տալիս ենք մենեջերին
            double result = manager.evaluate(); //ասում ենք հաշվիր
            std::cout << "Result: " << result << "\n"; //արդյունքն էլ տպիր
            
            size_t eqPos = input.find('='); 
            if (eqPos != std::string::npos) { /*մաքրումա բացատներն ու հարցնումա մենեջեին թե փոփոխականի արժեքն ինչ է ու տպում է*/
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
        }
        catch (const std::exception& e) { /*սխալներն ա բռնում։ Որ գրեմ 5/0 ծրագիրս չի քրաշվի, կասի որ error կա*/
            std::cerr << "Error: " << e.what() << "\n";
        }
        
        std::cout << "\n";
    }
    return 0;
}