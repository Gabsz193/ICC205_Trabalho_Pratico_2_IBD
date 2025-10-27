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

    const int ORDEM_SECUNDARIA = 6;
    const char* NOME_ARQUIVO_INDICE_SECUNDARIO = "sec.idx";

    BPlusTree<ChaveSecundaria, ORDEM_SECUNDARIA> arvore_sec;
    FileManager fm(1000, 4, "data.dat");

    FILE* arquivo_indice_secundario = std::fopen(NOME_ARQUIVO_INDICE_SECUNDARIO, "r+b");
    if (arquivo_indice_secundario == nullptr) { perror("Erro ao criar o arquivo de indice secundário"); return 1; }

    if (!arvore_sec.carregaRaiz(arquivo_indice_secundario)) {
        std::cerr << "Erro ao carregar raiz da árvore!" << std::endl;
        std::fclose(arquivo_indice_secundario);
        return 1;
    }

    const ChaveSecundaria titulo(argv[1]);

    int blocos_lidos_busca = 0;

    std::vector<DadosOffset> offset_encontrado = arvore_sec.buscaMultipla(arquivo_indice_secundario, titulo, &blocos_lidos_busca);

    if (offset_encontrado.empty()) {
        std::cout << "Chave não encontrada" << std::endl;
    } else {
        for (long i : offset_encontrado) {
            Artigo art;
            fm.readFromOffset(i, art);
            art.imprimir();
        }
    }

    return 0;
}
