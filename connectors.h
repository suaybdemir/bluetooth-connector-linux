#ifndef CONNECTORS_H
#define CONNECTORS_H
#include <iostream>
#include <fstream>
#include <string>

typedef struct connectors
{
    std::fstream db;
    std::string line;
    std::string temp;
    int lNum;
}CONNECTION_CONNECTORS;

#endif