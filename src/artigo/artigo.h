//
// Created by luizg on 10/25/25.
//

#ifndef ARTIGO_H
#define ARTIGO_H

#define MAX_SIZE_TITULO 300
#define MAX_SIZE_AUTORES 150
#define MAX_SIZE_SNIPPET 1024
#include <cstring>
#include <ios>
#include <iostream>
#include <fstream>
#include <string>

struct Artigo {
    int id;
    int ano;
    int citacoes;

    long long atualizacao;

    char titulo[MAX_SIZE_TITULO + 1];
    char autores[MAX_SIZE_AUTORES + 1];
    char snippet[MAX_SIZE_SNIPPET + 1];


    Artigo() { memset(this, 0, sizeof(Artigo)); }

    void imprimir() const;

    void preencher(
        int id,
        const std::string& titulo,
        int ano,
        const std::string& autores,
        int citacoes,
        const std::string& snippet
    );

};

const int TAMANHO_REGISTRO = sizeof(Artigo);


#endif //ARTIGO_H