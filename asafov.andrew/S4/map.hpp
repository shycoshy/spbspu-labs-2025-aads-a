#ifndef MAP_HPP
#define MAP_HPP

#include <utility>
#include <cstddef>
#include <deque>

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
    };

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
        if (!node->left)
        {
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
        else
        {
          InsertResult ir;
          if (key < node->key1) ir = insert(node->left, key, value);
          else if (key < node->key2) ir = insert(node->middle, key, value);
          else ir = insert(node->right, key, value);

          if (ir.wasSplit)
          {
            Node* leftNode = new Node(node->key1, node->value1);
            Node* rightNode = new Node(node->key2, node->value2);
            leftNode->isTwoNode = true;
            rightNode->isTwoNode = true;

            if (key < node->key1)
            {
              leftNode->left = ir.newChild;
              leftNode->middle = node->left;
              rightNode->left = node->middle;
              rightNode->middle = node->right;
              return {new Node(node->key1, node->value1), node->key1, node->value1, true};
            }
            else if (key < node->key2)
            {
              leftNode->left = node->left;
              leftNode->middle = ir.newChild;
              rightNode->left = node->middle;
              rightNode->middle = node->right;
              return {new Node(key, value), key, value, true};
            }
            else
            {
              leftNode->left = node->left;
              leftNode->middle = node->middle;
              rightNode->left = ir.newChild;
              rightNode->middle = node->right;
              return {new Node(node->key2, node->value2), node->key2, node->value2, true};
            }
          }
          return {nullptr, Key(), Value(), false};
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

      iterator()
      {
      }

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
      if (node && node->key1 == key) return node->value1;
      if (node && !node->isTwoNode && node->key2 == key) return node->value2;

      auto result = insert(root, key, Value{});
      if (result.wasSplit)
      {
        Node* newRoot = new Node(result.promotedKey, result.promotedValue);
        newRoot->isTwoNode = false;
        newRoot->left = result.newChild;
        newRoot->middle = root;
        root = newRoot;
      }

      node = findNode(root, key);

      return (key == node->key1) ? node->value1 : node->value2;
    }
  };
} // namespace asafov

#endif // MAP_HPP
