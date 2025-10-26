#ifndef ARVORE_B_MAIS_HPP
#define ARVORE_B_MAIS_HPP

// --- Includes necessários para a biblioteca ---
#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <type_traits> // Para std::enable_if
#include <vector>      // Para std::vector

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

    // --- Operadores de Comparação ---
    bool operator<(const ChaveSecundaria& other) const { return std::strcmp(titulo, other.titulo) < 0; }
    bool operator==(const ChaveSecundaria& other) const { return std::strcmp(titulo, other.titulo) == 0; }
    bool operator>(const ChaveSecundaria& other) const { return std::strcmp(titulo, other.titulo) > 0; }
    bool operator>=(const ChaveSecundaria& other) const { return std::strcmp(titulo, other.titulo) >= 0; }
    bool operator<=(const ChaveSecundaria& other) const { return std::strcmp(titulo, other.titulo) <= 0; } 
    
    // --- Construtores ---
    ChaveSecundaria() { titulo[0] = '\0'; }
    ChaveSecundaria(const char* s) {
        std::strncpy(titulo, s, TAM_CHAVE_TITULO);
        titulo[TAM_CHAVE_TITULO] = '\0';
    }
};
// Helper para impressão
inline std::ostream& operator<<(std::ostream& os, const ChaveSecundaria& chave) { return os << chave.titulo; }


// --- Classe da Árvore B+ (Template) ---
template <typename T, int Ordem>
class BPlusTree {

public: 
    // --- Constantes da Ordem da Árvore ---
    static const int ORDEM = Ordem;
    static const int MAX_CHAVES = (2 * Ordem);
    static const int TAM_CHAVES = (MAX_CHAVES + 1);      
    static const int TAM_PONTUADORES = (MAX_CHAVES + 2); 

    // --- Estrutura do Nó da Árvore (Interna) ---
    struct TipoNo {
        bool eh_folha;
        int num_chaves;
        T chaves[TAM_CHAVES]; 
        BlocoOffset apontadores[TAM_PONTUADORES]; 
        BlocoOffset proximo; 
    };

private:
    // --- Funções de I/O e Manipulação em Memória (Privadas) ---

    int ler_no_do_disco(FILE* arquivo_indice, BlocoOffset offset_bloco, TipoNo* destino) {
        if (offset_bloco == OFFSET_NULO) return 0;
        std::fseek(arquivo_indice, offset_bloco, SEEK_SET);
        if (std::fread(destino, sizeof(TipoNo), 1, arquivo_indice) != 1) {
             if (std::ferror(arquivo_indice)) { perror("Erro ao ler bloco do disco"); return 0; }
             if (std::feof(arquivo_indice)) { std::cerr << "Erro: EOF inesperado ao ler bloco @" << offset_bloco << std::endl; return 0; }
             return 0;
        }
        return 1;
    }

    int escrever_no_no_disco(FILE* arquivo_indice, BlocoOffset offset_bloco, const TipoNo* fonte) {
        if (offset_bloco == OFFSET_NULO) return 0;
        std::fseek(arquivo_indice, offset_bloco, SEEK_SET);
        if (std::fwrite(fonte, sizeof(TipoNo), 1, arquivo_indice) != 1) { 
            perror("Erro ao escrever bloco no disco"); 
            return 0; 
        }
        return 1;
    }

    BlocoOffset obter_novo_offset(FILE* arquivo_indice) {
        std::fseek(arquivo_indice, 0, SEEK_END);
        return std::ftell(arquivo_indice);
    }
    
    TipoNo* criaNo(bool folha) {
        TipoNo* no = new TipoNo; 
        no->eh_folha = folha;
        no->num_chaves = 0;
        no->proximo = OFFSET_NULO;
        return no;
    }

    // --- Lógica de Inserção (Helpers) ---

    /**
     * @brief (Helper de INSERÇÃO) Encontra posição para descida em nó interno.
     * Usa (<=) para descer pela DIREITA em caso de igualdade.
     */
    int encontraPosicao(const TipoNo* no, const T& chave) {
        int pos = 0;
        while (pos < no->num_chaves && no->chaves[pos] <= chave) {
            pos++;
        }
        return pos;
    }

