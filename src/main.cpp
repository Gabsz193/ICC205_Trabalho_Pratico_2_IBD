//
// Created by luizg on 17/10/2025.
//

#include <iostream>
#include "parser/parser_data.h"
#include "index_manager/index_manager.h"

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        std::cerr << "Uso: " << argv[0] << " <arquivo_entrada> <tamanho_bloco_bytes>" << std::endl;
        std::cerr << "Exemplo: " << argv[0] << " dados.csv 4096" << std::endl;
        return -1;
    }

    const char* arquivo_entrada = argv[1];
    int block_size = std::atoi(argv[2]);

    if (block_size <= 0)
    {
        std::cerr << "Erro: tamanho de bloco inválido" << std::endl;
        return -1;
    }

    std::cout << "=== Sistema de Indexação de Artigos ===" << std::endl;
    std::cout << "Arquivo de entrada: " << arquivo_entrada << std::endl;
    std::cout << "Tamanho do bloco: " << block_size << " bytes" << std::endl;
    std::cout << std::endl;

    // ==========================================
    // INICIALIZA SISTEMA DE INDEXAÇÃO
    // ==========================================

    IndexManager manager("artigos.dat",
                         "indice_primario.idx",
                         "indice_secundario.idx",
                         block_size);

    if (!manager.initialize())
    {
        std::cerr << "Erro ao inicializar sistema de indexação" << std::endl;
        return -1;
    }

    std::cout << "✓ Sistema de indexação inicializado" << std::endl;
    std::cout << "  - artigos.dat (arquivo de dados - hash)" << std::endl;
    std::cout << "  - indice_primario.idx (B+ Tree por ID)" << std::endl;
    std::cout << "  - indice_secundario.idx (B+ Tree por Título)" << std::endl;
    std::cout << std::endl;

    // ==========================================
    // LEITURA E INSERÇÃO DOS ARTIGOS
    // ==========================================

    DataParser parser(arquivo_entrada);
    parser.open();

    std::cout << "Processando artigos..." << std::endl;
    std::cout << std::endl;

    auto art = Artigo();
    art = parser.readLine();

    int count = 0;
    int erros = 0;

    while (art.id != 0)
    {
        // Insere o artigo no sistema de indexação
        if (manager.insertArticle(art))
        {
            count++;

            // Mostra progresso a cada 100 artigos
            if (count % 100 == 0)
            {
                std::cout << "Processados " << count << " artigos..." << std::endl;
            }

            // Mostra detalhes a cada 1000 artigos (opcional)
            if (count % 1000 == 0)
            {
                std::cout << "  └─ Último: ID=" << art.id
                    << " | Título: " << art.titulo << std::endl;
            }
        }
        else
        {
            erros++;
            std::cerr << "✗ Erro ao inserir artigo ID: " << art.id << std::endl;
        }

        art = parser.readLine();
    }

    parser.close();

    std::cout << std::endl;
    std::cout << "=== Resumo da Inserção ===" << std::endl;
    std::cout << "Total de artigos inseridos: " << count << std::endl;
    std::cout << "Erros: " << erros << std::endl;
    std::cout << std::endl;

    // ==========================================
    // TESTES DE BUSCA (OPCIONAL)
    // ==========================================

    if (count > 0)
    {
        std::cout << "=== Testando Sistema de Busca ===" << std::endl;
        std::cout << std::endl;

        char opcao;
        std::cout << "Deseja realizar testes de busca? (s/n): ";
        std::cin >> opcao;
        std::cin.ignore();

        if (opcao == 's' || opcao == 'S')
        {
            Artigo resultado;

            // Menu de testes
            int escolha;
            do
            {
                std::cout << "\n--- Menu de Busca ---" << std::endl;
                std::cout << "1. Buscar por ID" << std::endl;
                std::cout << "2. Buscar por Título" << std::endl;
                std::cout << "3. Inserir novo artigo" << std::endl;
                std::cout << "0. Sair" << std::endl;
                std::cout << "Escolha: ";
                std::cin >> escolha;
                std::cin.ignore();

                if (escolha == 1)
                {
                    long id;
                    std::cout << "\nDigite o ID: ";
                    std::cin >> id;
                    std::cin.ignore();

                    std::cout << "Buscando..." << std::endl;

                    if (manager.searchById(id, resultado))
                    {
                        std::cout << "\n✓ Artigo encontrado!" << std::endl;
                        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
                        std::cout << "ID: " << resultado.id << std::endl;
                        std::cout << "Título: " << resultado.titulo << std::endl;
                        std::cout << "Autores: " << resultado.autores << std::endl;
                        std::cout << "Ano: " << resultado.ano << std::endl;
                        std::cout << "Citações: " << resultado.citacoes << std::endl;
                        std::cout << "Atualização: " << resultado.atualizacao << std::endl;
                        std::cout << "Snippet: " << resultado.snippet << std::endl;
                        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
                    }
                    else
                    {
                        std::cout << "\n✗ Artigo não encontrado (ID: " << id << ")" << std::endl;
                    }
                }
                else if (escolha == 2)
                {
                    char titulo[300];
                    std::cout << "\nDigite o título (exato): ";
                    std::cin.getline(titulo, 300);

                    std::cout << "Buscando..." << std::endl;

                    if (manager.searchByTitle(titulo, resultado))
                    {
                        std::cout << "\n✓ Artigo encontrado!" << std::endl;
                        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
                        std::cout << "ID: " << resultado.id << std::endl;
                        std::cout << "Título: " << resultado.titulo << std::endl;
                        std::cout << "Autores: " << resultado.autores << std::endl;
                        std::cout << "Ano: " << resultado.ano << std::endl;
                        std::cout << "Citações: " << resultado.citacoes << std::endl;
                        std::cout << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━" << std::endl;
                    }
                    else
                    {
                        std::cout << "\n✗ Artigo não encontrado" << std::endl;
                        std::cout << "Dica: O título deve ser exatamente igual ao cadastrado" << std::endl;
                    }
                }
                else if (escolha == 3)
                {
                    Artigo novo_art;

                    std::cout << "\n--- Inserir Novo Artigo ---" << std::endl;
                    std::cout << "ID: ";
                    std::cin >> novo_art.id;
                    std::cin.ignore();

                    std::cout << "Título: ";
                    std::cin.getline(novo_art.titulo, 300);

                    std::cout << "Autores: ";
                    std::cin.getline(novo_art.autores, 150);

                    std::cout << "Ano: ";
                    std::cin >> novo_art.ano;

                    std::cout << "Citações: ";
                    std::cin >> novo_art.citacoes;
                    std::cin.ignore();

                    std::cout << "Snippet: ";
                    std::cin.getline(novo_art.snippet, 1024);

                    novo_art.atualizacao = time(nullptr);

                    if (manager.insertArticle(novo_art))
                    {
                        std::cout << "\n✓ Artigo inserido com sucesso!" << std::endl;
                        count++;
                    }
                    else
                    {
                        std::cout << "\n✗ Erro ao inserir artigo" << std::endl;
                    }
                }
            }
            while (escolha != 0);
        }
    }

    // ==========================================
    // FINALIZAÇÃO
    // ==========================================

    std::cout << "\n=== Encerrando Sistema ===" << std::endl;
    std::cout << "Total final de artigos: " << count << std::endl;
    std::cout << "Fechando arquivos..." << std::endl;

    manager.close();

    std::cout << "✓ Sistema encerrado com sucesso!" << std::endl;
    std::cout << "\nArquivos gerados:" << std::endl;
    std::cout << "  - artigos.dat" << std::endl;
    std::cout << "  - indice_primario.idx" << std::endl;
    std::cout << "  - indice_secundario.idx" << std::endl;

    return 0;
}
