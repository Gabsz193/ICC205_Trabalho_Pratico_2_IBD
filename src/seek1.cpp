//
// Created by luizg on 10/25/25.
//

#include <iostream>

#include "bplus_tree/arvorebmais.hpp"
#include "file_manager/FileManager.h"

int main(int argc, char *argv[]) {
    if (argc < 2) {
        std::cerr << "Erro: Digite um id para a busca. (Modo de uso): seek1 <id_do_registro>" << std::endl;
    }

    const int ORDEM_PRIMARIA = 230;
    const char* NOME_ARQUIVO_INDICE_PRIMARIO = "indice_primario.idx";

    BPlusTree<int, ORDEM_PRIMARIA> arvore_prim;
    FileManager fm(100000, 10, "dados.dat");

    FILE* arquivo_indice_primario = std::fopen(NOME_ARQUIVO_INDICE_PRIMARIO, "w+b");
    if (arquivo_indice_primario == nullptr) { perror("Erro ao criar o arquivo de indice primario"); return 1; }

    const int id = atoi(argv[1]);

    int blocos_lidos_busca = 0;

    std::cout << id << std::endl;

    DadosOffset offset_encontrado = arvore_prim.busca(arquivo_indice_primario, id, &blocos_lidos_busca);
//
    if (offset_encontrado == OFFSET_NULO) {
        std::cout << "Chave não encontrada" << std::endl;
    } else {
        Artigo art;
        fm.readFromOffset(offset_encontrado, art);
        art.imprimir();
    }

    return 0;
}
