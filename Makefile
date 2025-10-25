CC := g++
CFLAGS := -Wall -Wextra -std=c++17 -Wno-narrowing
# Adiciona o flag -Isrc para que o compilador encontre cabeçalhos em subdiretórios de src/
CFLAGS += -Isrc

PROGRAM_NAMES := upload findrec seek1 seek2

BIN_DIR := bin
OBJ_DIR := obj

# Encontra TODOS os arquivos .cpp em src/ e seus subdiretórios de forma recursiva
ALL_SOURCES := $(shell find src -name "*.cpp")

# Adiciona o prefixo src/ aos arquivos main (ex: src/upload.cpp)
MAIN_SOURCES_BASENAMES := $(addsuffix .cpp, $(PROGRAM_NAMES))
MAIN_SOURCES := $(patsubst %.cpp,src/%.cpp,$(MAIN_SOURCES_BASENAMES))

# Filtra os módulos (todos os .cpp que NÃO são os arquivos main)
MODULE_SOURCES := $(filter-out $(MAIN_SOURCES), $(ALL_SOURCES))

# Transforma o caminho src/pasta/modulo.cpp em obj/pasta/modulo.o
# Esta é a parte crucial para lidar com subdiretórios
OBJECTS := $(patsubst src/%.cpp,$(OBJ_DIR)/%.o,$(patsubst src/%,%,$(MODULE_SOURCES)))

# Define os alvos finais (ex: bin/upload)
TARGETS := $(addprefix $(BIN_DIR)/, $(PROGRAM_NAMES))

.PHONY: all clean build-upload build-findrec build-seek1 build-seek2

all: $(TARGETS)

build-upload: $(BIN_DIR)/upload
	@echo "Executável 'upload' construído em $(BIN_DIR)/upload"

build-findrec: $(BIN_DIR)/findrec
	@echo "Executável 'findrec' construído em $(BIN_DIR)/findrec"

build-seek1: $(BIN_DIR)/seek1
	@echo "Executável 'seek1' construído em $(BIN_DIR)/seek1"

build-seek2: $(BIN_DIR)/seek2
	@echo "Executável 'seek2' construído em $(BIN_DIR)/seek2"

# --- Regras de Compilação e Linkagem ---

# Regra de linkagem: bin/programa (Target) depende de src/programa.cpp (Source) e de todos os objetos
$(BIN_DIR)/%: src/%.cpp
	@mkdir -p $(BIN_DIR)
	# Linka o arquivo main ($<) com TODOS os objetos compilados ($(OBJECTS))
	$(CC) $(CFLAGS) $< $(MODULE_SOURCES) -o $@

# Regra de compilação de objetos: obj/caminho/modulo.o (Target) depende de src/caminho/modulo.cpp (Source)
# Note que $< é o caminho completo (ex: src/parser/parser_data.cpp)
$(OBJ_DIR)/%.o: $(OBJECTS)
	@mkdir -p $(dir $@)
	# Compila o módulo e cria o arquivo objeto na pasta correta
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "Removendo arquivos objeto e executáveis..."
	rm -rf $(OBJ_DIR) $(BIN_DIR)
