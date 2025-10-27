//
// Created by luizg on 10/25/25.
//

#include <iostream>

#include "arg_parser/arg_parser.h"
#include "artigo/artigo.h"
#include "file_manager/FileManager.h"
#include "logger/logger.h"

const char* PROGRAM_NAME = "findrec";

int main(int argc, char *argv[]) {
    char* dados_filename = NULL;
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

    FileManager fm(1000, 4, dados_filename);

    Artigo art;

    fm.buscarRegistro(id, art);

    art.imprimir();

    return 0;
}
