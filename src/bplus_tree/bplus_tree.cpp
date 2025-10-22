#include "bplus_tree.h"
#include <iostream>
#include <cmath>

// ============ Node Implementation ============

template <typename KeyType>
BPlusTree<KeyType>::Node::Node(int order, bool leaf) : is_leaf(leaf), num_keys(0), next_leaf(-1)
{
    keys = new KeyType[order];
    values = new long[order];
    children = new long[order + 1];

    // Inicializa arrays
    for (int i = 0; i < order; i++)
    {
        values[i] = -1;
        children[i] = -1;
    }
    children[order] = -1;
}

template <typename KeyType>
BPlusTree<KeyType>::Node::~Node()
{
    delete[] keys;
    delete[] values;
    delete[] children;
}

// ============ BPlusTree Implementation ============

template <typename KeyType>
BPlusTree<KeyType>::BPlusTree(const std::string& fname, int block_size)
    : filename(fname), root_offset(-1)
{
    // Calcula ordem baseada no tamanho do bloco
    // Aproximação: block_size = num_keys * (sizeof(key) + sizeof(long))
    int key_size = sizeof(KeyType);
    int pointer_size = sizeof(long);

    max_keys = (block_size - sizeof(bool) - sizeof(int) - sizeof(long)) /
        (key_size + pointer_size + pointer_size);

    if (max_keys < 3) max_keys = 3; // Mínimo para B+ tree funcionar

    order = max_keys + 1;
}

template <typename KeyType>
BPlusTree<KeyType>::~BPlusTree()
{
    close();
}

template <typename KeyType>
bool BPlusTree<KeyType>::open()
{
    file.open(filename, std::ios::in | std::ios::out | std::ios::binary);

    if (!file.is_open())
    {
        // Cria arquivo novo
        file.open(filename, std::ios::out | std::ios::binary);
        root_offset = -1;
        file.write(reinterpret_cast<char*>(&root_offset), sizeof(long));
        file.close();
        file.open(filename, std::ios::in | std::ios::out | std::ios::binary);
    }
    else
    {
        // Lê root offset
        file.seekg(0);
        file.read(reinterpret_cast<char*>(&root_offset), sizeof(long));
    }

    return file.is_open();
}

template <typename KeyType>
void BPlusTree<KeyType>::close()
{
    if (file.is_open())
    {
        // Salva root offset
        file.seekp(0);
        file.write(reinterpret_cast<char*>(&root_offset), sizeof(long));
        file.close();
    }
}

template <typename KeyType>
long BPlusTree<KeyType>::allocateNode(bool is_leaf)
{
    file.seekp(0, std::ios::end);
    return file.tellp();
}

template <typename KeyType>
void BPlusTree<KeyType>::writeNode(long offset, const Node& node)
{
    file.seekp(offset);
    file.write(reinterpret_cast<const char*>(&node.is_leaf), sizeof(bool));
    file.write(reinterpret_cast<const char*>(&node.num_keys), sizeof(int));
    file.write(reinterpret_cast<const char*>(&node.next_leaf), sizeof(long));

    for (int i = 0; i < order; i++)
    {
        file.write(reinterpret_cast<const char*>(&node.keys[i]), sizeof(KeyType));
    }
    for (int i = 0; i < order; i++)
    {
        file.write(reinterpret_cast<const char*>(&node.values[i]), sizeof(long));
    }
    for (int i = 0; i <= order; i++)
    {
        file.write(reinterpret_cast<const char*>(&node.children[i]), sizeof(long));
    }
    file.flush();
}

template <typename KeyType>
void BPlusTree<KeyType>::readNode(long offset, Node& node)
{
    file.seekg(offset);
    file.read(reinterpret_cast<char*>(&node.is_leaf), sizeof(bool));
    file.read(reinterpret_cast<char*>(&node.num_keys), sizeof(int));
    file.read(reinterpret_cast<char*>(&node.next_leaf), sizeof(long));

    for (int i = 0; i < order; i++)
    {
        file.read(reinterpret_cast<char*>(&node.keys[i]), sizeof(KeyType));
    }
    for (int i = 0; i < order; i++)
    {
        file.read(reinterpret_cast<char*>(&node.values[i]), sizeof(long));
    }
    for (int i = 0; i <= order; i++)
    {
        file.read(reinterpret_cast<char*>(&node.children[i]), sizeof(long));
    }
}

template <typename KeyType>
void BPlusTree<KeyType>::copyKey(KeyType& dest, const KeyType& src)
{
    dest = src; // Atribuição simples funciona
}

// Especialização para strings
template <>
void BPlusTree<char[300]>::copyKey(char (&dest)[300], const char (&src)[300])
{
    strncpy(dest, src, 300);
    dest[299] = '\0'; // Garante null termination
}

