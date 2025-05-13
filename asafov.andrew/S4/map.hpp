#ifndef MAP_HPP
#define MAP_HPP

#include <iostream>
#include <utility>
#include <stdexcept>
#include <vector>
#include <algorithm>

namespace asafov
{
  template< typename Key, typename Value >
  class map
  {
  private:
    struct Node
    {
      bool isLeaf;

      union
      {
        struct
        {
          Key keys[2];
          Value values[2];
          Node* children[3];
          int keyCount;
        } inner;

        struct
        {
          Key key;
          Value value;
        } leaf;
      };

      Node() : isLeaf(false)
      {
        inner.keyCount = 0;
        inner.children[0] = inner.children[1] = inner.children[2] = nullptr;
      }

      explicit Node(const Key& k, const Value& v) : isLeaf(true)
      {
        leaf.key = k;
        leaf.value = v;
      }

      ~Node()
      {
        if (!isLeaf)
        {
          for (int i = 0; i <= inner.keyCount; ++i)
          {
            delete inner.children[i];
          }
        }
      }
    };

    Node* root;
    size_t size_;

    // Helper functions
    bool isFull(const Node* node) const
    {
      if (!node) return false;
      return node->isLeaf ? true : (node->inner.keyCount == 2);
    }

    void insertIntoNode(Node* node, const Key& key, const Value& value, Node* leftChild = nullptr,
                        Node* rightChild = nullptr)
    {
      if (node->isLeaf)
      {
        // Shouldn't happen as we handle leaves separately
        return;
      }

      int pos = 0;
      while (pos < node->inner.keyCount && key > node->inner.keys[pos])
      {
        pos++;
      }

      // Shift keys and children to make space
      for (int i = node->inner.keyCount; i > pos; --i)
      {
        node->inner.keys[i] = node->inner.keys[i - 1];
        node->inner.values[i] = node->inner.values[i - 1];
      }
      for (int i = node->inner.keyCount + 1; i > pos + 1; --i)
      {
        node->inner.children[i] = node->inner.children[i - 1];
      }

      node->inner.keys[pos] = key;
      node->inner.values[pos] = value;
      if (leftChild)
      {
        node->inner.children[pos] = leftChild;
      }
      if (rightChild)
      {
        node->inner.children[pos + 1] = rightChild;
      }
      node->inner.keyCount++;
    }

    Node* splitNode(Node* node, Key& promotedKey, Value& promotedValue, Node*& promotedLeft, Node*& promotedRight)
    {
      if (node->isLeaf)
      {
        // Shouldn't happen as leaves are always single key
        return nullptr;
      }

      promotedKey = node->inner.keys[1];
      promotedValue = node->inner.values[1];

      // Create new node for the right part
      Node* newNode = new Node();
      newNode->inner.keyCount = 1;
      newNode->inner.keys[0] = node->inner.keys[2];
      newNode->inner.values[0] = node->inner.values[2];
      newNode->inner.children[0] = node->inner.children[2];
      newNode->inner.children[1] = node->inner.children[3];

      // Reset the original node
      node->inner.keyCount = 1;
      node->inner.children[2] = nullptr;

      promotedLeft = node;
      promotedRight = newNode;

      return newNode;
    }

