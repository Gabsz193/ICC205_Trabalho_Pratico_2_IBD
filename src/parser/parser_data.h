//
// Created by luizg on 17/10/2025.
//

#ifndef PARSER_DATA_H
#define PARSER_DATA_H
#include <fstream>

#include "../artigo/artigo.h"

class DataParser
{
    const char* arquivo_entrada;
    std::ifstream arquivo;
    int current_line = 0;
    long current_pos = 0;

public:
    explicit DataParser(const char* arquivo_entrada);

    void open();
    void close();
    Artigo readLine();
};


#endif //PARSER_DATA_H
