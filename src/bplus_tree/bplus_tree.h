#ifndef BPLUS_TREE_HPP
#define BPLUS_TREE_HPP

#include <fstream>
#include <string>
#include <cstring>
#include <vector>

// Template para chave genérica (long ou string)
template <typename KeyType>
class BPlusTree
{
private:
    struct Node
    {
        bool is_leaf;
        int num_keys;
        KeyType* keys;
        long* values; // Para folhas: offsets dos dados
        long* children; // Para nós internos: offsets dos filhos
        long next_leaf; // Para folhas: próxima folha (lista encadeada)

        Node(int order, bool leaf);
        ~Node();
    };

    std::string filename;
    std::fstream file;
    int order; // Ordem da árvore (max keys por nó)
    int max_keys; // order - 1
    long root_offset;

    // Funções auxiliares
    int compareKeys(const KeyType& k1, const KeyType& k2) const;
    long allocateNode(bool is_leaf);
    void writeNode(long offset, const Node& node);
    void readNode(long offset, Node& node);
    long searchInNode(long node_offset, const KeyType& key);
    void splitChild(Node& parent, int index, Node& child);
    void insertNonFull(long node_offset, const KeyType& key, long value);
    void copyKey(KeyType& dest, const KeyType& src);

public:
    BPlusTree(const std::string& fname, int block_size);
    ~BPlusTree();

    bool open();
    void close();
    void insert(const KeyType& key, long value);
    long search(const KeyType& key);
};

// Especialização para comparação de strings
template <>
inline int BPlusTree<char[300]>::compareKeys(const char (&k1)[300], const char (&k2)[300]) const
{
    return strcmp(k1, k2);
}

// Comparação para long
template <>
inline int BPlusTree<long>::compareKeys(const long& k1, const long& k2) const
{
    if (k1 < k2) return -1;
    if (k1 > k2) return 1;
    return 0;
}

#endif // BPLUS_TREE_HPP
