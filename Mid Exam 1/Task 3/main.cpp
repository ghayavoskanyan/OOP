#include "Task 3.h"
#include <iostream>
#include <map>

int main() {
    std::string expr;
    std::cout << "Enter expression: ";
    std::getline(std::cin, expr);

    std::map<std::string,double> vars;
    vars["x"] = 5;
    vars["y"] = 2;

    Evaluator eval;
    try {
        double result = eval.evaluate(expr, vars);
        std::cout << "Result: " << result << std::endl;
    } catch(const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}