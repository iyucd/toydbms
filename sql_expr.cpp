#include <iostream>
#include "sql_expr.h"


UCD::SQLExpression::SQLExpression() {
    this->clear();
}

UCD::SQLExpression::~SQLExpression() {
    //delete(this->child[0]);
    //this->child[0] = nullptr;
    //delete(this->child[1]);
    //this->child[1] = nullptr;
}

void UCD::SQLExpression::clear() {
    this->index = -1;
    this->func = -1;
    this->count = -1;
    this->type[0] = -1;
    this->type[1] = -1;
    this->data[0] = "";
    this->data[1] = "";
    this->alt_data[0] = "";
    this->alt_data[1] = "";
    this->num[0] = -1;
    this->num[1] = -1;
    this->child[0] = nullptr;
    this->child[1] = nullptr;
}

int UCD::SQLExpression::getFunc() {
    return this->func;
}

void UCD::SQLExpression::setFunc(int val) {
    this->func = val;
}

int UCD::SQLExpression::getCount() {
    return this->count;
}

void UCD::SQLExpression::setCount(int val) {
    this->count = val;
}

std::string UCD::SQLExpression::getName(int idx) {
    if(this->type[idx] != 0) return "";
    if(idx < 0 || idx > 2)
        return nullptr;
    return this->data[idx];
}

void UCD::SQLExpression::setName(int idx, std::string str) {
    this->type[idx] = 0;
    if(idx < 0 || idx > 2) {
        std::cout << "Out of index when setName value in node got index " << idx;
        return;
    }
    this->data[idx] = str;
}

std::string UCD::SQLExpression::getData(int idx) {
    if(this->type[idx] != 1) return "";
    if(idx < 0 || idx > 2)
        return nullptr;
    return this->data[idx];
}

void UCD::SQLExpression::setData(int idx, std::string str) {
    this->type[idx] = 1;
    if(idx < 0 || idx > 2) {
        std::cout << "Out of index when setData value in node got index " << idx;
        return;
    }
    this->data[idx] = str;
}

std::string UCD::SQLExpression::getAltData(int idx) {
    if(this->type[idx] != 0) return "";
    if(idx < 0 || idx > 2)
        return nullptr;
    return this->alt_data[idx];
}

void UCD::SQLExpression::setAltData(int idx, std::string str) {
    if(idx < 0 || idx > 2) {
        std::cout << "Out of index when setAltData value in node got index " << idx;
        return;
    }
    this->alt_data[idx] = str;
}

int UCD::SQLExpression::getNum(int idx) {
    if(this->type[idx] != 2) return -1;
    if(idx < 0 || idx > 2)
        return -1;
    return this->num[idx];
}

void UCD::SQLExpression::setNum(int idx, int val) {
    this->type[idx] = 2;
    if(idx < 0 || idx > 2) {
        std::cout << "Out of index when setNum value in node got index " << idx;
        return;
    }
    this->num[idx] = val;
}

UCD::SQLExpression* UCD::SQLExpression::getExpression(int idx) {
    if(this->type[idx] != 3) return nullptr;
    if(idx < 0 || idx > 2)
        return nullptr;
    return this->child[idx];
}

void UCD::SQLExpression::setExpression(int idx, UCD::SQLExpression* exp) {
    this->type[idx] = 3;
    if(idx < 0 || idx > 2) {
        std::cout << "Out of index when setExpression value in node got index " << idx;
        return;
    }
    this->child[idx] = exp;
}

void UCD::SQLExpression::print() {
    std::cout << this-> index << "  Func: " << this->func << "\tCount: " << this->count;
    switch (this->type[0]) {
        case 0: std::cout << "\t0) Name: " << this->data[0];
                if(this->alt_data[0].compare("") != 0)
                    std::cout << "\t Table: " << this->alt_data[0];
                break;
        case 1: std::cout << "\t0) Data: " << this->data[0];    break;
        case 2: std::cout << "\t0) Num: " << this->num[0];      break;
        case 3:
            if(this->child[0])
                std::cout << "\t0) Children " << this->child[0]->index;
            break;
        default: break;
    }

    switch (this->type[1]) {
        case 0: std::cout << "\t1) Name: " << this->data[1];
            if(this->alt_data[1].compare("") != 0)
                std::cout << "\t Table: " << this->alt_data[1];
            break;
        case 1: std::cout << "\t1) Data: " << this->data[1];    break;
        case 2: std::cout << "\t1) Num: "  << this->num[1];     break;
        case 3:
            if(this->child[1])
                std::cout << "\t1) Children " << this->child[1]->index;
            break;
        default: break;
    }
    std::cout << std::endl;
}