    Node* insert(Node* node, const Key& key, const Value& value, bool& inserted, Key& promotedKey, Value& promotedValue,
                 Node*& promotedLeft, Node*& promotedRight)
    {
      if (node->isLeaf)
      {
        if (node->leaf.key == key)
        {
          // Key already exists, update value
          node->leaf.value = value;
          inserted = false;
          return nullptr;
        }

        // Need to split this leaf and promote
        if (key < node->leaf.key)
        {
          promotedKey = key;
          promotedValue = value;
          promotedLeft = new Node(key, value);
          promotedRight = node;
        }
        else
        {
          promotedKey = node->leaf.key;
          promotedValue = node->leaf.value;
          promotedLeft = node;
          promotedRight = new Node(key, value);
        }
        inserted = true;
        return nullptr;
      }

      // Find the appropriate child to insert into
      int childPos = 0;
      while (childPos < node->inner.keyCount && key > node->inner.keys[childPos])
      {
        childPos++;
      }

      if (childPos < node->inner.keyCount && key == node->inner.keys[childPos])
      {
        // Key already exists in this node, update value
        node->inner.values[childPos] = value;
        inserted = false;
        return nullptr;
      }

      Node* child = node->inner.children[childPos];
      Key newPromotedKey;
      Value newPromotedValue;
      Node* newLeft = nullptr;
      Node* newRight = nullptr;
      bool childInserted = true;

      Node* result = insert(child, key, value, childInserted, newPromotedKey, newPromotedValue, newLeft, newRight);

      if (!childInserted)
      {
        inserted = false;
        return nullptr;
      }

      if (newLeft)
      {
        // Child was split, need to promote
        if (!isFull(node))
        {
          insertIntoNode(node, newPromotedKey, newPromotedValue, newLeft, newRight);
          inserted = true;
          return nullptr;
        }
        else
        {
          // Current node is full, need to split
          Node* newNode = splitNode(node, promotedKey, promotedValue, promotedLeft, promotedRight);

          // Determine where to put the promoted key from child
          if (newPromotedKey < promotedKey)
          {
            insertIntoNode(promotedLeft, newPromotedKey, newPromotedValue, newLeft, newRight);
          }
          else
          {
            insertIntoNode(promotedRight, newPromotedKey, newPromotedValue, newLeft, newRight);
          }

          return newNode;
        }
      }

      inserted = true;
      return nullptr;
    }

    const Node* findNode(const Node* node, const Key& key) const
    {
      if (!node) return nullptr;

      if (node->isLeaf)
      {
        return (node->leaf.key == key) ? node : nullptr;
      }

      for (int i = 0; i < node->inner.keyCount; ++i)
      {
        if (key == node->inner.keys[i])
        {
          return node;
        }
        if (key < node->inner.keys[i])
        {
          return findNode(node->inner.children[i], key);
        }
      }

      return findNode(node->inner.children[node->inner.keyCount], key);
    }

    void inOrderTraversal(const Node* node, std::vector< std::pair< Key, Value > >& result) const
    {
      if (!node) return;

      if (node->isLeaf)
      {
        result.emplace_back(node->leaf.key, node->leaf.value);
        return;
      }

      for (int i = 0; i <= node->inner.keyCount; ++i)
      {
        inOrderTraversal(node->inner.children[i], result);
        if (i < node->inner.keyCount)
        {
          result.emplace_back(node->inner.keys[i], node->inner.values[i]);
        }
      }
    }

  public:
    map() : root(nullptr), size_(0)
    {
    }

    ~map()
    {
      delete root;
    }

    Value& operator[](const Key& key)
    {
      bool inserted;
      Key promotedKey;
      Value promotedValue;
      Node* promotedLeft = nullptr;
      Node* promotedRight = nullptr;

      if (!root)
      {
        root = new Node(key, Value());
        size_++;
        return root->leaf.value;
      }

      Node* newRoot = insert(root, key, Value(), inserted, promotedKey, promotedValue, promotedLeft, promotedRight);

      if (promotedLeft)
      {
        Node* oldRoot = root;
        root = new Node();
        root->inner.keyCount = 1;
        root->inner.keys[0] = promotedKey;
        root->inner.values[0] = promotedValue;
        root->inner.children[0] = promotedLeft;
        root->inner.children[1] = promotedRight;
      }

      if (inserted)
      {
        size_++;
      }

      const Node* found = findNode(root, key);
      if (!found)
      {
        throw std::runtime_error("Key not found after insertion");
      }
      return const_cast< Node* >(found)->isLeaf
               ? const_cast< Node* >(found)->leaf.value
               : const_cast< Node* >(found)->inner.values[0];
    }

