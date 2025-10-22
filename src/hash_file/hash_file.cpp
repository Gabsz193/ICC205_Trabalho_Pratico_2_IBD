#include "hash_file.h"
#include <iostream>

HashFile::HashFile(const std::string& fname, int buckets)
    : filename(fname), num_buckets(buckets)
{
}

HashFile::~HashFile()
{
    close();
}

long HashFile::hash(long id) const
{
    return id % num_buckets;
}

bool HashFile::open()
{
    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);

    if (!file.is_open())
    {
        // Arquivo não existe, cria novo
        file.open(filename, std::ios::out | std::ios::binary);
        file.close();
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    }

    return file.is_open();
}

void HashFile::close()
{
    if (file.is_open())
    {
        file.close();
    }
}

long HashFile::insert(const Artigo& artigo)
{
    if (!file.is_open())
    {
        std::cerr << "Erro: arquivo não está aberto" << std::endl;
        return -1;
    }

    // Calcula posição baseada no hash
    long bucket = hash(artigo.id);
    long offset = bucket * sizeof(Artigo);

    // Busca próxima posição livre (tratamento de colisão linear)
    Artigo temp;
    file.seekg(offset);

    while (file.read(reinterpret_cast<char*>(&temp), sizeof(Artigo)))
    {
        if (temp.id == 0)
        {
            // Posição livre encontrada
            break;
        }
        offset += sizeof(Artigo);
        file.seekg(offset);
    }

    // Grava o artigo na posição encontrada
    file.clear(); // Limpa flags de erro/EOF
    file.seekp(offset);
    file.write(reinterpret_cast<const char*>(&artigo), sizeof(Artigo));
    file.flush();

    return offset;
}

bool HashFile::read(long offset, Artigo& artigo)
{
    if (!file.is_open())
    {
        std::cerr << "Erro: arquivo não está aberto" << std::endl;
        return false;
    }

    file.clear();
    file.seekg(offset);
    file.read(reinterpret_cast<char*>(&artigo), sizeof(Artigo));

    return file.gcount() == sizeof(Artigo);
}
