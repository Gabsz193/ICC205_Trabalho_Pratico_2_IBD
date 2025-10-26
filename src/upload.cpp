//
// Created by luizg on 10/25/25.
//

#include <iostream>

#include "arg_parser/arg_parser.h"
#include "bplus_tree/arvorebmais.hpp"
#include "file_manager/FileManager.h"
#include "logger/logger.h"
#include "parser/parser_data.h"

const char* PROGRAM_NAME = "upload";

int main(int argc, char *argv[]) {

    logger(PROGRAM_NAME, "Iniciando programa");

    char* indice_prim_filename = NULL;
    char* indice_sec_filename = NULL;
    char* dados_filename = NULL;
    char* input_filename = NULL;

    Argument defs[] = {
        {
            .long_name = "--indice-prim",
            .short_name = 'p',
            .help_text = "Nome do arquivo de índice primário",
            .type = ARG_TYPE_STRING,
            .required = 1,
            .value = &indice_prim_filename,
            .found = 0
        },
        {
            .long_name = "--indice-sec",
            .short_name = 's',
            .help_text = "Nome do arquivo de índice secundário",
            .type = ARG_TYPE_STRING,
            .required = 1,
            .value = &indice_sec_filename,
            .found = 0
        },
        {
            .long_name = "--input",
             .short_name = 'i',
             .help_text = "Caminho para o arquivo de dados de input",
             .type = ARG_TYPE_STRING,
             .required = 1,
             .value = &input_filename,
             .found = 0
        },
        {
            .long_name = "--dados",
             .short_name = 'd',
             .help_text = "Nome do arquivo de dados organizado por hashing",
             .type = ARG_TYPE_STRING,
             .required = 1,
             .value = &dados_filename,
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
    const int ORDEM_SECUNDARIA = 6;

    FileManager fm(1000, 4, dados_filename);
    fm.inicializarArquivo();

    BPlusTree<int, ORDEM_PRIMARIA> arvore_prim;
    BPlusTree<ChaveSecundaria, ORDEM_SECUNDARIA> arvore_sec;

    FILE* arquivo_indice_primario = std::fopen(indice_prim_filename, "w+b");
    if (arquivo_indice_primario == nullptr) { perror("Erro ao criar o arquivo de indice primario"); return 1; }
    FILE* arquivo_indice_secundario = std::fopen(indice_sec_filename, "w+b");
    if (arquivo_indice_secundario == nullptr) { perror("Erro ao criar o arquivo de indice secundario"); std::fclose(arquivo_indice_primario); return 1; }

    auto parser = DataParser(input_filename);

    parser.open();

    logger(PROGRAM_NAME, "Arquivos de configuração setados. Começando a indexação dos dados, aguarde.");

    auto art = parser.readLine();

    while (art.id != 0)
    {
        const long offset_reg = fm.inserirRegistro(art);

        int blocos_op;

        arvore_prim.insere(arquivo_indice_primario, art.id, offset_reg, &blocos_op);
        arvore_sec.insere(arquivo_indice_secundario, art.titulo, offset_reg, &blocos_op);

        art = parser.readLine();
    }

    parser.close();

    return 0;
}
