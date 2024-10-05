#pragma once

// LRU算法的使用
// 本地缓存的淘汰算法

#include <iostream>
#include <unordered_map>


// 双端链表的构建
// 暂时只是写出来-----------》后续改为模板的形式
class Node
{
public:
    Node *pre;
    Node *next;
    int key;
    int value; // 

    Node(int key = 0, int value = 0) : key(key), value(value) {}
};



class LRUCache {
public:
    LRUCache(int capacity) 
    : capacity(capacity) ,
    dummy(new Node())  // 可以不初始化，因为默认会初始化
    {
        dummy->next = dummy;
        dummy->pre = dummy
    }

    // 获取指定的元素
    int getNode(int key);

    // 往缓冲中加入元素
    void putNode(int key,int value);

public:
    int capacity;
    Node *dummy; // 虚拟头结点
    unordered_map<int,Node*>key_to_node;;
    

    // 移除双端链表节点
    void removeNode(Node *node);

    // 添加双端链表节点---》使节点移动到最上面
    void push_frontNode(Node *node);

    // 获取最上层的节点
    // 获取当前指定值的书在不在,也就是指定的节点存不存在
    // 不存在返回nullptr，存在返回对用的node，并且做好了移到最上层的工作
    Node* getLastNode();
}