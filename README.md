# Trabalho Prático 2 de IBD

Este documento descreve os pré-requisitos e os passos necessários para executar o projeto.

## Pré-requisitos

Você pode rodar este projeto de duas maneiras: usando Docker (recomendado) ou localmente.

### Para rodar com Docker
- [Docker](https://docs.docker.com/get-docker/)
- [Docker Compose](https://docs.docker.com/compose/install/)

### Para rodar localmente
- Um compilador C++ com suporte a C++17 (como o G++)
- `make`

## Como Rodar

Antes de começar, crie um diretório chamado `data` na raiz do projeto e coloque o arquivo `artigo.csv` dentro dele.

### Rodando com Docker (Recomendado)

Esta é a maneira mais simples, pois o ambiente já vem configurado.

**1. Construa a imagem Docker:**
A partir da raiz do projeto, execute o comando:
```shell
docker-compose build
```

**2. Carregue os dados:**
Este comando irá ler o `artigo.csv` e criar os arquivos de dados e de índice.
```shell
docker-compose run upload
```

**3. Execute as buscas:**
- Para buscar um registro por ID (busca sequencial):
  ```shell
  docker-compose run findrec <ID>
  ```
- Para buscar um registro por ID (usando índice primário):
  ```shell
  docker-compose run seek1 <ID>
  ```
- Para buscar registros por título (usando índice secundário):
  ```shell
  docker-compose run seek2 "<TÍTULO>"
  ```
  *Observação: coloque o título entre aspas duplas se ele contiver espaços.*

### Rodando Localmente

**1. Compile o projeto:**
Use o `make` para compilar todos os executáveis. Eles serão criados no diretório `bin/`.
```shell
make
```

**2. Carregue os dados:**
Execute o programa de upload para processar o `artigo.csv` e gerar os arquivos de dados.
```shell
./bin/upload -i data/artigo.csv -d data/data.dat -p data/prim.idx -s data/sec.idx
```

**3. Execute as buscas:**
- Para buscar um registro por ID (busca sequencial):
  ```shell
  ./bin/findrec -d data/data.dat -f <ID>
  ```
- Para buscar um registro por ID (usando índice primário):
  ```shell
  ./bin/seek1 -d data/data.dat -p data/prim.idx -f <ID>
  ```
- Para buscar registros por título (usando índice secundário):
  ```shell
  ./bin/seek2 -d data/data.dat -s data/sec.idx -t "<TÍTULO>"
  ```
  *Observação: coloque o título entre aspas duplas se ele contiver espaços.*