    void insereEmFolha(TipoNo* folha, const T& chave, DadosOffset offset_dados) {
        int pos = folha->num_chaves;
        while (pos > 0 && folha->chaves[pos - 1] > chave) {
            folha->chaves[pos] = folha->chaves[pos - 1];
            folha->apontadores[pos] = folha->apontadores[pos - 1]; 
            pos--;
        }
        folha->chaves[pos] = chave;
        folha->apontadores[pos] = offset_dados; 
        folha->num_chaves++;
    }

    void divideFolha(FILE* arquivo_indice, TipoNo* no_antigo, BlocoOffset offset_no_antigo, 
                     T* chave_promovida, BlocoOffset* offset_novo_no,
                     int* blocos_lidos_e_escritos) 
    {
        TipoNo* novo_no = criaNo(true);
        *offset_novo_no = obter_novo_offset(arquivo_indice);
        int ponto_divisao = (MAX_CHAVES + 1) / 2;
        novo_no->num_chaves = (MAX_CHAVES + 1) - ponto_divisao;
        for (int i = 0; i < novo_no->num_chaves; i++) {
            novo_no->chaves[i] = no_antigo->chaves[ponto_divisao + i];
            novo_no->apontadores[i] = no_antigo->apontadores[ponto_divisao + i];
        }
        no_antigo->num_chaves = ponto_divisao;
        novo_no->proximo = no_antigo->proximo;
        no_antigo->proximo = *offset_novo_no;
        *chave_promovida = novo_no->chaves[0];
        *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, offset_no_antigo, no_antigo);
        *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, *offset_novo_no, novo_no);
        delete novo_no;
    }

    void insereEmNoInterno(TipoNo* no, const T& chave, BlocoOffset offset_filho_direita) {
        int pos = no->num_chaves;
        while (pos > 0 && no->chaves[pos - 1] > chave) {
            no->chaves[pos] = no->chaves[pos - 1];
            no->apontadores[pos + 1] = no->apontadores[pos]; 
            pos--;
        }
        no->chaves[pos] = chave;
        no->apontadores[pos + 1] = offset_filho_direita;
        no->num_chaves++;
    }

    void divideNoInterno(FILE* arquivo_indice, TipoNo* no_antigo, BlocoOffset offset_no_antigo, 
                         T* chave_promovida, BlocoOffset* offset_novo_no,
                         int* blocos_lidos_e_escritos) 
    {
        TipoNo* novo_no = criaNo(false);
        *offset_novo_no = obter_novo_offset(arquivo_indice);
        int ponto_divisao_idx = ORDEM; 
        *chave_promovida = no_antigo->chaves[ponto_divisao_idx];
        novo_no->num_chaves = (MAX_CHAVES) - ponto_divisao_idx;
        for (int i = 0; i < novo_no->num_chaves; i++) {
            novo_no->chaves[i] = no_antigo->chaves[ponto_divisao_idx + 1 + i];
            novo_no->apontadores[i] = no_antigo->apontadores[ponto_divisao_idx + 1 + i];
        }
        novo_no->apontadores[novo_no->num_chaves] = no_antigo->apontadores[MAX_CHAVES + 1];
        no_antigo->num_chaves = ponto_divisao_idx;
        *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, offset_no_antigo, no_antigo);
        *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, *offset_novo_no, novo_no);
        delete novo_no;
    }

    BlocoOffset insereRecursivo(FILE* arquivo_indice, BlocoOffset offset_no_atual,
                                const T& chave, DadosOffset offset_dados,
                                T* chave_promovida_saida, BlocoOffset* offset_novo_filho_saida,
                                int* blocos_lidos_e_escritos) 
    {
        if (offset_no_atual == OFFSET_NULO) return OFFSET_NULO;
        TipoNo* no_atual = criaNo(false);
        *blocos_lidos_e_escritos += ler_no_do_disco(arquivo_indice, offset_no_atual, no_atual);
        if (no_atual->eh_folha) {
            insereEmFolha(no_atual, chave, offset_dados);
            if (no_atual->num_chaves > MAX_CHAVES) {
                T chave_promovida_local; BlocoOffset offset_novo_no_local;
                divideFolha(arquivo_indice, no_atual, offset_no_atual, &chave_promovida_local, &offset_novo_no_local, blocos_lidos_e_escritos);
                *chave_promovida_saida = chave_promovida_local;
                *offset_novo_filho_saida = offset_novo_no_local;
            } else {
                *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, offset_no_atual, no_atual);
                *offset_novo_filho_saida = OFFSET_NULO; 
            }
            delete no_atual;
            return *offset_novo_filho_saida; 
        } 
        else {
            int pos_descida = encontraPosicao(no_atual, chave);
            BlocoOffset offset_filho = no_atual->apontadores[pos_descida];
            T chave_promovida_filho = T(); 
            BlocoOffset offset_novo_filho_filho = OFFSET_NULO; 
            BlocoOffset retorno_recursao = insereRecursivo(arquivo_indice, offset_filho, chave, offset_dados, &chave_promovida_filho, &offset_novo_filho_filho, blocos_lidos_e_escritos);
            if (retorno_recursao != OFFSET_NULO) {
                insereEmNoInterno(no_atual, chave_promovida_filho, offset_novo_filho_filho);
                if (no_atual->num_chaves > MAX_CHAVES) {
                    T chave_promovida_local; BlocoOffset offset_novo_no_local;
                    divideNoInterno(arquivo_indice, no_atual, offset_no_atual, &chave_promovida_local, &offset_novo_no_local, blocos_lidos_e_escritos);
                    *chave_promovida_saida = chave_promovida_local;
                    *offset_novo_filho_saida = offset_novo_no_local;
                } else {
                    *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, offset_no_atual, no_atual);
                    *offset_novo_filho_saida = OFFSET_NULO;
                }
            }
            delete no_atual;
            return *offset_novo_filho_saida; 
        }
    }


    // --- Lógica de BUSCA MÚLTIPLA (Helpers) ---

    /**
     * @brief (Helper de BUSCA MÚLTIPLA) Encontra a *primeira* folha.
     * Usa (<) para descer pela ESQUERDA em caso de igualdade.
     */
    BlocoOffset encontraFolha(FILE* arquivo_indice, BlocoOffset offset_no_atual, const T& chave, int* blocos_lidos) {
        if (offset_no_atual == OFFSET_NULO) return OFFSET_NULO;
        TipoNo* no_atual = criaNo(false);
        *blocos_lidos += ler_no_do_disco(arquivo_indice, offset_no_atual, no_atual); 
        if (no_atual->eh_folha) {
            delete no_atual;
            return offset_no_atual; 
        }
        int pos = 0;
        // Usa '<' (estritamente menor) para descer pela esquerda em caso de igualdade
        while (pos < no_atual->num_chaves && no_atual->chaves[pos] < chave) {
            pos++;
        }
        BlocoOffset proximo_offset = no_atual->apontadores[pos];
        delete no_atual;
        return encontraFolha(arquivo_indice, proximo_offset, chave, blocos_lidos);
    }


    // --- Lógica de BUSCA ÚNICA (Helpers) ---

    /**
     * @brief (Helper de BUSCA ÚNICA) Busca recursiva original.
     * Usa (<=) para descer pela DIREITA (mesma lógica da inserção).
     */
    DadosOffset buscaRecursiva(FILE* arquivo_indice, BlocoOffset offset_no_atual, const T& chave, int* blocos_lidos) {
        if (offset_no_atual == OFFSET_NULO) return OFFSET_NULO;
        TipoNo* no_atual = criaNo(false);
        *blocos_lidos += ler_no_do_disco(arquivo_indice, offset_no_atual, no_atual);
        if (no_atual->eh_folha) {
            DadosOffset offset_dados = OFFSET_NULO;
            for(int i = 0; i < no_atual->num_chaves; i++) {
                if (no_atual->chaves[i] == chave) {
                    offset_dados = no_atual->apontadores[i]; 
                    break;
                }
            }
            delete no_atual;
            return offset_dados;
        }
        int pos = 0;
        while (pos < no_atual->num_chaves && no_atual->chaves[pos] <= chave) {
            pos++;
        }
        BlocoOffset proximo_offset = no_atual->apontadores[pos];
        delete no_atual;
        return buscaRecursiva(arquivo_indice, proximo_offset, chave, blocos_lidos);
    }

    // --- Funções de Impressão (Helpers) ---
    template <typename U = T>
    typename std::enable_if<std::is_integral<U>::value>::type 
    imprime_chave(const U& chave) {
        std::cout << chave << " ";
    }

    template <typename U = T>
    typename std::enable_if<std::is_same<U, ChaveSecundaria>::value>::type 
    imprime_chave(const U& chave) {
        std::cout << "'" << chave.titulo << "' ";
    }
    
    template <typename U = T>
    typename std::enable_if<!std::is_integral<U>::value && !std::is_same<U, ChaveSecundaria>::value>::type 
    imprime_chave(const U& chave) {
        std::cout << "[TipoDesconhecido] ";
    }


