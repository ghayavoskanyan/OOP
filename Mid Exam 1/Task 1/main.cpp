#include <iostream>
#include "Task 1.h"

int main() {
    ExpressionEvaluator ev;

    std::map<std::string, double> vars = {
        {"x", 5}
    };

    std::cout << ev.evaluate("3 + x * 2", vars) << std::endl;
}