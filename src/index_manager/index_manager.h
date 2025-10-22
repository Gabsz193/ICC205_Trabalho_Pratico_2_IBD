#ifndef INDEX_MANAGER_HPP
#define INDEX_MANAGER_HPP

#include "../hash_file/hash_file.h"
#include "../bplus_tree/bplus_tree.h"
#include <string>

// Gerencia todos os índices do sistema
class IndexManager
{
private:
    HashFile* data_file;
    BPlusTree<long>* primary_index; // Índice por ID
    BPlusTree<char[300]>* secondary_index; // Índice por Título

public:
    IndexManager(const std::string& data_filename,
                 const std::string& primary_idx_filename,
                 const std::string& secondary_idx_filename,
                 int block_size);
    ~IndexManager();

    // Inicializa todos os arquivos
    bool initialize();

    // Fecha todos os arquivos
    void close();

    // Insere um artigo em todos os arquivos
    bool insertArticle(const Artigo& artigo);

    // Busca por ID
    bool searchById(long id, Artigo& artigo);

    // Busca por título
    bool searchByTitle(const char* titulo, Artigo& artigo);
};

#endif // INDEX_MANAGER_HPP
