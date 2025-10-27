#include <iostream>
#include <cmath>
#include <cstddef>
#include "arvorebmais.hpp"

/**
 * @brief Calcula a ordem (M) da Árvore B+ com base nos tamanhos dos tipos.
 * * A fórmula é derivada da restrição: sizeof(TipoNo) <= TAMANHO_BLOCO
 * * Onde o tamanho do TipoNo é a soma de suas partes (ignorando padding entre membros):
 * Size(Ordem) = sizeof(bool eh_folha)
 * + sizeof(int num_chaves)
 * + sizeof(T chaves[2 * Ordem + 1])
 * + sizeof(BlocoOffset apontadores[2 * Ordem + 2])
 * + sizeof(BlocoOffset proximo)
 *
 * Size(Ordem) = (sizeof(bool) + sizeof(int) + sizeof(T) + 3 * sizeof(BlocoOffset))
 * + Ordem * (2 * sizeof(T) + 2 * sizeof(BlocoOffset))
 *
 * Chamamos:
 * Overhead_Fixo = sizeof(bool) + sizeof(int) + S_k + 3 * S_p
 * Tamanho_Por_Ordem = 2 * S_k + 2 * S_p
 *
 * Queremos o maior Ordem (M) tal que:
 * Overhead_Fixo + Ordem * Tamanho_Por_Ordem <= TAMANHO_BLOCO
 *
 * Ordem <= (TAMANHO_BLOCO - Overhead_Fixo) / Tamanho_Por_Ordem
 * * @param tamanho_bloco O tamanho total do bloco em bytes (ex: TAMANHO_BLOCO).
 * @param S_k O tamanho do tipo da chave (ex: sizeof(ChaveSecundaria)).
 * @param S_p O tamanho do tipo do apontador (ex: sizeof(BlocoOffset)).
 * @param S_bool O tamanho do tipo booleano (ex: sizeof(bool)).
 * @param S_int O tamanho do tipo inteiro (ex: sizeof(int)).
 * @return A Ordem (M) calculada. Retorna -1 em caso de erro.
 */
int calcularOrdemDaArvore(size_t tamanho_bloco, size_t S_k, size_t S_p, size_t S_bool, size_t S_int) {
    
    // Calcula o Overhead_Fixo (partes que não se repetem com a Ordem M)
    // Overhead_Fixo = sizeof(bool) + sizeof(int) + 1*S_k + 3*S_p
    double overhead_fixo = static_cast<double>(S_bool + S_int + S_k + (3 * S_p));

    // Calcula o Tamanho_Por_Ordem (partes que se multiplicam pela Ordem M)
    // Tamanho_Por_Ordem = 2*S_k + 2*S_p
    double tamanho_por_ordem = static_cast<double>((2 * S_k) + (2 * S_p));

    std::cout << "--- Parâmetros de Cálculo ---" << std::endl;
    std::cout << "Tamanho do Bloco: " << tamanho_bloco << " bytes" << std::endl;
    std::cout << "Tamanho da Chave (S_k): " << S_k << " bytes" <<std::endl;
    std::cout << "Tamanho do Apontador (S_p): " << S_p << " bytes" << std::endl;
    std::cout << "Tamanho Bool (S_bool): " << S_bool << " bytes" << std::endl;
    std::cout << "Tamanho Int (S_int): " << S_int << " bytes" << std::endl;
    std::cout << "---------------------------------" << std::endl;
    std::cout << "Overhead Fixo por Nó: " << overhead_fixo << " bytes" << std::endl;
    std::cout << "Tamanho por (Ordem): " << tamanho_por_ordem << " bytes" << std::endl;
    std::cout << "---------------------------------" << std::endl;

    if (tamanho_por_ordem == 0) {
        std::cerr << "Erro: Tamanho por ordem é zero. Divisão por zero." << std::endl;
        return -1;
    }

    if (tamanho_bloco < overhead_fixo) {
        std::cerr << "Erro: Tamanho do bloco (" << tamanho_bloco 
                  << ") é menor que o overhead fixo (" << overhead_fixo 
                  << ")." << std::endl;
        return -1;
    }

    // Ordem <= (TAMANHO_BLOCO - Overhead_Fixo) / Tamanho_Por_Ordem
    double ordem_fracionaria = (static_cast<double>(tamanho_bloco) - overhead_fixo) / tamanho_por_ordem;
    int ordem = static_cast<int>(std::floor(ordem_fracionaria));
    
    std::cout << "Cálculo: ( " << tamanho_bloco << " - " << overhead_fixo << " ) / " << tamanho_por_ordem << " = " << ordem_fracionaria << std::endl;

    return ordem;
}


