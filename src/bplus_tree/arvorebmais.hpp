#ifndef ARVORE_B_MAIS_HPP
#define ARVORE_B_MAIS_HPP

// --- Includes necessários para a biblioteca ---
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <type_traits>

// --- Constantes de E/S de Disco ---
#define TAMANHO_BLOCO 4096

typedef long BlocoOffset; 
const BlocoOffset OFFSET_NULO = -1L;
typedef BlocoOffset DadosOffset; 

typedef char Bloco[TAMANHO_BLOCO];

// --- Estrutura de Chave Secundária ---
const int TAM_CHAVE_TITULO = 300;
struct ChaveSecundaria {
    char titulo[TAM_CHAVE_TITULO + 1]; 
    bool operator<(const ChaveSecundaria& other) const { return std::strcmp(titulo, other.titulo) < 0; }
    bool operator==(const ChaveSecundaria& other) const { return std::strcmp(titulo, other.titulo) == 0; }
    bool operator>(const ChaveSecundaria& other) const { return std::strcmp(titulo, other.titulo) > 0; }
    // --- CORREÇÃO AQUI ---
    bool operator>=(const ChaveSecundaria& other) const { return std::strcmp(titulo, other.titulo) >= 0; }
    // ---------------------
    ChaveSecundaria() { titulo[0] = '\0'; }
    ChaveSecundaria(const char* s) {
        std::strncpy(titulo, s, TAM_CHAVE_TITULO);
        titulo[TAM_CHAVE_TITULO] = '\0';
    }
};
inline std::ostream& operator<<(std::ostream& os, const ChaveSecundaria& chave) { return os << chave.titulo; }


// --- Classe da Árvore B+ (Template) ---
template <typename T, int Ordem>
class BPlusTree {

public: 
    // Tornamos públicos para que a main() possa fazer os cálculos de sizeof
    // e imprimir os valores.

    // --- Constantes da Ordem da Árvore ---
    static const int ORDEM = Ordem;
    static const int MAX_CHAVES = (2 * Ordem);
    static const int TAM_CHAVES = (MAX_CHAVES + 1);      
    static const int TAM_PONTUADORES = (MAX_CHAVES + 2); 

    // --- Estrutura do Nó da Árvore (Agora interna) ---
    struct TipoNo {
        bool eh_folha;
        int num_chaves;
        T chaves[TAM_CHAVES]; // Usa a constante da classe
        // Em nós internos: ponteiros para filhos (BlocoOffset)
        // Em nós folha: offsets para dados no arquivo (DadosOffset/BlocoOffset)
        BlocoOffset apontadores[TAM_PONTUADORES]; // Usa a constante da classe
        BlocoOffset proximo; 
    };

private:
    // --- Funções de I/O de Disco ---
    int ler_no_do_disco(FILE* arquivo_indice, BlocoOffset offset, TipoNo* no_destino) {
        if (!arquivo_indice || offset == OFFSET_NULO) return 0;
        if (std::fseek(arquivo_indice, offset, SEEK_SET) != 0) return 0;
        Bloco buffer_bloco;
        if (std::fread(buffer_bloco, TAMANHO_BLOCO, 1, arquivo_indice) != 1) return 0;
        std::memcpy(no_destino, buffer_bloco, sizeof(TipoNo));
        return 1;
    }

    int escrever_no_no_disco(FILE* arquivo_indice, BlocoOffset offset, TipoNo* no_origem) {
        if (!arquivo_indice || offset == OFFSET_NULO) return 0;
        if (std::fseek(arquivo_indice, offset, SEEK_SET) != 0) return 0;
        Bloco buffer_bloco;
        std::memset(buffer_bloco, 0, TAMANHO_BLOCO); 
        std::memcpy(buffer_bloco, no_origem, sizeof(TipoNo));
        if (std::fwrite(buffer_bloco, TAMANHO_BLOCO, 1, arquivo_indice) != 1) return 0;
        std::fflush(arquivo_indice);
        return 1;
    }
    
