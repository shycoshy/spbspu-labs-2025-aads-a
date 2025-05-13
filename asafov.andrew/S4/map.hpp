#ifndef MAP_HPP
#define MAP_HPP

#include <iostream>
#include <utility>
#include <stdexcept>

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
          std::pair< Key, Value > pairs[2];
          Node* children[3];
          int pairCount;
        } inner;

        std::pair< Key, Value > leafPair;
      };

      Node() : isLeaf(false)
      {
        inner.pairCount = 0;
        inner.children[0] = inner.children[1] = inner.children[2] = nullptr;
      }

      explicit Node(const Key& k, const Value& v) : isLeaf(true)
      {
        leafPair = std::make_pair(k, v);
      }

      ~Node()
      {
        if (!isLeaf)
        {
          for (int i = 0; i <= inner.pairCount; ++i)
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
      return node->isLeaf ? true : (node->inner.pairCount == 2);
    }

    void insertIntoNode(Node* node, const std::pair< Key, Value >& pair, Node* leftChild = nullptr,
                        Node* rightChild = nullptr)
    {
      if (node->isLeaf) return;

      int pos = 0;
      while (pos < node->inner.pairCount && pair.first > node->inner.pairs[pos].first)
      {
        pos++;
      }

      // Shift pairs and children to make space
      for (int i = node->inner.pairCount; i > pos; --i)
      {
        node->inner.pairs[i] = node->inner.pairs[i - 1];
      }
      for (int i = node->inner.pairCount + 1; i > pos + 1; --i)
      {
        node->inner.children[i] = node->inner.children[i - 1];
      }

      node->inner.pairs[pos] = pair;
      if (leftChild) node->inner.children[pos] = leftChild;
      if (rightChild) node->inner.children[pos + 1] = rightChild;
      node->inner.pairCount++;
    }

    Node* splitNode(Node* node, std::pair< Key, Value >& promotedPair, Node*& promotedLeft, Node*& promotedRight)
    {
      promotedPair = node->inner.pairs[1];

      // Create new node for the right part
      Node* newNode = new Node();
      newNode->inner.pairCount = 1;
      newNode->inner.pairs[0] = node->inner.pairs[2];
      newNode->inner.children[0] = node->inner.children[2];
      newNode->inner.children[1] = node->inner.children[3];

      // Reset the original node
      node->inner.pairCount = 1;
      node->inner.children[2] = nullptr;

      promotedLeft = node;
      promotedRight = newNode;

      return newNode;
    }

    Node* insert(Node* node, const std::pair< Key, Value >& pair, bool& inserted,
                 std::pair< Key, Value >& promotedPair, Node*& promotedLeft, Node*& promotedRight)
    {
      if (node->isLeaf)
      {
        if (node->leafPair.first == pair.first)
        {
          // Key exists, update value
          node->leafPair.second = pair.second;
          inserted = false;
          return nullptr;
        }

        // Need to split this leaf and promote
        if (pair.first < node->leafPair.first)
        {
          promotedPair = pair;
          promotedLeft = new Node(pair.first, pair.second);
          promotedRight = node;
        }
        else
        {
          promotedPair = node->leafPair;
          promotedLeft = node;
          promotedRight = new Node(pair.first, pair.second);
        }
        inserted = true;
        return nullptr;
      }

      // Find appropriate child
      int childPos = 0;
      while (childPos < node->inner.pairCount && pair.first > node->inner.pairs[childPos].first)
      {
        childPos++;
      }

      // Check if key exists in this node
      if (childPos < node->inner.pairCount && pair.first == node->inner.pairs[childPos].first)
      {
        node->inner.pairs[childPos].second = pair.second;
        inserted = false;
        return nullptr;
      }

      Node* child = node->inner.children[childPos];
      std::pair< Key, Value > newPromotedPair;
      Node* newLeft = nullptr;
      Node* newRight = nullptr;
      bool childInserted = true;

      Node* result = insert(child, pair, childInserted, newPromotedPair, newLeft, newRight);
      (void)result; // Avoid unused variable warning

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
          insertIntoNode(node, newPromotedPair, newLeft, newRight);
          inserted = true;
          return nullptr;
        }
        else
        {
          // Current node is full, need to split
          Node* newNode = splitNode(node, promotedPair, promotedLeft, promotedRight);

          // Determine where to put the promoted pair from child
          if (newPromotedPair.first < promotedPair.first)
          {
            insertIntoNode(promotedLeft, newPromotedPair, newLeft, newRight);
          }
          else
          {
            insertIntoNode(promotedRight, newPromotedPair, newLeft, newRight);
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
        return (node->leafPair.first == key) ? node : nullptr;
      }

      for (int i = 0; i < node->inner.pairCount; ++i)
      {
        if (key == node->inner.pairs[i].first)
        {
          return node;
        }
        if (key < node->inner.pairs[i].first)
        {
          return findNode(node->inner.children[i], key);
        }
      }

      return findNode(node->inner.children[node->inner.pairCount], key);
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
      std::pair< Key, Value > promotedPair;
      Node* promotedLeft = nullptr;
      Node* promotedRight = nullptr;

      if (!root)
      {
        root = new Node(key, Value());
        size_++;
        return root->leafPair.second;
      }

      Node* newRoot = insert(root, std::make_pair(key, Value()), inserted, promotedPair, promotedLeft, promotedRight);
      (void)newRoot; // Avoid unused variable warning

      if (promotedLeft)
      {
        Node* oldRoot = root;
        (void)oldRoot; // Avoid unused variable warning
        root = new Node();
        root->inner.pairCount = 1;
        root->inner.pairs[0] = promotedPair;
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
               ? const_cast< Node* >(found)->leafPair.second
               : const_cast< Node* >(found)->inner.pairs[0].second;
    }

    const Value& at(const Key& key) const
    {
      const Node* node = findNode(root, key);
      if (!node)
      {
        throw std::out_of_range("Key not found");
      }
      return node->isLeaf ? node->leafPair.second : node->inner.pairs[0].second;
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
      Node* current;
      Node* parent;
      int index;
      bool atLeaf;

    public:
      iterator(Node* node = nullptr, Node* p = nullptr, int i = 0, bool leaf = false)
        : current(node), parent(p), index(i), atLeaf(leaf)
      {
      }

      std::pair< const Key, Value& > operator*()
      {
        if (!current)
        {
          throw std::runtime_error("Dereferencing end iterator");
        }
        if (atLeaf)
        {
          return {current->leafPair.first, current->leafPair.second};
        }
        else
        {
          if (parent)
          {
            return {parent->inner.pairs[index].first, parent->inner.pairs[index].second};
          }
          throw std::runtime_error("Invalid iterator state");
        }
      }

      iterator& operator++()
      {
        if (!current) return *this;

        if (atLeaf)
        {
          atLeaf = false;
          if (!parent)
          {
            current = nullptr;
          }
          return *this;
        }

        if (parent)
        {
          index++;
          if (index < parent->inner.pairCount)
          {
            return *this;
          }

          // Move to next child
          if (index <= parent->inner.pairCount)
          {
            current = parent->inner.children[index];
            while (current && !current->isLeaf)
            {
              parent = current;
              current = current->inner.children[0];
              index = 0;
            }
            atLeaf = (current != nullptr);
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
        return current != other.current ||
          parent != other.parent ||
          index != other.index ||
          atLeaf != other.atLeaf;
      }
    };

    iterator begin()
    {
      Node* node = root;
      Node* parent = nullptr;
      int index = 0;

      while (node && !node->isLeaf)
      {
        parent = node;
        node = node->inner.children[0];
      }

      return iterator(node, parent, 0, node != nullptr);
    }

    iterator end()
    {
      return iterator(nullptr);
    }

    iterator find(const Key& key)
    {
      Node* node = root;
      Node* parent = nullptr;
      int index = 0;

      while (node)
      {
        if (node->isLeaf)
        {
          if (node->leafPair.first == key)
          {
            return iterator(node, parent, index, true);
          }
          return end();
        }

        bool found = false;
        for (int i = 0; i < node->inner.pairCount; ++i)
        {
          if (key == node->inner.pairs[i].first)
          {
            return iterator(nullptr, node, i, false);
          }
          if (key < node->inner.pairs[i].first)
          {
            parent = node;
            node = node->inner.children[i];
            index = i;
            found = true;
            break;
          }
        }

        if (!found)
        {
          parent = node;
          index = node->inner.pairCount;
          node = node->inner.children[node->inner.pairCount];
        }
      }

      return end();
    }
  };
} // namespace asafov

#endif // MAP_HPP
