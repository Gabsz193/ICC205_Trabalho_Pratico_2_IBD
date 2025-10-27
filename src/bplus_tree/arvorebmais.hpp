
#ifndef ARVORE_B_MAIS_HPP
#define ARVORE_B_MAIS_HPP

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <cstdio>
#include <type_traits>
#include <vector>

#define TAMANHO_BLOCO 4096

typedef long BlocoOffset; 
const BlocoOffset OFFSET_NULO = -1L;
typedef BlocoOffset DadosOffset; 

typedef char Bloco[TAMANHO_BLOCO];

const int TAM_CHAVE_TITULO = 300;
struct ChaveSecundaria {
    char titulo[TAM_CHAVE_TITULO + 1]; 

    bool operator<(const ChaveSecundaria& other) const { return std::strcmp(titulo, other.titulo) < 0; }
    bool operator==(const ChaveSecundaria& other) const { return std::strcmp(titulo, other.titulo) == 0; }
    bool operator>(const ChaveSecundaria& other) const { return std::strcmp(titulo, other.titulo) > 0; }
    bool operator>=(const ChaveSecundaria& other) const { return std::strcmp(titulo, other.titulo) >= 0; }
    bool operator<=(const ChaveSecundaria& other) const { return std::strcmp(titulo, other.titulo) <= 0; } 

    ChaveSecundaria() { titulo[0] = '\0'; }
    ChaveSecundaria(const char* s) {
        std::strncpy(titulo, s, TAM_CHAVE_TITULO);
        titulo[TAM_CHAVE_TITULO] = '\0';
    }
};

inline std::ostream& operator<<(std::ostream& os, const ChaveSecundaria& chave) { 
    return os << chave.titulo; 
}

template <typename T, int Ordem>
class BPlusTree {

public: 
    static const int ORDEM = Ordem;
    static const int MAX_CHAVES = (2 * Ordem);
    static const int TAM_CHAVES = (MAX_CHAVES + 1);      
    static const int TAM_PONTUADORES = (MAX_CHAVES + 2); 

    struct TipoNo {
        bool eh_folha;
        int num_chaves;
        T chaves[TAM_CHAVES]; 
        BlocoOffset apontadores[TAM_PONTUADORES]; 
        BlocoOffset proximo; 

        TipoNo() {
            eh_folha = true;
            num_chaves = 0;
            proximo = OFFSET_NULO;
            for (int i = 0; i < TAM_PONTUADORES; i++) {
                apontadores[i] = OFFSET_NULO;
            }
        }
    };

private:
    int ler_no_do_disco(FILE* arquivo_indice, BlocoOffset offset_bloco, TipoNo* destino) {
        if (offset_bloco == OFFSET_NULO) return 0;
        std::fseek(arquivo_indice, offset_bloco, SEEK_SET);
        if (std::fread(destino, sizeof(TipoNo), 1, arquivo_indice) != 1) {
            if (std::ferror(arquivo_indice)) { 
                perror("Erro ao ler bloco do disco"); 
                return 0; 
            }
            if (std::feof(arquivo_indice)) { 
                std::cerr << "Erro: EOF inesperado ao ler bloco @" << offset_bloco << std::endl; 
                return 0; 
            }
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
        std::fflush(arquivo_indice);
        return 1;
    }

    BlocoOffset obter_novo_offset(FILE* arquivo_indice) {
        std::fseek(arquivo_indice, 0, SEEK_END);
        return std::ftell(arquivo_indice);
    }

    TipoNo* criaNo(bool folha) {
        TipoNo* no = new TipoNo(); 
        no->eh_folha = folha;
        return no;
    }

    // FIX CRÍTICO: Usa < para navegar corretamente
    int encontraFilhoParaDescer(const TipoNo* no, const T& chave) {
        int pos = 0;
        // Avança enquanto a chave procurada for MAIOR OU IGUAL à chave do nó
        // Isso garante que descemos pelo caminho correto
        while (pos < no->num_chaves && chave >= no->chaves[pos]) {
            pos++;
        }
        return pos;
    }

    int encontraPosicaoInsercao(const TipoNo* no, const T& chave) {
        int pos = 0;
        while (pos < no->num_chaves && chave > no->chaves[pos]) {
            pos++;
        }
        return pos;
    }

    void insereEmFolha(TipoNo* folha, const T& chave, DadosOffset offset_dados) {
        int pos = encontraPosicaoInsercao(folha, chave);
        
        // Desloca elementos para a direita
        for (int i = folha->num_chaves; i > pos; i--) {
            folha->chaves[i] = folha->chaves[i - 1];
            folha->apontadores[i] = folha->apontadores[i - 1];
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
        
        // Ponto de divisão: primeira metade fica no nó antigo, segunda no novo
        int ponto_divisao = (MAX_CHAVES + 1) / 2;
        
        // Move metade superior das chaves para o novo nó
        novo_no->num_chaves = (MAX_CHAVES + 1) - ponto_divisao;
        for (int i = 0; i < novo_no->num_chaves; i++) {
            novo_no->chaves[i] = no_antigo->chaves[ponto_divisao + i];
            novo_no->apontadores[i] = no_antigo->apontadores[ponto_divisao + i];
        }
        
        // Ajusta número de chaves no nó antigo
        no_antigo->num_chaves = ponto_divisao;
        
        // Atualiza encadeamento de folhas
        novo_no->proximo = no_antigo->proximo;
        *offset_novo_no = obter_novo_offset(arquivo_indice);
        no_antigo->proximo = *offset_novo_no;
        
        // A chave promovida é uma CÓPIA da primeira chave do novo nó
        *chave_promovida = novo_no->chaves[0];
        
        // Escreve ambos os nós no disco
        *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, offset_no_antigo, no_antigo);
        *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, *offset_novo_no, novo_no);
        
        delete novo_no;
    }

