#pragma once
#include <string>
#include <map>

class ExpressionEvaluator {
private:
    int precedence(char op); /*օպերատորի առաջնահերթություն*/
    std::string toPostfix(const std::string& expr); /*սարքում ենք Postfix, որ Stack-ով հեշտ հաշվենք*/
    double evalPostfix(const std::string& postfix, const std::map<std::string, double>& variables); /*հաշվում է 
    արտահայտությունը*/

public:
    double evaluate(const std::string& expr, const std::map<std::string, double>& variables); /*ստանում ենք 
    մուտքային string-ը, ասենք "3 + x * 2"*/
};