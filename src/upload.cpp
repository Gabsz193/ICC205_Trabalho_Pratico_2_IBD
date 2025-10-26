//
// Created by luizg on 10/25/25.
//

#include <iostream>

#include "bplus_tree/arvorebmais.hpp"
#include "file_manager/FileManager.h"
#include "parser/parser_data.h"

int main(int argc, char *argv[]) {

    const int ORDEM_ESCOLHIDA_PRIMARIA = 230;
    const int ORDEM_ESCOLHIDA_SECUNDARIA = 100; // Ordem pequena para forçar splits
    const char* NOME_ARQUIVO_INDICE_PRIMARIO = "indice_primario.idx";
    const char* NOME_ARQUIVO_INDICE_SECUNDARIO = "indice_secundario.idx";

    FileManager fm(100000, 10, "dados.dat");
    fm.inicializarArquivo();

    BPlusTree<int, ORDEM_ESCOLHIDA_PRIMARIA> arvore_int;
    BPlusTree<ChaveSecundaria, ORDEM_ESCOLHIDA_SECUNDARIA> arvore_sec;

    FILE* arquivo_indice_primario = std::fopen(NOME_ARQUIVO_INDICE_PRIMARIO, "w+b");
    if (arquivo_indice_primario == nullptr) { perror("Erro ao criar o arquivo de indice primario"); return 1; }
    FILE* arquivo_indice_secundario = std::fopen(NOME_ARQUIVO_INDICE_SECUNDARIO, "w+b");
    if (arquivo_indice_secundario == nullptr) { perror("Erro ao criar o arquivo de indice secundario"); std::fclose(arquivo_indice_primario); return 1; }

    auto parser = DataParser("data/artigo.csv");

    parser.open();

    auto art = parser.readLine();

    while (art.id != 0)
    {
        const long offset_reg = fm.inserirRegistro(art);
        //

        int blocos_op;

        arvore_int.insere(arquivo_indice_primario, art.id, offset_reg, &blocos_op);
        arvore_sec.insere(arquivo_indice_secundario, art.titulo, offset_reg, &blocos_op);

        art = parser.readLine();
    }

    parser.close();

    return 0;
}
