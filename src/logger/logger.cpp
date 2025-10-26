//
// Created by luizg on 10/26/25.
//

#include "logger.h"

#include <stdio.h>
#include <string.h>
#include <time.h>

void logger(const char* tipo, const char* acao)
{
    const time_t now = time(NULL);

    char* time_str = ctime(&now);

    // Retira o \n no final da string
    time_str[strlen(time_str) - 1] = '\0';

    printf("[%s] (%s): %s\n", time_str, tipo, acao);
}