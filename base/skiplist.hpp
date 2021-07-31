#pragma once
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <iostream>
#include <functional>

template<typename K, typename V>
class Node
{

public:
    int node_level;
    K key;
    V value;
    Node<K, V>** _forward;
    Node() {}
    Node(int level)
    {
        this->node_level = level;
        this->_forward = new Node<K, V>*[level + 1];
        memset(this->_forward, 0, sizeof(Node<K, V>*) * (level + 1));
    }
    Node(const K& k, const V& v, int level)
        :key(k), value(v)
    {
        this->node_level = level;
        this->_forward = new Node<K, V>*[level + 1];
        memset(this->_forward, 0, sizeof(Node<K, V>*) * (level + 1));
    };

    ~Node() { delete[] _forward; }
    Node<K, V>* next(){return _forward[0];}
private:
};

template <typename K, typename V>
class SkipList 
{
    typedef std::function<bool(K, K)> callback;
public:
    SkipList();
    ~SkipList();
    int size();
    int push_back(const K&, const V&);
    int get_random_level();
    Node<K,V>* find(const K&);
    Node<K, V>* create_node(const K&, const V&, int);
    Node<K, V>* begin(){return _header->_forward[0];}
    Node<K, V>* end(){return nullptr;}
    

private:
    int _max_level;
    int _skip_list_level;
    int _element_count;
    Node<K, V>* _header;
};

template<typename K, typename V>
Node<K, V>* SkipList<K, V>::create_node(const K& k, const V& v, int level) 
{
    Node<K, V>* n = new Node<K, V>(k, v, level);
    return n;
}

template<typename K, typename V>
int SkipList<K, V>::push_back(const K& k, const V& value)
{

    Node<K, V>* current = this->_header;
    Node<K, V>* update[_max_level + 1];
    memset(update, 0, sizeof(Node<K, V>*) * (_max_level + 1));

    for (int i = _skip_list_level; i >= 0; i--)
    {
        while (current->_forward[i] != nullptr && k > current->_forward[i]->key) 
        {
            current = current->_forward[i];
        }
        update[i] = current;
    }

    current = current->_forward[0];
        
    {
        int random_level = get_random_level();

        if (random_level > _skip_list_level) 
        {
            for (int i = _skip_list_level + 1; i < random_level + 1; i++)
            {
                update[i] = _header;
            }
            _skip_list_level = random_level;
        }
        Node<K, V>* inserted_node = create_node(k, value, random_level);

        for (int i = 0; i <= random_level; i++) 
        {
            inserted_node->_forward[i] = update[i]->_forward[i];
            update[i]->_forward[i] = inserted_node;
        }
        _element_count++;
    }
    return 1;
}

template<typename K, typename V>
int SkipList<K, V>::size() 
{
    return _element_count;
}


//返回>=key的第一个
template<typename K, typename V>
Node<K,V>* SkipList<K, V>::find(const K& key) 
{
    Node<K, V>* current = _header;

    for (int i = _skip_list_level; i >= 0; i--) {
        while (current->_forward[i] &&  key >current->_forward[i]->key) {
            current = current->_forward[i];
        }
    }

    current = current->_forward[0];

    
    return current;
}
template<typename K, typename V>
SkipList<K, V>::SkipList() 
{

    this->_max_level = 16;
    this->_skip_list_level = 0;
    this->_element_count = 0;
    this->_header = new Node<K, V>(_max_level);
};

template<typename K, typename V>
SkipList<K, V>::~SkipList() 
{
 
    delete _header;
}


template<typename K, typename V>
int SkipList<K, V>::get_random_level() 
{

    int k = 1;
    while (rand() % 2)
    {
        k++;
    }
    k = (k < _max_level) ? k : _max_level;
    return k;
}