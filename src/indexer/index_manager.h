//
// Created by luizg on 17/10/2025.
//

#ifndef INDEX_MANAGER_H
#define INDEX_MANAGER_H
#include <vector>

struct Registro
{
    int id;
    int idade;
    char nome[50];
};

struct RegistroIndice
{
    long apontador;
    int chave;
};

class GerenciadorIndice
{
    const char* nomeArquivoDados = "./data/dados.dat";
    const char* nomeArquivoIndice = "./data/indice.idx";
    std::vector<RegistroIndice> indice;

public:
    void criarArquivos(std::vector<Registro>& registros);

    void carregarIndice();

    long buscaBinaria(int chave) const;

    Registro lerRegistro(long posicao) const;

    void buscaCompleta(int chave);

    void listarIndice() const;
};

#endif //INDEX_MANAGER_H
