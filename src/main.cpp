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

    auto art = Artigo();
    art = parser.readLine();

    while (art.id != 0)
    {
        std::cout << "Título: " << art.titulo << std::endl;
        std::cout << "Snippet: " << art.snippet << std::endl;
        std::cout << "Autores: " << art.autores << std::endl;
        std::cout << "Citações: " << art.citacoes << std::endl;
        std::cout << "Atualização: " << art.atualizacao << std::endl;
        std::cout << "Ano: " << art.ano << std::endl;
        std::cout << "Id: " << art.id << std::endl;

        art = parser.readLine();
    }

    parser.close();

    std::cout << "Olá, tudo bem??" << std::endl;

    return 0;
}
