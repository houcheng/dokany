#pragma once

#include "CppSupport.h"
#include "uthash.h"
#include "wdm.h"

#pragma warning(disable: 4127)

//
// Can only run in PASSIVE level
// T and V can be either primitive type or structure.
//
template <typename T, typename V>
struct HashNode {
    T key;
    V data;
    UT_hash_handle hh;

    struct HashNode<T, V> * next() { return (HashNode<T,V> *)hh.next; }
};

//
// Can only run in PASSIVE level
// For string, uses KeyHash
//
template <typename T, typename V> class NONPAGESECTION Hash {
    HashNode<T, V> * table = nullptr;

public:
    ~Hash();
    NTSTATUS get(T index, OUT V *outData);
    V operator[](T index);

    bool contains(T index);

    NTSTATUS add(T index, V data);
    NTSTATUS del(T index);

    // For iterating table
    HashNode<T, V> *first() { return table; }
};

template<typename T, typename V>
inline Hash<T, V>::~Hash() {
    HashNode<T, V> *index, *tmp;
    // safe iterates
    HASH_ITER(hh, table, index, tmp) {
        HASH_DEL(table, index);
        free(index);
    }
}

template<typename T, typename V>
inline NTSTATUS Hash<T, V>::get(T key, OUT V *outData) {
    HashNode<T, V> *index;

    HASH_FIND(hh, table, &key, sizeof(T), index);

    if (index) {
        *outData = index->data;
    } else {
        *outData = { };
    }

    return index ? STATUS_SUCCESS : STATUS_NOT_FOUND;
}

template<typename T, typename V>
inline V Hash<T, V>::operator[](T key) {
    V result = { };
    get(key, &result);
    return result;
}

template<typename T, typename V>
inline bool Hash<T, V>::contains(T key) {
    HashNode<T, V> *index;
    HASH_FIND(hh, table, &key, sizeof(T), index);
    return index != nullptr;
}

template<typename T, typename V>
inline NTSTATUS Hash<T, V>::add(T key, V data) {
    if (contains(key)) return STATUS_ALIAS_EXISTS;

    auto newNode = (HashNode<T, V> *)malloc(sizeof(HashNode<T,V>));
    newNode->key = key;
    newNode->data = data;
    newNode->hh = { };
    HASH_ADD(hh, table, key, sizeof(key), newNode);
    return STATUS_SUCCESS;
}

template<typename T, typename V>
inline NTSTATUS Hash<T, V>::del(T key) {
    HashNode<T, V> *index;
    HASH_FIND(hh, table, &key, sizeof(T), index);
    if (index == nullptr) return STATUS_NOT_FOUND;

    HASH_DEL(table, index);
    free(index);

    return STATUS_SUCCESS;
}

template <typename T> class HashSet {
    HashNode<T, int> * table = nullptr;
public:
    HashSet() { }
    ~HashSet();
    bool contains(T index);
    void add(T index);
    NTSTATUS del(T index);
    // For iterating table
    HashNode<T, int> *first() { return table; }
};

template<typename T>
inline HashSet<T>::~HashSet() {
    HashNode<T, int> *index, *tmp;
    // safe iterates
    HASH_ITER(hh, table, index, tmp) {
        HASH_DEL(table, index);
        free(index);
    }
}

template<typename T>
inline bool HashSet<T>::contains(T key) {
    HashNode<T, int> *index = nullptr;

    HASH_FIND(hh, table, &key, sizeof(key), index);
    return index != nullptr;
}

template<typename T>
inline void HashSet<T>::add(T key) {
    if (contains(key)) return;

    auto newNode = (HashNode<T, int> *)malloc(sizeof(HashNode<T, int>));
    newNode->key = key;
    newNode->data = 0;
    newNode->hh = { };

    HASH_ADD(hh, table, key, sizeof(key), newNode);
    // Or HASH_ADD_KEYPTR(hh, table, &(newNode->key), sizeof(key), newNode);
    // But not HASH_ADD_KEYPTR(hh, table, &key, sizeof(key) newNode);
}

template<typename T>
inline NTSTATUS HashSet<T>::del(T key) {
    HashNode<T, int> *index;
    HASH_FIND(hh, table, &key, sizeof(key), index);
    if (index == nullptr) return STATUS_NOT_FOUND;

    HASH_DEL(table, index);
    free(index);

    return STATUS_SUCCESS;
}

//
// Advanced Hash and HashSet
//
class NONPAGESECTION Key {
public:
    virtual void *pointer() const = 0;
    virtual unsigned size() const = 0;
    virtual Key *cloneByPointer() const = 0;
    virtual ~Key() = default;
};

template <typename V> class NONPAGESECTION KeyNode {
public:
    Key * key = nullptr;
    V data = { };
    UT_hash_handle hh = { };

    KeyNode<V> * next() { return (KeyNode<V> *)hh.next; }
    KeyNode(Key *key0) {
        key = key0->cloneByPointer();
    }
    virtual ~KeyNode() {
        if (key) {
            delete key;
        }
    }
};

template <typename V> class NONPAGESECTION KeyHash {
    KeyNode<V> * table = nullptr;

public:
    virtual ~KeyHash();
    NTSTATUS get(Key *key, OUT V *outData);
    bool contains(Key *key);

    NTSTATUS add(Key *key, V data);
    NTSTATUS del(Key *key);

    // For iterating table
    KeyNode<V> *first() { return table; }
};

template<typename V>
inline KeyHash<V>::~KeyHash() {
    KeyNode<V> *index, *tmp;
    // safe iterates
    HASH_ITER(hh, table, index, tmp) {
        HASH_DEL(table, index);
        delete index;
    }
}

template<typename V>
inline NTSTATUS KeyHash<V>::get(Key *key, OUT V *outData) {
    KeyNode<V> *index;

    HASH_FIND(hh, table, key->pointer(), key->size(), index);

    if (index) {
        *outData = index->data;
    } else {
        *outData = { };
    }

    return index ? STATUS_SUCCESS : STATUS_NOT_FOUND;
}

template<typename V>
inline bool KeyHash<V>::contains(Key *key) {
    KeyNode<V> *index;
    HASH_FIND(hh, table, key->pointer(), key->size(), index);
    return index != nullptr;
}

template<typename V>
inline NTSTATUS KeyHash<V>::add(Key *key, V data) {
    if (contains(key)) return STATUS_ALIAS_EXISTS;

    auto newKeyNode = new KeyNode<V>(key);
    newKeyNode->data = data;
    newKeyNode->hh = { };
    HASH_ADD_KEYPTR(hh, table, newKeyNode->key->pointer(), newKeyNode->key->size(), newKeyNode);
    return STATUS_SUCCESS;
}

template<typename V>
inline NTSTATUS KeyHash<V>::del(Key *key) {
    KeyNode<V> *index;
    HASH_FIND(hh, table, key->pointer(), key->size(), index);
    if (index == nullptr) return STATUS_NOT_FOUND;

    HASH_DEL(table, index);
    delete index;

    return STATUS_SUCCESS;
}
