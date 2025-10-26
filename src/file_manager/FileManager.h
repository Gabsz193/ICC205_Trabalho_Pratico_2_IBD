//
// Created by luizg on 10/25/25.
//

#ifndef FILEMANAGER_H
#define FILEMANAGER_H
#include <string>
#include <vector>

#include "../artigo/artigo.h"

const long ENDERECO_VAZIO = -1;

struct BucketHeader {
    long overflow_ptr;
    int count;

    BucketHeader(): overflow_ptr(ENDERECO_VAZIO), count(0) {}
};

const long TAMANHO_CABECALHO_BUCKET = sizeof(BucketHeader);

class FileManager {
private:
    std::string nomeDoArquivo;
    int numBuckets;
    int fatorBloqueio;
    long tamanhoBucket;
    long offsetAreaOverflow;

    int hash(int id) const {
        return id % numBuckets;
    }

    long getBucketOffset(int hashValue) const {
        return (long)hashValue * tamanhoBucket;
    }

    bool writeBucket(long offset, const std::vector<Artigo>& registros, const BucketHeader& header);

    bool readBucket(long offset, std::vector<Artigo>& artigos, BucketHeader& header);

    long allocateOverflowBucket(const BucketHeader& newHeader);
public:
    FileManager(int nBuckets, int fBloqueio, const std::string& filename);

    void inicializarArquivo();

    long getTamanhoBucket() const { return this->tamanhoBucket; }

    int getNumBuckets() const { return this->numBuckets; }

    long inserirRegistro(const Artigo& art);

    long buscarRegistro(int id, Artigo& art);

    bool readFromOffset(long offset, Artigo& art) const;
};


#endif //FILEMANAGER_H