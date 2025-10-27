//
// Created by luizg on 10/25/25.
//

#include <iostream>

#include "arg_parser/arg_parser.h"
#include "bplus_tree/arvorebmais.hpp"
#include "file_manager/FileManager.h"
#include "logger/logger.h"

const char* PROGRAM_NAME = "seek1";

int main(int argc, char *argv[]) {
    char* dados_filename = NULL;
    char* indice_filename = NULL;
    int id;

    Argument defs[] = {
        {
            .long_name = "--arquivo-dados",
            .short_name = 'd',
            .help_text = "Nome do arquivo de dados",
            .type = ARG_TYPE_STRING,
            .required = 1,
            .value = &dados_filename,
            .found = 0
        },
        {
            .long_name = "--indice-prim",
            .short_name = 'p',
            .help_text = "Nome do arquivo de índice primário",
            .type = ARG_TYPE_STRING,
            .required = 1,
            .value = &indice_filename,
            .found = 0
        },
        {
            .long_name = "--id",
            .short_name = 'f',
            .help_text = "Id do registro a ser procurado",
            .type = ARG_TYPE_INT,
            .required = 1,
            .value = &id,
            .found = 0
        }
    };

    const int defs_count = sizeof(defs) / sizeof(Argument);

    const int status = parse_args(argc, argv, defs, defs_count);

    if (status == MISSING_REQUIRED)
    {
        logger(PROGRAM_NAME, "(Erro) Faltou algum argumento obrigatório");
        print_usage(argv[0], defs, defs_count);
        return -1;
    }

    const int ORDEM_PRIMARIA = 254;
    const char* NOME_ARQUIVO_INDICE_PRIMARIO = indice_filename;

    BPlusTree<int, ORDEM_PRIMARIA> arvore_prim;
    FileManager fm(1000, 4, dados_filename);

    FILE* arquivo_indice_primario = std::fopen(NOME_ARQUIVO_INDICE_PRIMARIO, "r+b");
    if (arquivo_indice_primario == nullptr) { perror("Erro ao criar o arquivo de indice primario"); return 1; }

    if (!arvore_prim.carregaRaiz(arquivo_indice_primario)) {
        std::cerr << "Erro ao carregar raiz da árvore!" << std::endl;
        std::fclose(arquivo_indice_primario);
        return 1;
    }

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