int main() {
    std::cout << "=== Calculadora da Ordem (M) da Árvore B+ ===" << std::endl;
    std::cout << std::endl;

    // --- Cálculo para ChaveSecundaria ---
    {
        std::cout << "--- Iniciando Cálculo para Chave: ChaveSecundaria ---" << std::endl;
        typedef ChaveSecundaria T_secundaria;
        
        int ordem = calcularOrdemDaArvore(
            TAMANHO_BLOCO,          // 4096
            sizeof(T_secundaria),   // sizeof(ChaveSecundaria) = 301
            sizeof(BlocoOffset),    // sizeof(long) = 8
            sizeof(bool),           // 1
            sizeof(int)             // 4
        );

        std::cout << "---------------------------------" << std::endl;
        if (ordem != -1) {
            std::cout << "Ordem (M) máxima para ChaveSecundaria: " << ordem << std::endl;

            // Verificação final
            double S_k = sizeof(T_secundaria);
            double S_p = sizeof(BlocoOffset);
            double overhead_fixo = sizeof(bool) + sizeof(int) + S_k + (3.0 * S_p);
            double tamanho_por_ordem = (2.0 * S_k) + (2.0 * S_p);
            double tamanho_total_calculado = overhead_fixo + (ordem * tamanho_por_ordem);
            
            std::cout << "Tamanho total usado no bloco com Ordem=" << ordem << ": " 
                      << tamanho_total_calculado << " / " << TAMANHO_BLOCO << " bytes" << std::endl;
        } else {
            std::cout << "Não foi possível calcular a ordem para ChaveSecundaria." << std::endl;
        }
        std::cout << "======================================================" << std::endl;
        std::cout << std::endl;
    }

    // --- Cálculo para Chave Inteira ---
    {
        std::cout << "--- Iniciando Cálculo para Chave: int ---" << std::endl;
        typedef int T_inteira;
        
        int ordem_int = calcularOrdemDaArvore(
            TAMANHO_BLOCO,          // 4096
            sizeof(T_inteira),      // sizeof(int) = 4
            sizeof(BlocoOffset),    // sizeof(long) = 8
            sizeof(bool),           // 1
            sizeof(int)             // 4 (comum)
        );

        std::cout << "---------------------------------" << std::endl;
        if (ordem_int != -1) {
            std::cout << "Ordem (M) máxima para Chave Inteira: " << ordem_int << std::endl;

            // Verificação final
            double S_k = sizeof(T_inteira);
            double S_p = sizeof(BlocoOffset);
            double overhead_fixo = sizeof(bool) + sizeof(int) + S_k + (3.0 * S_p);
            double tamanho_por_ordem = (2.0 * S_k) + (2.0 * S_p);
            double tamanho_total_calculado = overhead_fixo + (ordem_int * tamanho_por_ordem);
            
            std::cout << "Tamanho total usado no bloco com Ordem=" << ordem_int << ": " 
                      << tamanho_total_calculado << " / " << TAMANHO_BLOCO << " bytes" << std::endl;
        } else {
            std::cout << "Não foi possível calcular a ordem para Chave Inteira." << std::endl;
        }
        std::cout << "======================================================" << std::endl;
    }

    return 0;
}