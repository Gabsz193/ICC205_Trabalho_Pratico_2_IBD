//
// Created by luizg on 10/25/25.
//

#include "FileManager.h"

FileManager::FileManager(int nBuckets, int fBloqueio, const std::string &filename):
    nomeDoArquivo(filename), numBuckets(nBuckets), fatorBloqueio(fBloqueio)
{
    this->tamanhoBucket = TAMANHO_CABECALHO_BUCKET + (long)fBloqueio * TAMANHO_REGISTRO;
    // std::remove(nomeDoArquivo.c_str());
    // this->inicializarArquivo();
}

void FileManager::inicializarArquivo() {
    std::fstream arquivo(this->nomeDoArquivo, std::ios::out | std::ios::binary | std::ios::trunc);
    if (!arquivo.is_open()) {
        std::cerr << "Erro ao inicializar o arquivo de dados." << std::endl;
        return;
    }

    BucketHeader vazioHeader;
    std::vector<char> bucketBuffer(this->tamanhoBucket, 0);

    std::memcpy(bucketBuffer.data(), &vazioHeader, TAMANHO_CABECALHO_BUCKET);

    for (int i = 0; i < this->numBuckets; i++) {
        arquivo.write(bucketBuffer.data(), this->tamanhoBucket);
    }

    this->offsetAreaOverflow = (long)this->numBuckets * this->tamanhoBucket;

    arquivo.close();

    std::cout << "[FM] Arquivo configurado: N=" << this->numBuckets
    << ", C=" << this->fatorBloqueio << ", TamBuckets=" << this->tamanhoBucket << " bytes." << std::endl;
}

bool FileManager::readBucket(long offset, std::vector<Artigo> &registros, BucketHeader &header) {
    std::ifstream arquivo(this->nomeDoArquivo, std::ios::binary);

    if (!arquivo.is_open()) return false;

    arquivo.seekg(offset);
    arquivo.read(reinterpret_cast<char*>(&header), TAMANHO_CABECALHO_BUCKET);

    registros.resize(fatorBloqueio);
    arquivo.read(reinterpret_cast<char*>(registros.data()), (long)this->fatorBloqueio * TAMANHO_REGISTRO);

    arquivo.close();
    return true;
}

bool FileManager::writeBucket(long offset, const std::vector<Artigo> &registros, const BucketHeader &header) {
    std::fstream arquivo(this->nomeDoArquivo, std::ios::in | std::ios::out | std::ios::binary);
    if (!arquivo.is_open()) return false;

    arquivo.seekp(offset);
    arquivo.write(reinterpret_cast<const char*>(&header), TAMANHO_CABECALHO_BUCKET);
    arquivo.write(reinterpret_cast<const char*>(registros.data()), (long)this->fatorBloqueio * TAMANHO_REGISTRO);

    arquivo.close();
    return true;
}

long FileManager::allocateOverflowBucket(const BucketHeader &newHeader) {
    std::fstream arquivo(this->nomeDoArquivo, std::ios::out | std::ios::binary | std::ios::in | std::ios::app);
    if (!arquivo.is_open()) return ENDERECO_VAZIO;

    arquivo.seekp(0, std::ios::end);
    long new_offset = arquivo.tellp();

    std::vector<char> bucketBuffer(this->tamanhoBucket, 0);

    std::memcpy(bucketBuffer.data(), &newHeader, TAMANHO_CABECALHO_BUCKET);
    arquivo.write(bucketBuffer.data(), tamanhoBucket);

    std::cout << "[FM] Novo Bucket de Overflow alocado no offset: " << new_offset << std::endl;
    return new_offset;
}

long FileManager::inserirRegistro(const Artigo &art) {
    int hashValue = hash(art.id);
    long currentOffset = getBucketOffset(hashValue);
    BucketHeader currentHeader;

    std::vector<Artigo> currentRegistros;

    long slotOffset = -1;
    long lastBucketOffset = ENDERECO_VAZIO;

    while (currentOffset != ENDERECO_VAZIO) {
        if (!readBucket(currentOffset, currentRegistros, currentHeader)) return ENDERECO_VAZIO;

        lastBucketOffset = currentOffset;

        if (currentHeader.count < this->fatorBloqueio) {
            for (int i = 0; i < fatorBloqueio; i++) {
                if (currentRegistros[i].id == 0) {
                    currentRegistros[i] = art;
                    currentHeader.count++;
                    this->writeBucket(currentOffset, currentRegistros, currentHeader);

                    slotOffset = currentOffset + TAMANHO_CABECALHO_BUCKET + (long)i * TAMANHO_REGISTRO;

                     // std::cout << "[FM] Inserido ID " << art.id << " no offset: " << slotOffset
                     // << " (Bucket: " << currentOffset << ")." << std::endl;

                    return slotOffset;
                }
            }
        }
        currentOffset = currentHeader.overflow_ptr;
    }

    BucketHeader newHeader;
    newHeader.count = 1;

    long newBucketOffset = this->allocateOverflowBucket(newHeader);
    if (newBucketOffset == ENDERECO_VAZIO) return ENDERECO_VAZIO;

    currentHeader.overflow_ptr = newBucketOffset;
    writeBucket(lastBucketOffset, currentRegistros, currentHeader);

    std::vector<Artigo> newRegistros(fatorBloqueio);
    newRegistros[0] = art;
    writeBucket(newBucketOffset, newRegistros, newHeader);

    slotOffset = newBucketOffset + TAMANHO_CABECALHO_BUCKET + 0 * TAMANHO_REGISTRO;

    std::cout << "[FM] Colisão! ID " << art.id << " inserido no novo Overflow Bucket: "
    << newBucketOffset << ". Offset do registro: " << slotOffset << std::endl;

    return slotOffset;
}

long FileManager::buscarRegistro(int id, Artigo &art) {
    int hashValue = this->hash(id);
    long currentOffset = this->getBucketOffset(hashValue);

    BucketHeader currentHeader;
    std::vector<Artigo> currentRegistros;

    while (currentOffset != ENDERECO_VAZIO) {
        if (!this->readBucket(currentOffset, currentRegistros, currentHeader)) return false;

        for (int i = 0; i < currentHeader.count; i++) {
            if (currentRegistros[i].id == id) {
                art = currentRegistros[i];
                return true;
            }
        }

        currentOffset = currentHeader.overflow_ptr;
    }

    return false;
}

bool FileManager::readFromOffset(long offset, Artigo &art) const {
    std::ifstream arquivo(this->nomeDoArquivo, std::ios::binary);

    if (!arquivo.is_open()) return false;

    arquivo.seekg(offset);

    arquivo.read(reinterpret_cast<char*>(&art), TAMANHO_REGISTRO);

    arquivo.close();
    return true;
}
