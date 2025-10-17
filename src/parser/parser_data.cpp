//
// Created by luizg on 17/10/2025.
//

#include "parser_data.h"

#include <fstream>
#include <iostream>
#include <regex>
#include <string>

DataParser::DataParser(const char* arquivo_entrada)
{
    this->arquivo_entrada = arquivo_entrada;
}

void DataParser::open()
{
    std::ifstream abre_arquivo(this->arquivo_entrada);
    this->arquivo = std::move(abre_arquivo);
}

void DataParser::close()
{
    arquivo.close();
}


void DataParser::readLine()
{
    if (!this->arquivo.is_open())
    {
        std::cerr << "Ocorreu um erro ao ler a linha do arquivo" << std::endl;
        return;
    }

    std::string line;

    const std::regex pattern("\"((?:[^\"]*(?:\"\")?)*)\"|(NULL)|;();");

    std::vector<int> linhas_com_problema({
        291620, 331166, 386464, 558904,
    });

    arquivo.seekg(this->current_pos);
    std::getline(this->arquivo, line);
    this->current_line++;

    // Corrigin os erros
    if (const auto it = std::find(linhas_com_problema.begin(), linhas_com_problema.end(), this->current_line); it !=
        linhas_com_problema.end())
    {
        this->current_line++;
        std::string aux_line;
        std::getline(this->arquivo, aux_line);
        line.pop_back();

        line += aux_line;
    }

    this->current_pos = arquivo.tellg();

    const auto words_begin = std::sregex_iterator(line.begin(), line.end(), pattern);
    const auto words_end = std::sregex_iterator();


    for (auto it = words_begin; it != words_end; ++it)
    {
        std::smatch match(*it);
        std::string captured_group = match[1].str();

        std::cout << "-> " << captured_group << std::endl;
    }
}