    BlocoOffset obter_novo_offset(FILE* arquivo_indice) {
        if (!arquivo_indice) return OFFSET_NULO;
        if (std::fseek(arquivo_indice, 0, SEEK_END) != 0) return OFFSET_NULO;
        return std::ftell(arquivo_indice);
    }
    
    // --- Métodos de Manipulação em Memória ---
    TipoNo* criaNo(bool eh_folha) {
        TipoNo* no = new TipoNo();
        if (no == nullptr) { perror("Falha ao alocar memoria para o No"); exit(EXIT_FAILURE); }
        no->eh_folha = eh_folha;
        no->num_chaves = 0;
        no->proximo = OFFSET_NULO;
        for (int i = 0; i < TAM_PONTUADORES; i++) no->apontadores[i] = OFFSET_NULO;
        return no;
    }

    int encontraPosicao(TipoNo* no, const T& chave) {
        int pos = 0;
        // Esta lógica agora funciona pois ChaveSecundaria tem operator>=
        while (pos < no->num_chaves && chave >= no->chaves[pos]) {
            pos++;
        }
        return pos; 
    }
    
    void insereEmFolha(TipoNo* folha, const T& chave, DadosOffset offset_dados) {
        int pos = folha->num_chaves;
        
        while (pos > 0 && folha->chaves[pos - 1] > chave) {
            folha->chaves[pos] = folha->chaves[pos - 1];
            folha->apontadores[pos] = folha->apontadores[pos - 1]; // Deslocamento do offset de dados
            pos--;
        }
        
        folha->chaves[pos] = chave;
        folha->apontadores[pos] = offset_dados; // Armazena o offset de dados
        folha->num_chaves++;
    }

    void divideFolha(TipoNo* folha, TipoNo* nova_folha_memoria, T* chave_promovida) {
        int total = folha->num_chaves; 
        int meio = total / 2; 
        
        nova_folha_memoria->num_chaves = total - meio; 

        // Copia a segunda metade das chaves
        std::memcpy(
            nova_folha_memoria->chaves,
            &folha->chaves[meio],
            nova_folha_memoria->num_chaves * sizeof(T)
        );
        
        // Copia a segunda metade dos offsets de dados
        std::memcpy(
            nova_folha_memoria->apontadores,
            &folha->apontadores[meio],
            nova_folha_memoria->num_chaves * sizeof(DadosOffset)
        );

        folha->num_chaves = meio; 
        
        nova_folha_memoria->proximo = folha->proximo;
        folha->proximo = obter_novo_offset(nullptr); 

        *chave_promovida = nova_folha_memoria->chaves[0]; 
    }
    // ----------------------------------------------------------------

    void insereEmNoInterno(TipoNo* no, int pos, const T& chave, BlocoOffset offset_filho_direito) {
        // Deslocamento de chaves (pos em diante)
        std::memmove(&no->chaves[pos + 1], &no->chaves[pos], (no->num_chaves - pos) * sizeof(T));
        
        // Deslocamento de apontadores (pos+1 em diante)
        std::memmove(&no->apontadores[pos + 2], &no->apontadores[pos + 1], (no->num_chaves - pos) * sizeof(BlocoOffset));

        no->chaves[pos] = chave;
        no->apontadores[pos + 1] = offset_filho_direito;
        no->num_chaves++;
    }

    void divideNoInterno(TipoNo* no, TipoNo* novo_no_memoria, T* chave_promovida) {
        int total = no->num_chaves;
        int meio = total / 2; 
        
        *chave_promovida = no->chaves[meio];
        
        novo_no_memoria->num_chaves = total - meio - 1; 
        
        // Copia as chaves maiores (meio + 1 em diante)
        std::memcpy(novo_no_memoria->chaves, &no->chaves[meio + 1], novo_no_memoria->num_chaves * sizeof(T));

        // Copia os apontadores maiores (meio + 1 em diante)
        std::memcpy(novo_no_memoria->apontadores, &no->apontadores[meio + 1], (novo_no_memoria->num_chaves + 1) * sizeof(BlocoOffset));

        no->num_chaves = meio; 
    }

