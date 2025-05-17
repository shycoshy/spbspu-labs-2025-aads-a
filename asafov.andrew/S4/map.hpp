#ifndef MAP_HPP
#define MAP_HPP

#include <utility>
#include <cstddef>
#include <deque>
#include <stdexcept>

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
        : isTwoNode(true), key1(k), value1(v), left(nullptr), middle(nullptr), right(nullptr)
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
      Node* rightChild;

      InsertResult(Node* n = nullptr, Key k = Key(), Value v = Value(), bool s = false, Node* r = nullptr)
        : newChild(n), promotedKey(k), promotedValue(v), wasSplit(s), rightChild(r)
      {
      }
    };

    InsertResult insert(Node* node, const Key& key, const Value& value, int depth = 0)
    {
      if (depth > 1000) throw std::runtime_error("Stack overflow likely due to infinite recursion");
      if (!node)
      {
        return {new Node(key, value), Key(), Value(), true};
      }

      if (!node->left)
      {
        if (node->isTwoNode)
        {
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
          return {};
        }
        else
        {
          Key lowKey = node->key1;
          Value lowValue = node->value1;
          Key midKey;
          Value midValue;
          Key highKey = node->key2;
          Value highValue = node->value2;

          if (key < lowKey)
          {
            midKey = lowKey;
            midValue = lowValue;
            lowKey = key;
            lowValue = value;
          }
          else if (key < highKey)
          {
            midKey = key;
            midValue = value;
          }
          else
          {
            midKey = highKey;
            midValue = highValue;
            highKey = key;
            highValue = value;
          }

          Node* left = new Node(lowKey, lowValue);
          Node* right = new Node(highKey, highValue);
          ++size_;
          return {left, midKey, midValue, true, right};
        }
      }
      else
      {
        InsertResult ir;

        if (key < node->key1)
          ir = insert(node->left, key, value, depth + 1);
        else if (node->isTwoNode || key < node->key2)
          ir = insert(node->middle, key, value);
        else
          ir = insert(node->right, key, value);

        if (!ir.wasSplit)
          return {};

        if (node->isTwoNode)
        {
          if (key < node->key1)
          {
            node->key2 = node->key1;
            node->value2 = node->value1;
            node->key1 = ir.promotedKey;
            node->value1 = ir.promotedValue;
            node->right = node->middle;
            node->left = ir.newChild;
            node->middle = ir.rightChild;
          }
          else
          {
            node->key2 = ir.promotedKey;
            node->value2 = ir.promotedValue;
            node->middle = ir.newChild;
            node->right = ir.rightChild;
          }
          node->isTwoNode = false;
          return {};
        }
        else
        {
          Node* left = new Node(Key(), Value());
          Node* right = new Node(Key(), Value());
          Key midKey;
          Value midValue;

          if (key < node->key1)
          {
            left->key1 = ir.promotedKey;
            left->value1 = ir.promotedValue;
            left->left = ir.newChild;
            left->middle = ir.rightChild;

            right->key1 = node->key2;
            right->value1 = node->value2;
            right->left = node->middle;
            right->middle = node->right;

            midKey = node->key1;
            midValue = node->value1;
          }
          else if (key < node->key2)
          {
            left->key1 = node->key1;
            left->value1 = node->value1;
            left->left = node->left;
            left->middle = ir.newChild;

            right->key1 = node->key2;
            right->value1 = node->value2;
            right->left = ir.rightChild;
            right->middle = node->right;

            midKey = ir.promotedKey;
            midValue = ir.promotedValue;
          }
          else
          {
            left->key1 = node->key1;
            left->value1 = node->value1;
            left->left = node->left;
            left->middle = node->middle;

            right->key1 = ir.promotedKey;
            right->value1 = ir.promotedValue;
            right->left = ir.newChild;
            right->middle = ir.rightChild;

            midKey = node->key2;
            midValue = node->value2;
          }

          left->isTwoNode = true;
          right->isTwoNode = true;

          return {left, midKey, midValue, true, right};
        }
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

    void inorder(Node* node, std::deque< std::pair< const Key, Value > >& output) const
    {
      if (!node) return;
      if (node->left) inorder(node->left, output);
      output.emplace_back(node->key1, node->value1);
      if (node->middle) inorder(node->middle, output);
      if (!node->isTwoNode)
      {
        output.emplace_back(node->key2, node->value2);
        if (node->right) inorder(node->right, output);
      }
    }

  public:
    map() : root(nullptr), size_(0)
    {
    }

    ~map()
    {
      clear();
    }

    void clear()
    {
      destroy(root);
      root = nullptr;
      size_ = 0;
    }

    class iterator
    {
    public:
      using pair_t = std::pair< const Key, Value >;
      std::deque< pair_t > elements;
      typename std::deque< pair_t >::const_iterator current;

      iterator() = default;

      iterator(Node* root)
      {
        map temp;
        temp.inorder(root, elements);
        current = elements.begin();
      }

      iterator(std::deque< pair_t >&& deque_init)
      {
        elements = std::move(deque_init);
        current = elements.begin();
      }

      const pair_t& operator*() const
      {
        return *current;
      }

      const pair_t* operator->() const
      {
        return &(*current);
      }

      iterator& operator++()
      {
        ++current;
        return *this;
      }

      bool operator==(const iterator& other) const
      {
        return current == other.current;
      }

      bool operator!=(const iterator& other) const
      {
        return !(*this == other);
      }
    };

    using const_iterator = iterator;

    iterator begin() const
    {
      return iterator(root);
    }

    iterator end() const
    {
      return iterator();
    }

    const_iterator cbegin() const
    {
      return iterator(root);
    }

    const_iterator cend() const
    {
      return iterator();
    }

    bool empty() const
    {
      return size_ == 0;
    }

    std::size_t size() const
    {
      return size_;
    }

    iterator find(const Key& key) const
    {
      Node* found = findNode(root, key);
      if (!found) return end();
      std::deque< std::pair< const Key, Value > > output;
      if (found->key1 == key) output.emplace_back(found->key1, found->value1);
      else if (!found->isTwoNode && found->key2 == key) output.emplace_back(found->key2, found->value2);
      return iterator(std::move(output));
    }

    Value& operator[](const Key& key)
    {
      Node* node = findNode(root, key);
      if (node)
      {
        if (node->key1 == key) return node->value1;
        if (!node->isTwoNode && node->key2 == key) return node->value2;
      }

      InsertResult result = insert(root, key, Value{});
      if (result.wasSplit)
      {
        Node* newRoot = new Node(result.promotedKey, result.promotedValue);
        newRoot->isTwoNode = true;
        newRoot->left = result.newChild;
        newRoot->middle = result.rightChild;
        root = newRoot;
      }

      node = findNode(root, key);
      return (node->key1 == key) ? node->value1 : node->value2;
    }
  };
}

#endif