    void insereEmNoInterno(TipoNo* no, const T& chave, BlocoOffset offset_filho_direita) {
        int pos = encontraPosicaoInsercao(no, chave);
        
        // Desloca chaves e ponteiros para a direita
        for (int i = no->num_chaves; i > pos; i--) {
            no->chaves[i] = no->chaves[i - 1];
            no->apontadores[i + 1] = no->apontadores[i];
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
        
        // Ponto de divisão: chave do meio SOBE
        int meio = ORDEM;
        
        // A chave do meio sobe (não fica em nenhum dos nós)
        *chave_promovida = no_antigo->chaves[meio];
        
        // Move chaves após o meio para o novo nó
        novo_no->num_chaves = MAX_CHAVES - meio;
        for (int i = 0; i < novo_no->num_chaves; i++) {
            novo_no->chaves[i] = no_antigo->chaves[meio + 1 + i];
            novo_no->apontadores[i] = no_antigo->apontadores[meio + 1 + i];
        }
        novo_no->apontadores[novo_no->num_chaves] = no_antigo->apontadores[MAX_CHAVES + 1];
        
        // Ajusta o nó antigo
        no_antigo->num_chaves = meio;
        
        // Escreve nó antigo primeiro, depois obtém offset para o novo
        *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, offset_no_antigo, no_antigo);
        *offset_novo_no = obter_novo_offset(arquivo_indice);
        *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, *offset_novo_no, novo_no);
        