    BlocoOffset insereRecursivo(FILE* arquivo_indice, BlocoOffset offset_no, const T& chave, 
                                DadosOffset offset_dados, T* chave_promovida, BlocoOffset* offset_novo_filho, 
                                int* blocos_lidos_e_escritos) {
        if (offset_no == OFFSET_NULO) return OFFSET_NULO;

        TipoNo* no = criaNo(false);
        *blocos_lidos_e_escritos += ler_no_do_disco(arquivo_indice, offset_no, no);

        if (no->eh_folha) {
            insereEmFolha(no, chave, offset_dados);

            if (no->num_chaves <= MAX_CHAVES) {
                *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, offset_no, no);
                delete no;
                return OFFSET_NULO; 
            }

            // Divisão de Folha
            TipoNo* novo_filho_memoria = criaNo(true);
            divideFolha(no, novo_filho_memoria, chave_promovida);
            
            *offset_novo_filho = obter_novo_offset(arquivo_indice);

            if (no->proximo != OFFSET_NULO) novo_filho_memoria->proximo = no->proximo;
            no->proximo = *offset_novo_filho;

            *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, offset_no, no);
            *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, *offset_novo_filho, novo_filho_memoria);

            delete novo_filho_memoria;
            BlocoOffset retorno = offset_no;
            delete no;
            return retorno;
        }

        // Nó interno: descer
        int pos = encontraPosicao(no, chave);
        BlocoOffset offset_filho = no->apontadores[pos];

        T chave_promovida_filho;
        BlocoOffset offset_novo_filho_interno = OFFSET_NULO;

        // Passa offset_dados
        BlocoOffset offset_retorno_filho = insereRecursivo(arquivo_indice, offset_filho, chave, 
                                                        offset_dados, 
                                                        &chave_promovida_filho, 
                                                        &offset_novo_filho_interno,
                                                        blocos_lidos_e_escritos);

        if (offset_retorno_filho == OFFSET_NULO) { delete no; return OFFSET_NULO; }

        // Houve divisão: inserir a chave promovida no nó atual
        insereEmNoInterno(no, pos, chave_promovida_filho, offset_novo_filho_interno);
        
        if (no->num_chaves <= MAX_CHAVES) {
            *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, offset_no, no);
            delete no;
            return OFFSET_NULO;
        }
        
        // Divisão do nó interno
        TipoNo* novo_no_memoria = criaNo(false);
        divideNoInterno(no, novo_no_memoria, chave_promovida);
        
        *offset_novo_filho = obter_novo_offset(arquivo_indice);
        
        *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, offset_no, no);
        *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, *offset_novo_filho, novo_no_memoria);

        delete novo_no_memoria;
        BlocoOffset retorno = offset_no; 
        delete no;
        return retorno;
    }

    void imprime_chave(const T& chave) {
        // A checagem de tipo é feita em tempo de compilação, 
        // o compilador otimiza isso.
        if (std::is_same<T, ChaveSecundaria>::value) {
            // --- CORREÇÃO AQUI ---
            std::cout << "\"" << chave << "\" ";
        } else {
            std::cout << chave << " ";
        }
    }


