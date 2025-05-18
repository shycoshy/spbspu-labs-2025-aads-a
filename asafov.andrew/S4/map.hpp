#ifndef MAP_HPP
#define MAP_HPP

#include <utility>

namespace asafov
{
  template< typename Key, typename Value >
  class map
  {
    enum Color: char
    {
      RED,
      BLACK
    };

    struct node
    {
      std::pair< Key, Value > data;
      node* left = nullptr;
      node* right = nullptr;
      node* parent = nullptr;
      Color color = RED;

      explicit node(const std::pair< Key, Value >& val):
        data(val)
      {
      }
    };

    void leftRotate(node* x)
    {
      node* y = x->right;
      x->right = y->left;
      if (y->left) y->left->parent = x;
      y->parent = x->parent;
      if (!x->parent) root_ = y;
      else if (x == x->parent->left) x->parent->left = y;
      else x->parent->right = y;
      y->left = x;
      x->parent = y;
    }

    void rightRotate(node* y)
    {
      node* x = y->left;
      y->left = x->right;
      if (x->right) x->right->parent = y;
      x->parent = y->parent;
      if (!y->parent) root_ = x;
      else if (y == y->parent->right) y->parent->right = x;
      else y->parent->left = x;
      x->right = y;
      y->parent = x;
    }

    void insertFix(node* z)
    {
      while (z->parent && z->parent->color == RED)
      {
        if (z->parent == z->parent->parent->left)
        {
          node* y = z->parent->parent->right;
          if (y && y->color == RED)
          {
            z->parent->color = BLACK;
            y->color = BLACK;
            z->parent->parent->color = RED;
            z = z->parent->parent;
          }
          else
          {
            if (z == z->parent->right)
            {
              z = z->parent;
              leftRotate(z);
            }
            z->parent->color = BLACK;
            z->parent->parent->color = RED;
            rightRotate(z->parent->parent);
          }
        }
        else
        {
          node* y = z->parent->parent->left;
          if (y && y->color == RED)
          {
            z->parent->color = BLACK;
            y->color = BLACK;
            z->parent->parent->color = RED;
            z = z->parent->parent;
          }
          else
          {
            if (z == z->parent->left)
            {
              z = z->parent;
              rightRotate(z);
            }
            z->parent->color = BLACK;
            z->parent->parent->color = RED;
            leftRotate(z->parent->parent);
          }
        }
      }
      root_->color = BLACK;
    }

    node* findNode(const Key& key) const
    {
      node* current = root_;
      while (current)
      {
        if (key == current->data.first) return current;
        if (key < current->data.first) current = current->left;
        else current = current->right;
      }
      return nullptr;
    }

    void deleteTree(node* node)
    {
      if (!node) return;
      deleteTree(node->left);
      deleteTree(node->right);
      delete node;
    }

  public:
    map() = default;

    ~map()
    {
      deleteTree(root_);
    }

    Value& operator[](const Key& key)
    {
      node* current = root_;
      node* parent = nullptr;
      while (current)
      {
        if (key == current->data.first)
          return current->data.second;
        parent = current;
        if (key < current->data.first) current = current->left;
        else current = current->right;
      }

      node* newNode = new node({key, Value{}});
      newNode->parent = parent;

      if (!parent)
        root_ = newNode;
      else if (key < parent->data.first)
        parent->left = newNode;
      else
        parent->right = newNode;

      insertFix(newNode);
      return newNode->data.second;
    }

    bool empty() const
    {
      return root_ == nullptr;
    }

    class iterator
    {
      node* current;

      void goLeftmost()
      {
        while (current && current->left) current = current->left;
      }

    public:
      iterator(node* node) : current(node)
      {
        goLeftmost();
      }

      std::pair< Key, Value >& operator*() const
      {
        return current->data;
      }

      std::pair< Key, Value >* operator->() const
      {
        return &(current->data);
      }

      iterator& operator++()
      {
        if (!current) return *this;

        if (current->right)
        {
          current = current->right;
          while (current->left) current = current->left;
        }
        else
        {
          node* p = current->parent;
          while (p && current == p->right)
          {
            current = p;
            p = p->parent;
          }
          current = p;
        }

        return *this;
      }

      bool operator!=(const iterator& other) const
      {
        return current != other.current;
      }
    };

    iterator begin() const
    {
      return iterator(root_);
    }

    iterator end() const
    {
      return iterator(nullptr);
    }

    iterator find(const Key& key) const
    {
      return iterator(findNode(key));
    }

  private:
    node* root_ = nullptr;
  };
}

#endif
