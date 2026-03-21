#include "Manager.h"
#include <iostream>

Manager::Manager() {
    calculator = std::make_unique<Calculator>(symbolTable); /*մենեջերը ձեռի տակ կալկուլյատոր ունենա*/
}

void Manager::setInput(const std::string& expression) {
    reset(); /*հին լեքսեր, փարսեր, տեքստեր մաքրում ա*/
    
    auto inputStream = std::make_unique<std::istringstream>(expression); /*տեքստը սարքում է կարդացվող հոսք stream*/
    
    lexer = std::make_unique<Lexer>(*inputStream); /*սարքում ա լեքսեր ու իրան տալիս էդ հոսքը*/
    parser = std::make_unique<Parser>(*lexer, symbolTable); /*սարքում ա փարսեր ու իրան տալիս փարսերն ու սիմվոլների աղյուսակը*/
    
    ownedStreams.push_back(std::move(inputStream)); /*պահում ենք հոսքը վեկտորի մեջ որ այն չջնջվի հիշողությունից քանի դեռ Lexer-ը կարդում է այն*/
}

double Manager::evaluate() { 
    if (!parser) return 0.0; /*Ստուգում ենք արդյոք պարսերը ստեղծվել է*/

    auto ast = parser->parse(); /*փարսերը տոկեններից հավաքում է ծառը AST*/
    
    if (!ast) {
        std::cerr << "Error: Failed to parse expression.\n";
        return 0.0;
    }

    /*ծառը պատրաստ է, տալիս ենք Calculator-ին*/
    return calculator->calculate(std::move(ast));
}

bool Manager::getVariable(const std::string& name, double& value) {
    return symbolTable.getValue(name, value); /*հարցնում է աղյուսակին՝ Ունե՞նք 'տվյալ' անունով փոփոխական*/
}

void Manager::reset() { /*զրոյացնում ենք որ ամեն տող գրելուց օգտագործենք*/
    lexer.reset();
    parser.reset();
    ownedStreams.clear();
}