        delete novo_no;
    }

    BlocoOffset insereRecursivo(FILE* arquivo_indice, BlocoOffset offset_no_atual,
                                const T& chave, DadosOffset offset_dados,
                                T* chave_promovida_saida, BlocoOffset* offset_novo_filho_saida,
                                int* blocos_lidos_e_escritos) 
    {
        if (offset_no_atual == OFFSET_NULO) {
            *offset_novo_filho_saida = OFFSET_NULO;
            return OFFSET_NULO;
        }
        
        TipoNo* no_atual = criaNo(false);
        *blocos_lidos_e_escritos += ler_no_do_disco(arquivo_indice, offset_no_atual, no_atual);
        
        if (no_atual->eh_folha) {
            // Insere na folha
            insereEmFolha(no_atual, chave, offset_dados);
            
            if (no_atual->num_chaves > MAX_CHAVES) {
                // Folha transbordou, divide
                T chave_promovida_local;
                BlocoOffset offset_novo_no_local;
                divideFolha(arquivo_indice, no_atual, offset_no_atual, 
                           &chave_promovida_local, &offset_novo_no_local, blocos_lidos_e_escritos);
                *chave_promovida_saida = chave_promovida_local;
                *offset_novo_filho_saida = offset_novo_no_local;
            } else {
                // Apenas escreve a folha atualizada
                *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, offset_no_atual, no_atual);
                *offset_novo_filho_saida = OFFSET_NULO;
            }
            
            delete no_atual;
            return *offset_novo_filho_saida;
        } 
        else {
            // Nó interno: desce recursivamente
            int pos_filho = encontraFilhoParaDescer(no_atual, chave);
            BlocoOffset offset_filho = no_atual->apontadores[pos_filho];
            
            T chave_promovida_filho;
            BlocoOffset offset_novo_filho_filho = OFFSET_NULO;
            
            BlocoOffset retorno_recursao = insereRecursivo(arquivo_indice, offset_filho, chave, offset_dados,
                                                          &chave_promovida_filho, &offset_novo_filho_filho, 
                                                          blocos_lidos_e_escritos);
            
            if (retorno_recursao != OFFSET_NULO) {
                // Filho dividiu, precisa inserir chave promovida no nó interno
                insereEmNoInterno(no_atual, chave_promovida_filho, offset_novo_filho_filho);
                
                if (no_atual->num_chaves > MAX_CHAVES) {
                    // Nó interno transbordou, divide
                    T chave_promovida_local;
                    BlocoOffset offset_novo_no_local;
                    divideNoInterno(arquivo_indice, no_atual, offset_no_atual, 
                                   &chave_promovida_local, &offset_novo_no_local, blocos_lidos_e_escritos);
                    *chave_promovida_saida = chave_promovida_local;
                    *offset_novo_filho_saida = offset_novo_no_local;
                } else {
                    // Apenas escreve o nó interno atualizado
                    *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, offset_no_atual, no_atual);
                    *offset_novo_filho_saida = OFFSET_NULO;
                }
            } else {
                // Filho não dividiu, nada a propagar
                *offset_novo_filho_saida = OFFSET_NULO;
            }
            
            delete no_atual;
            return *offset_novo_filho_saida;
        }
    }

    DadosOffset buscaRecursiva(FILE* arquivo_indice, BlocoOffset offset_no_atual, const T& chave, int* blocos_lidos) {
        if (offset_no_atual == OFFSET_NULO) return OFFSET_NULO;
        
        TipoNo* no_atual = criaNo(false);
        *blocos_lidos += ler_no_do_disco(arquivo_indice, offset_no_atual, no_atual);
        
        if (no_atual->eh_folha) {
            // Busca linear na folha
            DadosOffset offset_dados = OFFSET_NULO;
            for (int i = 0; i < no_atual->num_chaves; i++) {
                if (no_atual->chaves[i] == chave) {
                    offset_dados = no_atual->apontadores[i];
                    break;
                }
            }
            delete no_atual;
            return offset_dados;
        }
        
        // Nó interno: encontra filho correto e desce
        int pos_filho = encontraFilhoParaDescer(no_atual, chave);
        BlocoOffset proximo_offset = no_atual->apontadores[pos_filho];
        delete no_atual;
        
        return buscaRecursiva(arquivo_indice, proximo_offset, chave, blocos_lidos);
    }

    BlocoOffset encontraPrimeiraFolha(FILE* arquivo_indice, BlocoOffset offset_no_atual, int* blocos_lidos) {
        if (offset_no_atual == OFFSET_NULO) return OFFSET_NULO;
        
        TipoNo* no_atual = criaNo(false);
        *blocos_lidos += ler_no_do_disco(arquivo_indice, offset_no_atual, no_atual);
        
        if (no_atual->eh_folha) {
            delete no_atual;
            return offset_no_atual;
        }
        
        // Desce sempre pelo primeiro filho (mais à esquerda)
        BlocoOffset primeiro_filho = no_atual->apontadores[0];
        delete no_atual;
        
        return encontraPrimeiraFolha(arquivo_indice, primeiro_filho, blocos_lidos);
    }

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
        std::cout << "[Tipo] ";
    }

