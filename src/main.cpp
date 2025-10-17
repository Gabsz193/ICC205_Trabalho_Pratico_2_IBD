//
// Created by luizg on 17/10/2025.
//

#include <iostream>

#include "parser/parser_data.h"

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        std::cerr << "Por favor, insira o nome do arquivo de entrada" << std::endl;
        return -1;
    }

    const char* arquivo_entrada = argv[1];

    DataParser parser(arquivo_entrada);

    parser.open();

    while (true)
    {
        parser.readLine();
    }

    parser.close();

    std::cout << "Olá, tudo bem??" << std::endl;

    return 0;
}