template <typename KeyType>
void BPlusTree<KeyType>::insert(const KeyType& key, long value)
{
    if (root_offset == -1)
    {
        // Cria raiz
        Node root(order, true);

        copyKey(root.keys[0], key);
        root.values[0] = value;
        root.num_keys = 1;

        root_offset = allocateNode(true);
        writeNode(root_offset, root);
        return;
    }

    Node root(order, false);
    readNode(root_offset, root);

    if (root.num_keys == max_keys)
    {
        // Raiz está cheia, precisa dividir
        Node new_root(order, false);
        new_root.children[0] = root_offset;

        long new_root_offset = allocateNode(false);
        splitChild(new_root, 0, root);

        root_offset = new_root_offset;
        writeNode(root_offset, new_root);

        std::cout << "Vindo aqui 1" << std::endl;

        insertNonFull(root_offset, key, value);
    }
    else
    {
        insertNonFull(root_offset, key, value);
    }
}

template <typename KeyType>
void BPlusTree<KeyType>::insertNonFull(long node_offset, const KeyType& key, long value)
{
    Node node(order, false);
    readNode(node_offset, node);
    std::cout << "Vindo aqui 2" << std::endl;

    if (node.is_leaf)
    {
        // Insere na folha
        int i = node.num_keys - 1;

        while (i >= 0 && compareKeys(key, node.keys[i]) < 0)
        {
            copyKey(node.keys[i + 1], node.keys[i]);
            node.values[i + 1] = node.values[i];
            i--;
        }

        copyKey(node.keys[i + 1], key);
        node.values[i + 1] = value;
        node.num_keys++;

        writeNode(node_offset, node);
    }
    else
    {
        std::cout << "Vindo aqui 3" << std::endl;

        // Encontra filho correto
        int i = node.num_keys - 1;
        while (i >= 0 && compareKeys(key, node.keys[i]) < 0)
        {
            i--;
        }
        i++;



        Node child(order, false);
        readNode(node.children[i], child);



        if (child.num_keys == max_keys)
        {
            splitChild(node, i, child);
            writeNode(node_offset, node);

            if (compareKeys(key, node.keys[i]) > 0)
            {
                i++;
            }
        }

        std::cout << "Vindo aqui 5" << std::endl;

        insertNonFull(node.children[i], key, value);
    }
}

template <typename KeyType>
long BPlusTree<KeyType>::searchInNode(long node_offset, const KeyType& key)
{
    Node node(order, false);
    readNode(node_offset, node);

    int i = 0;
    while (i < node.num_keys && compareKeys(key, node.keys[i]) > 0)
    {
        i++;
    }

    if (i < node.num_keys && compareKeys(key, node.keys[i]) == 0)
    {
        if (node.is_leaf)
        {
            return node.values[i];
        }
    }

    if (node.is_leaf)
    {
        return -1;
    }

    return searchInNode(node.children[i], key);
}

template <typename KeyType>
void BPlusTree<KeyType>::splitChild(Node& parent, int index, Node& child)
{
    int mid = max_keys / 2;
    Node new_child(order, child.is_leaf);
    new_child.num_keys = max_keys - mid;

    // Copia metade das chaves para novo nó
    for (int i = 0; i < new_child.num_keys; i++)
    {
        copyKey(new_child.keys[i], child.keys[mid + i]);
        new_child.values[i] = child.values[mid + i];
    }

    if (!child.is_leaf)
    {
        for (int i = 0; i <= new_child.num_keys; i++)
        {
            new_child.children[i] = child.children[mid + i];
        }
    }
    else
    {
        new_child.next_leaf = child.next_leaf;
    }

    child.num_keys = mid;

    if (child.is_leaf)
    {
        child.next_leaf = allocateNode(true);
        writeNode(child.next_leaf, new_child);
    }
    else
    {
        long new_child_offset = allocateNode(false);
        writeNode(new_child_offset, new_child);
    }

    // Insere chave do meio no pai
    for (int i = parent.num_keys; i > index; i--)
    {
        copyKey(parent.keys[i], parent.keys[i - 1]);
        parent.children[i + 1] = parent.children[i];
    }

    copyKey(parent.keys[index], child.keys[mid]);
    parent.children[index + 1] = (child.is_leaf) ? child.next_leaf : allocateNode(false);
    parent.num_keys++;

    writeNode(parent.children[index], child);
}

template <typename KeyType>
long BPlusTree<KeyType>::search(const KeyType& key)
{
    if (root_offset == -1)
    {
        return -1;
    }

    return searchInNode(root_offset, key);
}

// Instanciações explícitas dos templates
template class BPlusTree<long>;
template class BPlusTree<char[300]>;
