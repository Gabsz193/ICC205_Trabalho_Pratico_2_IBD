//
// Created by luizg on 10/25/25.
//

#include <iostream>

#include "arg_parser/arg_parser.h"
#include "bplus_tree/arvorebmais.hpp"
#include "file_manager/FileManager.h"
#include "logger/logger.h"

const char* PROGRAM_NAME = "seek2";

int main(int argc, char *argv[]) {
    char* dados_filename = NULL;
    char* indice_filename = NULL;
    char* titulo = NULL;

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
            .long_name = "--indice-sec",
            .short_name = 's',
            .help_text = "Nome do arquivo de índice secundário",
            .type = ARG_TYPE_STRING,
            .required = 1,
            .value = &indice_filename,
            .found = 0
        },
        {
            .long_name = "--titulo",
            .short_name = 't',
            .help_text = "Título do registro a ser procurado",
            .type = ARG_TYPE_STRING,
            .required = 1,
            .value = &titulo,
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

    const int ORDEM_SECUNDARIA = 6;
    const char* NOME_ARQUIVO_INDICE_SECUNDARIO = indice_filename;

    BPlusTree<ChaveSecundaria, ORDEM_SECUNDARIA> arvore_sec;
    FileManager fm(1000, 4, dados_filename);

    FILE* arquivo_indice_secundario = std::fopen(NOME_ARQUIVO_INDICE_SECUNDARIO, "r+b");
    if (arquivo_indice_secundario == nullptr) { perror("Erro ao criar o arquivo de indice secundário"); return 1; }

    if (!arvore_sec.carregaRaiz(arquivo_indice_secundario)) {
        std::cerr << "Erro ao carregar raiz da árvore!" << std::endl;
        std::fclose(arquivo_indice_secundario);
        return 1;
    }

    const ChaveSecundaria _titulo(titulo);

    int blocos_lidos_busca = 0;

    std::vector<DadosOffset> offset_encontrado = arvore_sec.buscaMultipla(arquivo_indice_secundario, _titulo, &blocos_lidos_busca);

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
