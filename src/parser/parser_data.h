//
// Created by luizg on 17/10/2025.
//

#ifndef PARSER_DATA_H
#define PARSER_DATA_H
#include <fstream>

struct Artigo
{
    long id;
    char titulo[300];
    int ano;
    char autores[150];
    int citacoes;
    time_t atualizacao;
    char snippet[1024];
};

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
    void readLine();
};


#endif //PARSER_DATA_H
