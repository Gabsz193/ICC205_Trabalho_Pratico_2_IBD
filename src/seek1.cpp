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

    const int ORDEM_PRIMARIA = 254;
    const char* NOME_ARQUIVO_INDICE_PRIMARIO = "prim.idx";

    BPlusTree<int, ORDEM_PRIMARIA> arvore_prim;
    FileManager fm(1000, 4, "data.dat");

    FILE* arquivo_indice_primario = std::fopen(NOME_ARQUIVO_INDICE_PRIMARIO, "r+b");
    if (arquivo_indice_primario == nullptr) { perror("Erro ao criar o arquivo de indice primario"); return 1; }

    if (!arvore_prim.carregaRaiz(arquivo_indice_primario)) {
        std::cerr << "Erro ao carregar raiz da árvore!" << std::endl;
        std::fclose(arquivo_indice_primario);
        return 1;
    }

    const int id = atoi(argv[1]);

    int blocos_lidos_busca = 0;

    DadosOffset offset_encontrado = arvore_prim.busca(arquivo_indice_primario, id, &blocos_lidos_busca);

    if (offset_encontrado == OFFSET_NULO) {
        std::cout << "Chave não encontrada" << std::endl;
    } else {
        Artigo art;
        fm.readFromOffset(offset_encontrado, art);
        art.imprimir();
    }

    return 0;
}