public:
    BlocoOffset raiz = OFFSET_NULO;

    // --- Função Pública: Inserir ---
    void insere(FILE* arquivo_indice, const T& chave, DadosOffset offset_dados, int* blocos_lidos_e_escritos) {
        *blocos_lidos_e_escritos = 0; 
        if (raiz == OFFSET_NULO) { 
            TipoNo* nova_raiz = criaNo(true); 
            raiz = obter_novo_offset(arquivo_indice);
            *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, raiz, nova_raiz);
            delete nova_raiz; 
        }
        T chave_promovida;
        BlocoOffset offset_novo_filho = OFFSET_NULO;
        BlocoOffset offset_retorno = insereRecursivo(arquivo_indice, raiz, chave, offset_dados, &chave_promovida, &offset_novo_filho, blocos_lidos_e_escritos); 
        if (offset_retorno != OFFSET_NULO) {
            TipoNo* nova_raiz = criaNo(false); 
            nova_raiz->chaves[0] = chave_promovida;
            nova_raiz->apontadores[0] = raiz; 
            nova_raiz->apontadores[1] = offset_novo_filho; 
            nova_raiz->num_chaves = 1;
            BlocoOffset novo_offset_raiz = obter_novo_offset(arquivo_indice);
            *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, novo_offset_raiz, nova_raiz);
            raiz = novo_offset_raiz; 
            delete nova_raiz;
        }
    }


    // --- FUNÇÃO PÚBLICA: BUSCA ÚNICA (Para chaves primárias) ---
    DadosOffset busca(FILE* arquivo_indice, const T& chave, int* blocos_lidos) {
        *blocos_lidos = 0;
        return buscaRecursiva(arquivo_indice, raiz, chave, blocos_lidos);
    }


    // --- FUNÇÃO PÚBLICA: BUSCA MÚLTIPLA (Para chaves secundárias) ---
    std::vector<DadosOffset> buscaMultipla(FILE* arquivo_indice, const T& chave, int* blocos_lidos) {
        std::vector<DadosOffset> resultados;
        *blocos_lidos = 0; 
        
        BlocoOffset offset_folha_atual = encontraFolha(arquivo_indice, raiz, chave, blocos_lidos);

        if (offset_folha_atual == OFFSET_NULO) {
            return resultados; // Árvore vazia
        }

        bool encontrou_maior = false; // Flag de parada global
        
        // 2. Itera pelos nós folha (enquanto houver nós e não tivermos encontrado uma chave maior)
        while (offset_folha_atual != OFFSET_NULO && !encontrou_maior) {
            
            TipoNo* no_folha = criaNo(true);
            *blocos_lidos += ler_no_do_disco(arquivo_indice, offset_folha_atual, no_folha);
            
            // 3. Varre as chaves dentro do nó folha atual
            for (int i = 0; i < no_folha->num_chaves; i++) {
                
                if (no_folha->chaves[i] == chave) {
                    resultados.push_back(no_folha->apontadores[i]);
                    // Continua o 'for'
                } 
                else if (no_folha->chaves[i] > chave) {
                    encontrou_maior = true; // Para TUDO (loop 'while' e 'for')
                    break; // Para o loop 'for'
                }
                // Se (no_folha->chaves[i] < chave), o loop 'for' simplesmente continua
            }

            // 4. Pega o próximo offset ANTES de deletar
            offset_folha_atual = no_folha->proximo;
            delete no_folha;
        }

        return resultados;
    }


    // --- Função Pública: Imprimir (para Debug) ---
    void imprime(FILE* arquivo_indice, BlocoOffset offset, int nivel) {
        if (offset == OFFSET_NULO) return;
        TipoNo* no = criaNo(false);
        if (ler_no_do_disco(arquivo_indice, offset, no) == 0) { delete no; return; }
        
        for (int i = 0; i < nivel; ++i) std::cout << "    "; 
        std::cout << "Nivel " << nivel << " (" << (no->eh_folha ? "folha" : "interno") << ") @" << offset << ": ";
        
        for (int i = 0; i < no->num_chaves; i++) {
            imprime_chave(no->chaves[i]);
            if (no->eh_folha) std::cout << "(" << no->apontadores[i] << ") ";
        }
        std::cout << std::endl;
        
        if (no->eh_folha && no->proximo != OFFSET_NULO) {
            for (int i = 0; i < nivel; ++i) std::cout << "    "; 
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