//
// Created by luizg on 10/26/25.
//

#ifndef ARG_PARSER_H
#define ARG_PARSER_H

typedef enum
{
    ARG_TYPE_INT,
    ARG_TYPE_STRING,
    ARG_TYPE_FLAG
} ArgType;

typedef enum
{
    SUCCESS = 1, // Tudo foi bem sucedido :)
    MISSING_REQUIRED = -1, // Faltou fornecer um argumento obrigatório
    MISSING_VALUE = -2, // Argumento fornecido não apresentou seu valor
    UNKNOWN_ARG = -3, // Argumento desconhecido foi fornecido
    INVALID_VALUE = -4 // Valor inválido foi fornecido ao argumento
} ArgReturn;

typedef struct
{
    const char* long_name;
    char short_name;
    const char* help_text;
    ArgType type;
    int required;
    void* value;
    int found;
} Argument;

/**
 * @brief Analisa os argumentos da linha de comando com base em um array de definições.
 * @param argc O contador de argumento de main()
 * @param argv O vetor de argumentos de main()
 * @param defs Um array de estruturas Argument que definem os argumentos esperados
 * @param defs_count O número de elementos no array 'defs'
 * @return 0 em caso de sucesso, -1 em caso de erro (ex. argumento faltando, valor inválido).
 */
int parse_args(int argc, char** argv, Argument* defs, int defs_count);


/**
 * @brief Imprime uma mensagem de uso padronizada com base nas definições dos argumentos.
 *
 * @param program_name O nome do executável (argv[0])
 * @param defs Um array de estruturas Argument
 * @param defs_count O número de elementos no array 'defs'
 */
void print_usage(const char* program_name, Argument* defs, int defs_count);

#endif //ARG_PARSER_H