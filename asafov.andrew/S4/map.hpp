#ifndef MAP_HPP
#define MAP_HPP

#include <utility>
#include <cstddef>
#include <cassert>

namespace asafov
{
  template< typename Key, typename Value >
  class map
  {
  private:
    struct Node
    {
      bool isTwoNode;
      Key key1, key2;
      Value value1, value2;
      Node* left;
      Node* middle;
      Node* right;

      Node(const Key& k, const Value& v)
        : isTwoNode(true), key1(k), value1(v),
          left(nullptr), middle(nullptr), right(nullptr)
      {
      }
    };

    Node* root;
    std::size_t size_;

    struct InsertResult
    {
      Node* newChild;
      Key promotedKey;
      Value promotedValue;
      bool wasSplit;
    };

    // Вставка с балансировкой
    InsertResult insert(Node* node, const Key& key, const Value& value)
    {
      if (!node)
      {
        return {new Node(key, value), Key(), Value(), true};
      }

      if (node->isTwoNode)
      {
        if (!node->left)
        {
          // Лист, преобразуем в 3-узел
          if (key < node->key1)
          {
            node->key2 = node->key1;
            node->value2 = node->value1;
            node->key1 = key;
            node->value1 = value;
          }
          else
          {
            node->key2 = key;
            node->value2 = value;
          }
          node->isTwoNode = false;
          ++size_;
          return {nullptr, Key(), Value(), false};
        }
        else
        {
          // Рекурсивная вставка
          InsertResult ir;
          if (key < node->key1) ir = insert(node->left, key, value);
          else ir = insert(node->middle, key, value);

          if (ir.wasSplit)
          {
            Node* newNode = new Node(ir.promotedKey, ir.promotedValue);
            newNode->isTwoNode = false;
            if (key < node->key1)
            {
              newNode->left = ir.newChild;
              newNode->middle = node;
            }
            else
            {
              newNode->left = node;
              newNode->middle = ir.newChild;
            }
            return {newNode, node->key1, node->value1, true};
          }
          return {nullptr, Key(), Value(), false};
        }
      }
      else
      {
        // Расщепляем 3-узел
        Node* newNode = new Node(Key(), Value());
        Key midKey;
        Value midValue;
        if (key < node->key1)
        {
          midKey = node->key1;
          midValue = node->value1;
          newNode->key1 = node->key2;
          newNode->value1 = node->value2;
          node->key1 = key;
          node->value1 = value;
        }
        else if (key < node->key2)
        {
          midKey = key;
          midValue = value;
          newNode->key1 = node->key2;
          newNode->value1 = node->value2;
        }
        else
        {
          midKey = node->key2;
          midValue = node->value2;
          newNode->key1 = key;
          newNode->value1 = value;
        }
        node->isTwoNode = true;
        ++size_;
        return {newNode, midKey, midValue, true};
      }
    }

    Node* findNode(Node* node, const Key& key) const
    {
      if (!node) return nullptr;
      if (key == node->key1) return node;
      if (!node->isTwoNode && key == node->key2) return node;
      if (key < node->key1) return findNode(node->left, key);
      if (node->isTwoNode || key < node->key2) return findNode(node->middle, key);
      return findNode(node->right, key);
    }

    void destroy(Node* node)
    {
      if (!node) return;
      destroy(node->left);
      destroy(node->middle);
      destroy(node->right);
      delete node;
    }

  public:
    map() : root(nullptr), size_(0)
    {
    }

    ~map()
    {
      destroy(root);
    }

    class iterator
    {
      // минимальный итератор для begin() и end()
      using pair_t = std::pair< const Key, Value >;

      struct StackNode
      {
        Node* node;
        bool firstDone;
        bool secondDone;
      };

      StackNode stack[64];
      int top;
      pair_t current;

      void pushLeft(Node* node)
      {
        while (node)
        {
          stack[++top] = {node, false, false};
          node = node->left;
        }
      }

    public:
      iterator() : top(-1)
      {
      }

      iterator(Node* root) : top(-1)
      {
        pushLeft(root);
        ++(*this);
      }

      iterator& operator++()
      {
        while (top >= 0)
        {
          auto& sn = stack[top];
          if (!sn.firstDone)
          {
            current = {sn.node->key1, sn.node->value1};
            sn.firstDone = true;
            pushLeft(sn.node->middle);
            return *this;
          }
          else if (!sn.node->isTwoNode && !sn.secondDone)
          {
            current = {sn.node->key2, sn.node->value2};
            sn.secondDone = true;
            pushLeft(sn.node->right);
            return *this;
          }
          else
          {
            --top;
          }
        }
        current = pair_t();
        return *this;
      }

      const pair_t& operator*() const
      {
        return current;
      }

      const pair_t* operator->() const
      {
        return &current;
      }

      bool operator!=(const iterator& other) const
      {
        return top != other.top;
      }
    };

    iterator begin() const
    {
      return iterator(root);
    }

    iterator end() const
    {
      return iterator(); // пустой
    }

    bool empty() const
    {
      return size_ == 0;
    }

    iterator find(const Key& key) const
    {
      Node* found = findNode(root, key);
      if (!found) return end();
      map temp;
      temp.root = found;
      return temp.begin();
    }

    Value& operator[](const Key& key)
    {
      Node* node = findNode(root, key);
      if (!node)
      {
        auto result = insert(root, key, Value{});
        if (result.wasSplit)
        {
          root = new Node(result.promotedKey, result.promotedValue);
          root->isTwoNode = false;
          root->left = result.newChild;
          root->middle = root; // old root
        }
        node = findNode(root, key);
      }
      return key == node->key1 ? node->value1 : node->value2;
    }
  };
} // namespace asafov

#endif // MAP_HPP