public:
    BlocoOffset raiz = OFFSET_NULO;

    bool carregaRaiz(FILE* arquivo_indice) {
        if (arquivo_indice == nullptr) {
            std::cerr << "Erro: arquivo_indice é NULL" << std::endl;
            return false;
        }

        std::fseek(arquivo_indice, 0, SEEK_SET);
        size_t bytes_lidos = std::fread(&raiz, sizeof(BlocoOffset), 1, arquivo_indice);

        if (bytes_lidos != 1) {
            if (std::feof(arquivo_indice)) {
                raiz = OFFSET_NULO;
                return true;
            }
            std::cerr << "Erro ao ler offset da raiz" << std::endl;
            return false;
        }

        return true;
    }

    bool salvaRaiz(FILE* arquivo_indice) {
        if (arquivo_indice == nullptr) {
            std::cerr << "Erro: arquivo_indice é NULL" << std::endl;
            return false;
        }

        std::fseek(arquivo_indice, 0, SEEK_SET);
        size_t bytes_escritos = std::fwrite(&raiz, sizeof(BlocoOffset), 1, arquivo_indice);
        std::fflush(arquivo_indice);

        if (bytes_escritos != 1) {
            std::cerr << "Erro ao escrever offset da raiz" << std::endl;
            return false;
        }

        return true;
    }

    void insere(FILE* arquivo_indice, const T& chave, DadosOffset offset_dados, int* blocos_lidos_e_escritos) {
        *blocos_lidos_e_escritos = 0;
        
        if (raiz == OFFSET_NULO) {
            // Primeira inserção: cria raiz folha
            // Reserva espaço para cabeçalho se arquivo estiver vazio
            if (std::ftell(arquivo_indice) == 0) {
                BlocoOffset raiz_temp = OFFSET_NULO;
                std::fwrite(&raiz_temp, sizeof(BlocoOffset), 1, arquivo_indice);
                std::fflush(arquivo_indice);
            }
            
            TipoNo* nova_raiz = criaNo(true);
            raiz = obter_novo_offset(arquivo_indice);
            *blocos_lidos_e_escritos += escrever_no_no_disco(arquivo_indice, raiz, nova_raiz);
            delete nova_raiz;
        }
        
        T chave_promovida;
        BlocoOffset offset_novo_filho = OFFSET_NULO;
        BlocoOffset offset_retorno = insereRecursivo(arquivo_indice, raiz, chave, offset_dados,
                                                     &chave_promovida, &offset_novo_filho, blocos_lidos_e_escritos);
        
        if (offset_retorno != OFFSET_NULO) {
            // Raiz dividiu: cria nova raiz
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

    DadosOffset busca(FILE* arquivo_indice, const T& chave, int* blocos_lidos) {
        *blocos_lidos = 0;
        return buscaRecursiva(arquivo_indice, raiz, chave, blocos_lidos);
    }

    std::vector<DadosOffset> buscaMultipla(FILE* arquivo_indice, const T& chave, int* blocos_lidos) {
        std::vector<DadosOffset> resultados;
        *blocos_lidos = 0;
        
        if (raiz == OFFSET_NULO) {
            return resultados;
        }
        
        // Encontra a primeira folha (mais à esquerda)
        BlocoOffset offset_folha_atual = encontraPrimeiraFolha(arquivo_indice, raiz, blocos_lidos);
        
        if (offset_folha_atual == OFFSET_NULO) {
            return resultados;
        }
        
        // Percorre folhas da esquerda para direita até encontrar todas as ocorrências
        while (offset_folha_atual != OFFSET_NULO) {
            TipoNo* no_folha = criaNo(true);
            *blocos_lidos += ler_no_do_disco(arquivo_indice, offset_folha_atual, no_folha);
            
            bool encontrou_chave = false;
            for (int i = 0; i < no_folha->num_chaves; i++) {
                if (no_folha->chaves[i] == chave) {
                    resultados.push_back(no_folha->apontadores[i]);
                    encontrou_chave = true;
                } else if (no_folha->chaves[i] > chave && encontrou_chave) {
                    // Já passou da chave procurada
                    delete no_folha;
                    return resultados;
                }
            }
            
            offset_folha_atual = no_folha->proximo;
            delete no_folha;
            
            // Se encontrou a chave e a próxima folha tem chaves maiores, pode parar
            // (otimização - depende de estar ordenado)
        }
        
        return resultados;
    }

    void imprime(FILE* arquivo_indice, BlocoOffset offset, int nivel) {
        if (offset == OFFSET_NULO) return;
        
        TipoNo* no = criaNo(false);
        if (ler_no_do_disco(arquivo_indice, offset, no) == 0) { 
            delete no; 
            return; 
        }
        
        for (int i = 0; i < nivel; ++i) std::cout << "  ";
        std::cout << "Nv" << nivel << " (" << (no->eh_folha ? "F" : "I") << ") @" << offset << ": ";
        
        for (int i = 0; i < no->num_chaves; i++) {
            imprime_chave(no->chaves[i]);
            if (no->eh_folha) std::cout << "[" << no->apontadores[i] << "] ";
        }
        std::cout << std::endl;
        
        if (no->eh_folha && no->proximo != OFFSET_NULO) {
            for (int i = 0; i < nivel; ++i) std::cout << "  ";
            std::cout << " -> Next @" << no->proximo << std::endl;
        }
        
        if (!no->eh_folha) {
            for (int i = 0; i <= no->num_chaves; i++) {
                imprime(arquivo_indice, no->apontadores[i], nivel + 1);
            }
        }
        
        delete no;
    }
};

#endif // ARVORE_B_MAIS_HPP