public:
    BlocoOffset raiz = OFFSET_NULO;

    // Adiciona offset_dados
    void insere(FILE* arquivo_indice, const T& chave, DadosOffset offset_dados, int* blocos_lidos_e_escritos) {
        
        *blocos_lidos_e_escritos = 0; 
        
        if (raiz == OFFSET_NULO) { 
            TipoNo* nova_raiz = criaNo(true);
            raiz = obter_novo_offset(arquivo_indice);
            // --- CORREÇÃO AQUI (Contabilizar escrita) ---
            *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, raiz, nova_raiz);
            delete nova_raiz;
        }

        T chave_promovida;
        BlocoOffset offset_novo_filho = OFFSET_NULO;

        // --- CORREÇÃO AQUI (Removido o &) ---
        BlocoOffset offset_retorno = insereRecursivo(arquivo_indice, raiz, chave, offset_dados,
                                                    &chave_promovida, &offset_novo_filho, 
                                                    blocos_lidos_e_escritos); 

        if (offset_retorno != OFFSET_NULO) {
            // Nova raiz (divisão da antiga raiz)
            TipoNo* nova_raiz = criaNo(false);
            nova_raiz->chaves[0] = chave_promovida;
            nova_raiz->apontadores[0] = raiz;
            nova_raiz->apontadores[1] = offset_novo_filho;
            nova_raiz->num_chaves = 1;
            
            BlocoOffset novo_offset_raiz = obter_novo_offset(arquivo_indice);
            // --- CORREÇÃO AQUI (Contabilizar escrita) ---
            *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, novo_offset_raiz, nova_raiz);
            raiz = novo_offset_raiz;
            delete nova_raiz;
        }
    }

    // Retorna DadosOffset
    DadosOffset busca(FILE* arquivo_indice, const T& chave, int* blocos_lidos) {
        *blocos_lidos = 0; 
        return buscaRecursiva(arquivo_indice, raiz, chave, blocos_lidos);
    }
    
    // Retorna DadosOffset
    DadosOffset buscaRecursiva(FILE* arquivo_indice, BlocoOffset offset_no_atual, const T& chave, int* blocos_lidos) {
        if (offset_no_atual == OFFSET_NULO) return OFFSET_NULO;

        TipoNo* no_atual = criaNo(false);
        *blocos_lidos += ler_no_do_disco(arquivo_indice, offset_no_atual, no_atual); 

        if (no_atual->eh_folha) {
            DadosOffset offset_dados = OFFSET_NULO;
            for(int i = 0; i < no_atual->num_chaves; i++) {
                if (no_atual->chaves[i] == chave) {
                    offset_dados = no_atual->apontadores[i]; // Retorna o offset de dados
                    break;
                }
            }
            
            delete no_atual;
            return offset_dados; // Retorna OFFSET_NULO se não encontrado
        }

        // A função encontraPosicao() agora funciona para ChaveSecundaria
        int pos = encontraPosicao(no_atual, chave);
        BlocoOffset proximo_offset = no_atual->apontadores[pos];

        delete no_atual;
        
        return buscaRecursiva(arquivo_indice, proximo_offset, chave, blocos_lidos);
    }

    void imprime(FILE* arquivo_indice, BlocoOffset offset, int nivel) {
        if (offset == OFFSET_NULO) return;

        TipoNo* no = criaNo(false);
        if (ler_no_do_disco(arquivo_indice, offset, no) == 0) {
            delete no;
            return;
        }
        
        for (int i = 0; i < nivel; ++i) std::cout << "    "; // Indentação
        std::cout << "Nivel " << nivel << " (" << (no->eh_folha ? "folha" : "interno") << ") @" << offset << ": ";
        
        for (int i = 0; i < no->num_chaves; i++) {
            imprime_chave(no->chaves[i]);
            // Se for folha, imprime o offset de dados
            if (no->eh_folha) std::cout << "(" << no->apontadores[i] << ") ";
        }
        std::cout << std::endl;
        
        // Imprime o offset da folha seguinte
        if (no->eh_folha && no->proximo != OFFSET_NULO) {
            for (int i = 0; i < nivel; ++i) std::cout << "    "; // Indentação
            std::cout << " -> Prox Folha @" << no->proximo << std::endl;
        }
        
        if (!no->eh_folha) {
            for (int i = 0; i <= no->num_chaves; i++)
                imprime(arquivo_indice, no->apontadores[i], nivel + 1);
        }
        delete no;
    }
};

#endif // ARVORE_B_MAIS_HPP