
#include "mprpcLRU.h"

 // 获取指定的元素
int LRUCache::getNode(int key)
{
    auto node = getLastNode(key);
    // 检测节点存不存在
    if(node == nullptr)
    {
        return -1;
    }
    // 存在返回节点的值
    return node->value;
}

// 往缓冲中加入元素
// 1. 先判断是否已经存在，如果存在，修改value即可，并且移动到最上层
// 2. 不存在，先添加元素，看看是否越界，如果越界，表明区间大小容量不够，需要删除最下层元素
void LRUCache::putNode(int key,int value)
{
    auto node = getLastNode(key);
    // 检测是否存在，存在，修改值
    if(node)
    {
        node->value = value;
        return;
    }
    // 代表不存在，需要插入元素
    // 创建节点
    node = new Node(key,value);
    // 加入哈希表
    key_to_node[key] = node;
    // 加入链表中---》添加链表
    push_frontNode(node);
    // 检测元素个数是否超过给定的容量
    if(key_to_node.size() > capacity)
    {
        // 删除链表的尾部元素----》也就是dummy->pre指针所指向的元素
        auto back_node = dummy->pre;
        // 在哈希表移除这个节点
        key_to_node.erase(back_node->key);
        // 移除节点
        removeNode(back_node);
        // 删除节点
        delete back_node
    }
}




void LRUCache::removeNode(Node *node)
{
    node->pre->next = node->next;
    node->next->pre = node->pre;
}

// 添加双端链表节点
// 节点加在dummy后面
void LRUCache::push_frontNode(Node *node)
{
    // 待插入元素的前指针指向虚拟头结点
    node->pre = dummy;
    // 待插入元素的后指针指向虚拟头结点指向的节点
    node->next = dummy->next;
    // 虚拟头结点的后指针指向待插入元素
    node->pre->next = node;
    // 虚拟头结点原先的前指针指向待插入元素
    node->next->pre = node;
}

// 获取最上层的节点
// 获取当前指定值的书在不在,也就是指定的节点存不存在
// 不存在返回nullptr，存在返回对用的node，并且做好了移到最上层的工作
Node* LRUCache::getLastNode(int key)
{
    auto it = key_to_node.find(key);
    // 表示当前节点不存在，直接返回空----》这个值被getNode捕获既可以返回-1
    if(it == key_to_node.end())
    {
        return nullptr;
    }
    // 表示当前的节点存在于哈希表中
    Node *node = it->second;
    // 移除当前节点
    removeNode(node);
    // 添加到最上层
    push_frontNode(node);
    return node;
}