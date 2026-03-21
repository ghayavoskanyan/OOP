/*ծրագրի հիշողությունն է*/
#include "SymbolTable.h"

/*
nameToIndex (std::map) -> Սա բառարան է, որն ասում է w անունով փոփոխականը գտնվում է 5-րդ համարի տակ
values (std::vector) -> Սա հենց հիշողությունն է, որտեղ 5-րդ համարի տակ պահված է իրական թիվը, օրինակ 8.0
*/

/*Սա թույլ է տալիս VM-ին չաշխատել string-ների հետ։ VM-ը պարզապես ասում է տուր ինձ 0-րդ փոփոխականը, 
և դա շատ ավելի արագ է*/
size_t SymbolTable::getIndex(const std::string& name) {
    auto it = nameToIndex.find(name); 
    if (it == nameToIndex.end()) { //Եթե այս անունով փոփոխական դեռ չկա
        size_t idx = values.size(); //Վերցնում ենք հաջորդ ազատ համարը
        nameToIndex[name] = idx; //Կպցնում ենք անունը այդ համարին
        values.push_back(0.0); //Ստեղծում ենք տեղ վեկտորի մեջ (սկզբնական 0)
        return idx;
    }
    return it->second; //Եթե արդեն կար, ուղղակի ասում ենք իր համարը
}

bool SymbolTable::getValue(const std::string& name, double& value) {
    auto it = nameToIndex.find(name);
    if (it == nameToIndex.end()) return false; //Եթե չկա նման անուն
    value = values[it->second]; //Գտնում ենք համարը ու վերցնում թիվը
    return true;
}

/*Այս 2 ֆունկցիաները շատ արագ են։ VM-ը չի փնտրում անունով, նա ուղղակի ասում է տուր ինձ 
ինդեքս idx-ի տակ եղած թիվը -> Direct Access*/
double SymbolTable::getValueByIndex(size_t idx) const {
    if (idx < values.size()) {
        return values[idx];
    }
    return 0.0;
}

void SymbolTable::setValueByIndex(size_t idx, double val) {
    if (idx < values.size()) {
        values[idx] = val;
    }
}