/* Սովորական infix արտահայտությունը դժվար է հաշվել, որովհետև առաջնահերթություն 
հաշվելու խնդիր կա։ Իսկ postfix-ում արդեն դասավորված ու stack-ով հեշտ է հաշվել */
#include "Task 1.h"
#include <stack>
#include <sstream>
#include <stdexcept>
#include <cctype>

/*օպերատորի առաջնահերթություն*/
int ExpressionEvaluator::precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

/*սարքում ենք Postfix, որ Stack-ով հեշտ հաշվենք*/
std::string ExpressionEvaluator::toPostfix(const std::string& s) {
    std::stack<char> st; /*ստեղ պահելու ենք օպերատորները*/
    std::string result; /*postfix արտահայտությունն է*/
    bool lastWasOp = true; /*սա պահում է ինֆո վերջին կարդացած սիմվոլը օպերատոր էր թե ոչ?*/

    for (size_t i = 0; i < s.length(); ++i) {
        char c = s[i]; /*պահում ենք ընթացիկ սիմվոլը*/

        if (isspace(c)) continue; /*անտեսում է space-երը*/

        if (c == '-' && lastWasOp) { /*ստեղ նայում ենք դա թվի նշան է թե օպերատոր*/
            result += "0 ";
            st.push('-');
            continue;
        }

        if (std::isalnum(c) || c == '.') { /*եթե թիվ է*/
            std::string token; 
            while (i < s.length() && (std::isalnum(s[i]) || s[i] == '.')) {
                token += s[i++]; /*հավաքում ենք ամբողջ թիվը*/
            }
            result += token + " "; /*հետո ավելացնում ենք postfix-ին*/
            --i;
            lastWasOp = false; /*քանի որ հիմա վերջինը թիվ է*/
        }
        else if (c == '(') {
            st.push(c); /*քցում ենք Stack*/
            lastWasOp = true;
        }
        else if (c == ')') {
            while (!st.empty() && st.top() != '(') { /*հանում ենք Stack-ից*/
                result += st.top();
                result += " ";
                st.pop();
            }
            if (st.empty())
                throw std::runtime_error("Mismatched parentheses");
            st.pop();
            lastWasOp = false;
        }
        else { /*օպերատոր է*/
            while (!st.empty() && precedence(st.top()) >= precedence(c)) {
                /*եթե stack-ի վերևի օպերատորը ունի ավելի բարձր կամ հավասար priority, հանիր*/
                result += st.top();
                result += " ";
                st.pop();
            }
            st.push(c);
            lastWasOp = true;
        }
    }

    while (!st.empty()) {
        if (st.top() == '(')
            throw std::runtime_error("Mismatched parentheses");
        result += st.top();
        result += " ";
        st.pop();
    }

    return result;
}

double ExpressionEvaluator::evalPostfix(
    const std::string& postfix,
    const std::map<std::string, double>& variables) {

    std::stack<double> st;
    std::stringstream ss(postfix);
    std::string token;

    while (ss >> token) { /*վերցնում ենք հերթական token-ը*/
        try {
            size_t idx;
            double val = std::stod(token, &idx);
            if (idx == token.size()) {
                st.push(val);
                continue;
            }
        } catch (...) {}

        if (std::isalpha(token[0])) {
            auto it = variables.find(token);
            if (it == variables.end())
                throw std::runtime_error("Undefined variable: " + token);
            st.push(it->second);
            continue;
        }

        if (st.size() < 2)
            throw std::runtime_error("Invalid expression");

        double right = st.top(); st.pop();
        double left = st.top(); st.pop();

        if (token == "+") st.push(left + right);
        else if (token == "-") st.push(left - right);
        else if (token == "*") st.push(left * right);
        else if (token == "/") {
            if (right == 0)
                throw std::runtime_error("Division by zero");
            st.push(left / right);
        }
        else {
            throw std::runtime_error("Unknown operator: " + token);
        }
    }

    if (st.size() != 1)
        throw std::runtime_error("Invalid expression");

    return st.top();
}

double ExpressionEvaluator::evaluate(
    const std::string& expr,
    const std::map<std::string, double>& variables) {

    std::string postfix = toPostfix(expr);
    return evalPostfix(postfix, variables);
}

