//
// Created by luizg on 10/25/25.
//

#include <iostream>

#include "parser/parser_data.h"

int main(int argc, char *argv[]) {

    auto parser = DataParser("data/artigo.csv");

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

    std::cout << "Seek2" << std::endl;

    return 0;
}
