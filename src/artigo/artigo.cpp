//
// Created by luizg on 10/25/25.
//

#include "artigo.h"

void Artigo::preencher(int id, const std::string &titulo, int ano, const std::string &autores, int citacoes, const std::string &snippet) {
    this->id = id;
    this->ano = ano;
    strcpy(this->titulo, titulo.c_str());
    strcpy(this->autores, autores.c_str());
    this->citacoes = citacoes;
    strcpy(this->snippet, snippet.c_str());
}

void Artigo::imprimir() const {
    std::cout << "------------------------------------------\n";
    std::cout << "ID: " << this->id << "\n";
    std::cout << "Título: " << this->titulo << "\n";
    std::cout << "Ano: " << this->ano << "\n";
    std::cout << "Autores: " <<this-> autores << "\n";
    std::cout << "Citações: " << this->citacoes << "\n";
    std::cout << "Atualização: " << this->atualizacao << "\n";
    std::cout << "Snippet (início): " << std::string(this->snippet).substr(0, 30) << "...\n";
    std::cout << "------------------------------------------\n";
}
