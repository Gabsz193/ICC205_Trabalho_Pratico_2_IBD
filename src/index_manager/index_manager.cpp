#include "index_manager.h"
#include <iostream>

IndexManager::IndexManager(const std::string& data_filename,
                           const std::string& primary_idx_filename,
                           const std::string& secondary_idx_filename,
                           int block_size)
{
    data_file = new HashFile(data_filename);
    primary_index = new BPlusTree<long>(primary_idx_filename, block_size);
    secondary_index = new BPlusTree<char[300]>(secondary_idx_filename, block_size);
}

IndexManager::~IndexManager()
{
    close();
    delete data_file;
    delete primary_index;
    delete secondary_index;
}

bool IndexManager::initialize()
{
    bool success = true;

    success &= data_file->open();
    if (!success)
    {
        std::cerr << "Erro ao abrir arquivo de dados" << std::endl;
        return false;
    }

    success &= primary_index->open();
    if (!success)
    {
        std::cerr << "Erro ao abrir índice primário" << std::endl;
        return false;
    }

    success &= secondary_index->open();
    if (!success)
    {
        std::cerr << "Erro ao abrir índice secundário" << std::endl;
        return false;
    }

    return success;
}

void IndexManager::close()
{
    data_file->close();
    primary_index->close();
    secondary_index->close();
}

bool IndexManager::insertArticle(const Artigo& artigo)
{
    // 1. Insere no arquivo de dados (hash) e obtém offset
    long offset = data_file->insert(artigo);

    if (offset == -1)
    {
        std::cerr << "Erro ao inserir artigo no arquivo de dados" << std::endl;
        return false;
    }

    // 2. Insere no índice primário (ID -> offset)
    primary_index->insert(artigo.id, offset);

    // 3. Insere no índice secundário (Título -> offset)
    char titulo_copy[300];
    strncpy(titulo_copy, artigo.titulo, 300);
    titulo_copy[299] = '\0'; // Garante null termination
    secondary_index->insert(titulo_copy, offset);

    return true;
}

bool IndexManager::searchById(long id, Artigo& artigo)
{
    // Busca no índice primário
    long offset = primary_index->search(id);

    if (offset == -1)
    {
        return false;
    }

    // Lê do arquivo de dados
    return data_file->read(offset, artigo);
}

bool IndexManager::searchByTitle(const char* titulo, Artigo& artigo)
{
    // Busca no índice secundário
    char titulo_copy[300];
    strncpy(titulo_copy, titulo, 300);
    titulo_copy[299] = '\0';

    long offset = secondary_index->search(titulo_copy);

    if (offset == -1)
    {
        return false;
    }

    // Lê do arquivo de dados
    return data_file->read(offset, artigo);
}