    const Value& at(const Key& key) const
    {
      const Node* node = findNode(root, key);
      if (!node)
      {
        throw std::out_of_range("Key not found");
      }
      return node->isLeaf ? node->leaf.value : node->inner.values[0];
    }

    size_t size() const
    {
      return size_;
    }

    bool empty() const
    {
      return size_ == 0;
    }

    void clear()
    {
      delete root;
      root = nullptr;
      size_ = 0;
    }

    class iterator
    {
    private:
      struct StackItem
      {
        Node* node;
        int index;
      };

      Node* current;
      std::vector< StackItem > stack;
      bool atLeaf;

      void moveToLeftmost(Node* node)
      {
        while (node)
        {
          if (node->isLeaf)
          {
            current = node;
            atLeaf = true;
            return;
          }
          stack.push_back({node, 0});
          node = node->inner.children[0];
        }
        current = nullptr;
      }

    public:
      iterator(Node* root = nullptr) : current(nullptr), atLeaf(false)
      {
        if (root)
        {
          moveToLeftmost(root);
        }
      }

      std::pair< const Key, Value& > operator*()
      {
        if (!current)
        {
          throw std::runtime_error("Dereferencing end iterator");
        }
        if (atLeaf)
        {
          return {current->leaf.key, current->leaf.value};
        }
        else
        {
          auto& top = stack.back();
          return {top.node->inner.keys[top.index], top.node->inner.values[top.index]};
        }
      }

      iterator& operator++()
      {
        if (!current && stack.empty())
        {
          return *this;
        }

        if (atLeaf)
        {
          atLeaf = false;
          if (stack.empty())
          {
            current = nullptr;
          }
          else
          {
            auto& top = stack.back();
            if (top.index < top.node->inner.keyCount)
            {
              // Return the key in the inner node
              return *this;
            }
            else
            {
              // Need to move up
              current = nullptr;
              return *this;
            }
          }
        }
        else
        {
          if (!stack.empty())
          {
            auto& top = stack.back();
            top.index++;
            if (top.index < top.node->inner.keyCount)
            {
              // There's another key in this node
              return *this;
            }
            else
            {
              // Need to move to the next child
              if (top.index <= top.node->inner.keyCount)
              {
                Node* next = top.node->inner.children[top.index];
                stack.pop_back();
                moveToLeftmost(next);
              }
              else
              {
                stack.pop_back();
                current = nullptr;
              }
            }
          }
          else
          {
            current = nullptr;
          }
        }

        return *this;
      }

      bool operator!=(const iterator& other) const
      {
        if (current != other.current) return true;
        if (stack.size() != other.stack.size()) return true;
        for (size_t i = 0; i < stack.size(); ++i)
        {
          if (stack[i].node != other.stack[i].node ||
            stack[i].index != other.stack[i].index)
          {
            return true;
          }
        }
        return false;
      }
    };

    iterator begin()
    {
      return iterator(root);
    }

    iterator end()
    {
      return iterator(nullptr);
    }

    iterator find(const Key& key)
    {
      Node* node = root;
      std::vector< iterator::StackItem > path;

      while (node)
      {
        if (node->isLeaf)
        {
          if (node->leaf.key == key)
          {
            iterator it;
            it.current = node;
            it.atLeaf = true;
            return it;
          }
          return end();
        }

        bool found = false;
        for (int i = 0; i < node->inner.keyCount; ++i)
        {
          if (key == node->inner.keys[i])
          {
            iterator it;
            it.stack.push_back({node, i});
            return it;
          }
          if (key < node->inner.keys[i])
          {
            path.push_back({node, i});
            node = node->inner.children[i];
            found = true;
            break;
          }
          path.push_back({node, i});
        }

        if (!found)
        {
          path.push_back({node, node->inner.keyCount});
          node = node->inner.children[node->inner.keyCount];
        }
      }

      return end();
    }
  };
}

#endif
