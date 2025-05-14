#ifndef MAP_HPP
#define MAP_HPP

#include <utility>
#include <cstddef>
#include <vector>
#include <stdexcept>
#include <functional>
#include <initializer_list>
#include <queue>

namespace asafov
{
  template< class Key, class Value, class Comparator = std::less< Key > >
  class map
  {
  private:
    struct Node
    {
      Node* left;
      Node* middle;
      Node* right;
      Node* parent;
      std::pair< Key, Value > pair1;
      std::pair< Key, Value > pair2;
      bool isThreeNode;

      Node(const Key& k, const Value& v, Node* p = nullptr) :
        left(nullptr), middle(nullptr), right(nullptr),
        parent(p), pair1(k, v), isThreeNode(false)
      {
      }

      bool isLeaf() const
      {
        return !left && !middle && !right;
      }
    };

    Node* root;
    size_t size_;
    Comparator comp;

    void clear(Node* node)
    {
      if (!node) return;
      clear(node->left);
      clear(node->middle);
      clear(node->right);
      delete node;
    }

    Node* findNode(const Key& key) const
    {
      Node* current = root;
      while (current)
      {
        if (!comp(key, current->pair1.first) && !comp(current->pair1.first, key))
          return current;

        if (current->isThreeNode &&
          !comp(key, current->pair2.first) &&
          !comp(current->pair2.first, key))
          return current;

        if (comp(key, current->pair1.first))
          current = current->left;
        else if (!current->isThreeNode || comp(key, current->pair2.first))
          current = current->middle;
        else
          current = current->right;
      }
      return nullptr;
    }

    void split(Node* node, const Key& key, const Value& value)
    {
      if (!node->parent)
      {
        // Split root
        Node* newRoot = new Node(node->pair2.first, node->pair2.second);
        Node* leftChild = new Node(node->pair1.first, node->pair1.first, newRoot);
        Node* rightChild = new Node(key, value, newRoot);

        newRoot->left = leftChild;
        newRoot->right = rightChild;

        leftChild->left = node->left;
        leftChild->right = node->middle;
        rightChild->left = node->right;

        if (leftChild->left) leftChild->left->parent = leftChild;
        if (leftChild->right) leftChild->right->parent = leftChild;
        if (rightChild->left) rightChild->left->parent = rightChild;

        delete node;
        root = newRoot;
      }
      else
      {
        // Split non-root node
        Node* parent = node->parent;

        if (!parent->isThreeNode)
        {
          // Parent is 2-node
          if (parent->left == node)
          {
            parent->pair2 = parent->pair1;
            parent->pair1 = node->pair1;
            parent->isThreeNode = true;

            Node* newLeft = new Node(node->pair1.first, node->pair1.first, parent);
            Node* newMiddle = new Node(key, value, parent);

            newLeft->left = node->left;
            newLeft->right = node->middle;
            newMiddle->left = node->right;

            parent->left = newLeft;
            parent->middle = newMiddle;
          }
          else
          {
            parent->pair2 = node->pair1;
            parent->isThreeNode = true;

            Node* newMiddle = new Node(node->pair1.first, node->pair1.first, parent);
            Node* newRight = new Node(key, value, parent);

            newMiddle->left = node->left;
            newMiddle->right = node->middle;
            newRight->left = node->right;

            parent->middle = newMiddle;
            parent->right = newRight;
          }
        }
        else
        {
          // Parent is 3-node - need to propagate split
          Node* temp = new Node(key, value);
          temp->isThreeNode = true;
          temp->pair2 = node->pair1;

          if (comp(node->pair1.first, temp->pair1.first))
            std::swap(temp->pair1, temp->pair2);

          split(parent, temp->pair1.first, temp->pair1.second);
          delete temp;
          return;
        }

        delete node;
      }
    }

  public:
    class Iterator
    {
    private:
      std::queue< Node* > nodes;
      Node* current;
      bool useSecond;

      void traverse(Node* node)
      {
        if (!node) return;
        traverse(node->left);
        nodes.push(node);
        if (node->isThreeNode)
        {
          traverse(node->middle);
          nodes.push(node); // For pair2
        }
        traverse(node->right);
      }

    public:
      Iterator(Node* root, bool end = false) : current(nullptr), useSecond(false)
      {
        if (root && !end)
        {
          traverse(root);
          if (!nodes.empty())
          {
            current = nodes.front();
            nodes.pop();
          }
        }
      }

      std::pair< const Key, Value >& operator*()
      {
        return useSecond
                 ? reinterpret_cast< std::pair< const Key, Value >& >(current->pair2)
                 : reinterpret_cast< std::pair< const Key, Value >& >(current->pair1);
      }

      Iterator& operator++()
      {
        if (!nodes.empty())
        {
          if (current->isThreeNode && !useSecond)
          {
            useSecond = true;
          }
          else
          {
            current = nodes.front();
            nodes.pop();
            useSecond = false;
          }
        }
        else
        {
          if (current->isThreeNode && !useSecond)
          {
            useSecond = true;
          }
          else
          {
            current = nullptr;
          }
        }
        return *this;
      }

      bool operator!=(const Iterator& other) const
      {
        return current != other.current || useSecond != other.useSecond;
      }
    };

    map() : root(nullptr), size_(0)
    {
    }

    map(std::initializer_list< std::pair< Key, Value > > init) : map()
    {
      for (const auto& p: init)
        insert(p.first, p.second);
    }

    ~map()
    {
      clear(root);
    }

    Iterator begin()
    {
      return Iterator(root);
    }

    Iterator end()
    {
      return Iterator(root, true);
    }

    void insert(const Key& key, const Value& value)
    {
      if (!root)
      {
        root = new Node(key, value);
        size_++;
        return;
      }

      Node* current = root;
      while (!current->isLeaf())
      {
        if (comp(key, current->pair1.first))
          current = current->left;
        else if (!current->isThreeNode || comp(key, current->pair2.first))
          current = current->middle;
        else
          current = current->right;
      }

      if (!current->isThreeNode)
      {
        // Simple insert into 2-node
        if (comp(key, current->pair1.first))
        {
          current->pair2 = current->pair1;
          current->pair1 = {key, value};
        }
        else
        {
          current->pair2 = {key, value};
        }
        current->isThreeNode = true;
        size_++;
      }
      else
      {
        // Need to split node
        split(current, key, value);
        size_++;
      }
    }

    Iterator find(const Key& key)
    {
      Node* node = findNode(key);
      if (!node) return end();

      Iterator it(root);
      while (it != end())
      {
        if ((!comp(key, it->first) && !comp(it->first, key)))
          return it;
        ++it;
      }
      return end();
    }

    Value& operator[](const Key& key)
    {
      auto it = find(key);
      if (it != end())
        return it->second;

      insert(key, Value());
      return find(key)->second;
    }

    Value& at(const Key& key)
    {
      auto it = find(key);
      if (it == end())
        throw std::out_of_range("Key not found");
      return it->second;
    }

    const Value& at(const Key& key) const
    {
      Node* node = findNode(key);
      if (!node)
        throw std::out_of_range("Key not found");

      if (!comp(key, node->pair1.first) && !comp(node->pair1.first, key))
        return node->pair1.second;
      return node->pair2.second;
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
      clear(root);
      root = nullptr;
      size_ = 0;
    }
  };
}

#endif
