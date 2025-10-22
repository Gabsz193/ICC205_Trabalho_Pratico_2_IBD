#ifndef HASH_FILE_HPP
#define HASH_FILE_HPP

#include <fstream>
#include <string>
#include "../parser/parser_data.h"
#include <ctime>

// Classe para gerenciar arquivo de dados usando hash
class HashFile
{
private:
    std::string filename;
    std::fstream file;
    int num_buckets;

    // Função hash simples
    long hash(long id) const;

public:
    HashFile(const std::string& fname, int buckets = 1000);
    ~HashFile();

    // Abre/fecha arquivo
    bool open();
    void close();

    // Insere artigo e retorna o offset onde foi gravado
    long insert(const Artigo& artigo);

    // Busca artigo pelo offset
    bool read(long offset, Artigo& artigo);
};

#endif // HASH_FILE_HPP
