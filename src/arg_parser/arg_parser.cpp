//
// Created by luizg on 10/26/25.
//

#include "arg_parser.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Todos os logs de erro foram desabilitador para serem tratados a partir dos status de retorno em níveis superiores
static Argument* find_def(const char* arg_str, Argument* defs, const int defs_count)
{
    for (int i = 0; i < defs_count; i++)
    {
        if (strcmp(arg_str, defs[i].long_name) == 0)
        {
            return &defs[i];
        }
        if (arg_str[0] == '-' && arg_str[1] == defs[i].short_name && arg_str[2] == '\0')
        {
            return &defs[i];
        }
    }

    return NULL;
}

int is_argument(const char* arg_str)
{
    const int is_short_arg = arg_str[0] == '-' && arg_str[2] == '\0';
    const int is_long_arg = arg_str[0] == '-' && arg_str[1] == '-';

    return is_long_arg || is_short_arg;
}

int parse_args(const int argc, char** argv, Argument* defs, const int defs_count)
{
    for (int i = 1; i < argc; i++)
    {
        Argument* def = find_def(argv[i], defs, defs_count);

        if (!def)
        {
            // fprintf(stderr, "Erro: Argumento desconhecido '%s'\n", argv[i]);
            return UNKNOWN_ARG;
        }

        def->found = 1;

        switch (def->type)
        {
        case ARG_TYPE_INT:
        case ARG_TYPE_STRING: {
            if (is_argument(argv[i + 1]))
            {
                // fprintf(stderr, "Erro: Argumento '%s' precisa de um valor, mas nenhum foi fornecido\n", argv[i]);
                return MISSING_VALUE;
            }

            char* value_str = argv[++i];

            if (def->type == ARG_TYPE_INT)
            {
                *(int*)def->value = atoi(value_str);
            }
            else if (def->type == ARG_TYPE_STRING)
            {
                *(char**)def->value = value_str;
            }

            break;
        }
        case ARG_TYPE_FLAG:
            *(int*)def->value = 1;
            break;
        }
    }

    for (int i = 0; i < defs_count; i++)
    {
        if (defs[i].required && !defs[i].found)
        {
            // fprintf(stderr, "Erro: O argumento obrigatório '%s' não foi fornecido.\n", defs[i].long_name);
            return MISSING_REQUIRED;
        }
    }

    return 0;
}

void print_usage(const char* program_name, Argument* defs, int defs_count)
{
    printf("Uso: %s [opcoes]\n", program_name);
    printf("\nOpcoes disponiveis:\n");
    for (int i = 0; i < defs_count; i++)
    {
        printf("  -%c, %-20s %s%s\n",
               defs[i].short_name,
               defs[i].long_name,
               defs[i].help_text,
               defs[i].required ? " (obrigatorio)" : "");
    }
}