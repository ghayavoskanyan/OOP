/*Lexer.cpp*/ /*աշխատում է որպես Finite State Machine (FSM)։ Ինքը կարդում է տեքստը նշան առ նշան ու 
վիճակների միջոցով որոշում՝ թիվ է կարդում, թե անուն*/
#include "Lexer.h"
#include <cctype>

Lexer::Lexer(std::istream& is) : input(is), currentState(LexerState::Start), line(1), column(0) {
    initializeTransitionMatrix(); /*ասում է եթե հիմա Start վիճակում ես ու տեսար թիվ գնա InNumber վիճակ*/
}

void Lexer::initializeTransitionMatrix() {
    for (int i = 0; i < STATE_COUNT; i++) { /*լցնում ենք մատրիցան default start-երով*/
        for (int j = 0; j < CHAR_TYPE_COUNT; j++) {
            transitionMatrix[i][j] = LexerState::Start;
        }
    }

    /*սահմանել եմ կանոնները ու անցումները, օրինակ եթե արդեն InNumber-ի մեջ եմ ու նորից թիվ եմ տեսնում, մնում եմ InNumber վիճակում*/
    transitionMatrix[(int)LexerState::Start][(int)CharType::Digit] = LexerState::InNumber;
    transitionMatrix[(int)LexerState::Start][(int)CharType::Letter] = LexerState::InName;
    transitionMatrix[(int)LexerState::Start][(int)CharType::Operator_] = LexerState::InOperator;
    
    transitionMatrix[(int)LexerState::InNumber][(int)CharType::Digit] = LexerState::InNumber;
    transitionMatrix[(int)LexerState::InNumber][(int)CharType::Operator_] = LexerState::InOperator;
    
    transitionMatrix[(int)LexerState::InName][(int)CharType::Letter] = LexerState::InName;
    transitionMatrix[(int)LexerState::InName][(int)CharType::Digit] = LexerState::InName;
    transitionMatrix[(int)LexerState::InName][(int)CharType::Operator_] = LexerState::InOperator;
}

/*սա ասում է, թե ինչ տեսակի է կոնկրետ սիմվոլը, որ մատրիցայի մեջ ճիշտ սյունակը գտնի*/
Lexer::CharType Lexer::getCharType(char c) { 
    if (isdigit(c)) return CharType::Digit;
    if (isalpha(c)) return CharType::Letter;
    if (isspace(c)) {
        return CharType::Whitespace;
    }
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '=') return CharType::Operator_;
    if (c == '(' || c == ')') return CharType::Paren;
    return CharType::Other;
}

Token Lexer::getNextToken() {
    currentToken.clear();
    currentState = LexerState::Start;
    char c;

    /*սա կարդում է սիմվոլները մեկ առ մեկ*/
    while (input.get(c)) {
        column++;
        
        CharType type = getCharType(c);

        if (currentState == LexerState::Start && type == CharType::Whitespace) {
            continue;
        } /*եթե բացատներ են, իգնոր ա անում*/

        /*եթե փակագծեր են, ստեղծում է OpenParen կամ CloseParen տոկեն*/
        if (type == CharType::Paren) {
            if (!currentToken.empty()) {
                input.unget();
                column--;
                break;
            }
            currentToken = c;
            return Token(c == '(' ? TokenType::OpenParen : TokenType::CloseParen, currentToken, line, column);
        }

        /*որոշում ենք հաջորդ վիճակը մատրիցայից*/
        LexerState next = transitionMatrix[(int)currentState][(int)type];
        
        if (currentState != LexerState::Start && next == LexerState::Start) {
            input.unget();
            column--;
            break;
        } /*Օրինակ, եթե կարդում եմ 123 +, երբ տեսնում եմ +-ը հասկանում եմ, որ թիվը վերջացավ։ 
        Հետ եմ դնում +-ը, որ հաջորդ անգամ getNextToken կանչելիս այն սկսի հենց +-ից*/

        currentState = next;
        currentToken += c;

        if (currentState == LexerState::InOperator) {
            break;
        }
    }

    if (currentToken.empty()) return Token(TokenType::EndOfExpr, "", line, column);

    if (currentState == LexerState::InNumber) return Token(TokenType::Number, currentToken, line, column);
    if (currentState == LexerState::InName) return Token(TokenType::Name, currentToken, line, column);
    if (currentState == LexerState::InOperator) {
        //Ստուգում է = է, թե + - * /
        return Token(currentToken == "=" ? TokenType::Assignment : TokenType::Operator, currentToken, line, column);
    }

    return Token(TokenType::Unknown, currentToken, line, column);
}