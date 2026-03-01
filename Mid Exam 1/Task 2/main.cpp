#include "Task 2.h"
#include <iostream>
#include <map>
#include <memory>

int main() {
    std::string expr;
    std::cout << "Enter expression: ";
    std::getline(std::cin, expr);

    Parser parser(expr);
    try {
        std::unique_ptr<Node> tree = parser.parseExpression();

        std::map<std::string, double> vars;
        vars["x"] = 5;  // օրինակ variable value

        std::cout << "\nExpression tree:\n";
        tree->print();

        double result = tree->eval(vars);
        std::cout << "\nResult: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}