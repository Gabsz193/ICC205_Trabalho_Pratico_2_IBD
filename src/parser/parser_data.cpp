//
// Created by luizg on 17/10/2025.
//

#include "parser_data.h"

#include <cstring>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>

#include "../indexer/index_manager.h"

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


Artigo DataParser::readLine()
{
    if (!this->arquivo.is_open())
    {
        std::cerr << "Ocorreu um erro ao ler a linha do arquivo" << std::endl;
        return Artigo();
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

    int idx = 0;
    auto reg = Artigo();

    for (auto it = words_begin; it != words_end; ++it)
    {
        std::smatch match(*it);
        std::string captured_group = match[1].str();

        std::cout << captured_group << std::endl;
        switch (idx)
        {
        case 0:
            reg.id = std::stoi(captured_group);
            break;
        case 1:
            strncpy(reg.titulo, captured_group.c_str(), sizeof(reg.titulo) - 1);
            reg.titulo[sizeof(reg.titulo) - 1] = '\0';
            break;
        case 2:
            reg.ano = std::stoi(captured_group);
            break;
        case 3:
            strncpy(reg.autores, captured_group.c_str(), sizeof(reg.autores) - 1);
            reg.autores[sizeof(reg.autores) - 1] = '\0';
            break;
        case 4:
            reg.citacoes = std::stoi(captured_group);
            break;
        case 5:
            reg.atualizacao = std::stol(captured_group);
            break;
        case 6:
            strncpy(reg.snippet, captured_group.c_str(), sizeof(reg.snippet) - 1);
            reg.snippet[sizeof(reg.snippet) - 1] = '\0';
            break;
        default:
            break;
        }
        idx++;
    }

    return reg;
}
