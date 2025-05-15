#ifndef MAP_HPP
#define MAP_HPP

#include <utility>
#include <stdexcept>
#include <vector>
#include <deque>

namespace asafov
{
  template< class Key, class Value >
  class map
  {
    struct node
    {
      node(const Key& k, Value& v, node* p = nullptr):
        left(nullptr),
        middle(nullptr),
        right(nullptr),
        parent(p),
        isThreeNode(false)
      {
        pair1 = std::make_pair(k, v);
      }

      ~node()
      {
        ~pair1;
        ~pair2;
      }

      node* left;
      node* middle;
      node* right;
      node* parent;
      std::pair< const Key, Value > pair1;
      std::pair< const Key, Value > pair2;
      bool isThreeNode;
      bool pair2Processed = false;
    };


    node* find_approximately(const Key& k)
    {
      node* where = root_;
      while (where->left)
      {
        if (where->type)
        {
          if (Comparator(where->pair1.first, k))
          {
            where = where->left;
          }
          else if (Comparator(k, where->pair2.first))
          {
            where = where->right;
          }
          else
          {
            where = where->middle;
          }
        }
        else
        {
          if (Comparator(where->pair1.first, k))
          {
            where = where->left;
          }
          else
          {
            where = where->rigth;
          }
        }
      }
      return where;
    }

    bool a_less_then_b(Key a, Key b)
    {
      return a < b;
    }

  public:
    map():
      root_(nullptr),
      size_(0)
    {
    }

    void insert(const Key& k, Value& v)
    {
      if (!root_)
      {
        root_ = new node{nullptr, nullptr, nullptr, nullptr, std::make_pair(k, v), std::make_pair(k, v), false};
        deletes_.push_back(root_);
        return;
      }
      std::pair< const Key, Value > temp = std::make_pair(k, v);
      node* where = root_;
      //ищем куда вставить
      while (where->left)
      {
        if (a_less_then_b(temp.first, where->pair1.first))
        {
          where = where->left;
        }
        else if (a_less_then_b(temp.first, where->pair2.first) && where->isThreeNode)
        {
          where = where->middle;
        }
        else
        {
          where = where->right;
        }
      }
      //проверям ноду
      while (true)
      {
        if (!where->isThreeNode)
        {
          if (a_less_then_b(temp.first, where->pair1.first))
          {
            std::swap(where->pair1, where->pair2);
          }
          std::swap(temp, where->pair2);
          where->isThreeNode = true;
          ~temp;
          break;
        }
        if (where->isThreeNode && where->parent)
        {
          if (a_less_then_b(temp.first, where->pair1.first))
          {
            std::swap(where->pair1, temp);
          }
          if (a_less_then_b(where->pair2.first), temp.first)
          {
            std::swap(where->pair2, temp);
          }
          where = where->parent;
        }
        else //root-переполнение
        {
          //left, middle, right, parent, pair1, pair2, isThreeNode
          node* tl = new node{root_->left, nullptr, root_->middle->left, root_, root_->pair1, root_->pair1, false};
          deletes_.push_back(tl);
          node* tr = new node{root_->right, nullptr, root_->middle->right, root_, root_->pair2, root_->pair2, false};
          deletes_.push_back(tr);
          std::swap(temp, root_->pair1);
          root_->isThreeNode = false;
          root_->middle = nullptr;
          root_->left = tl;
          root_->right = tr;
          tl = nullptr;
          tr = nullptr;
          ~temp;
          break;
        }
      }
    }

    void clear()
    {
      while (!deletes_.empty())
      {
        delete deletes_.front();
        deletes_.pop_front();
      }
    }


    Value& operator[](const Key& k)
    {
      node* where = find_approximately(k);
      if (Comparator(where->pair1.first, k) && Comparator(k, where->pair1.first))
      {
        return where->pair1.second;
      }
      else if (where->type)
      {
        if (Comparator(where->pair2.first, k) && Comparator(k, where->pair2.first))
        {
          return where->pair2.second;
        }
      }
      return Value();
    }

    Value& at(const Key& k)
    {
      Value& temp = operator[](k);
      if (temp != Value())
      {
        throw std::out_of_range("key not found!");
      }
      return temp;
    }

    const Value& at(const Key& k) const
    {
      Value& temp = operator[](k);
      if (temp != Value())
      {
        throw std::out_of_range("key not found!");
      }
      return temp;
    }

    void swap(map& other) noexcept
    {
      std::swap(root_, other->root_);
      std::swap(size_, other->size_);
    }

    size_t size() const noexcept
    {
      return size_;
    }

    bool empty() const noexcept
    {
      return size_ == 0;
    }

    class iterator
    {
    public:
      iterator(node* root)
      {
        if (root)
        {
          traverseLeft(root);
        }
      }

      std::pair< const Key, Value >& operator*()
      {
        return stack_.back()->pair1;
      }

      std::pair< const Key, Value >* operator->()
      {
        return &stack_.back()->pair1;
      }

      iterator& operator++()
      {
        node* current = stack_.back();
        stack_.pop_back();

        // Если есть правое поддерево, идем в него
        if (current->right)
        {
          traverseLeft(current->right);
        }
        // Если это 3-узел и мы обработали только pair1
        else if (current->isThreeNode && !current->pair2Processed)
        {
          current->pair2Processed = true;
          stack_.push_back(current);
        }

        return *this;
      }

      bool operator!=(const iterator& other) const
      {
        return !stack_.empty() || !other.stack_.empty();
      }

    private:
      void traverseLeft(node* n)
      {
        while (n)
        {
          stack_.push_back(n);
          n = n->left;
        }
        // Помечаем, что pair1 будет обработан первым
        if (!stack_.empty())
        {
          stack_.back()->pair2Processed = false;
        }
      }

      std::deque< node* > stack_;
    };

    iterator begin()
    {
      return iterator(root_);
    }

    iterator end()
    {
      return iterator(nullptr);
    }

    iterator find(const Key& key)
    {
      iterator it = begin();
      iterator end_it = end();

      while (it != end_it)
      {
        if (!a_less_then_b(it->first, key) && !a_less_then_b(key, it->first))
        {
          return it;
        }
        // Для 3-узлов проверяем оба ключа
        if (it.stack_.back()->isThreeNode)
        {
          if (!a_less_then_b(it.stack_.back()->pair2.first, key) &&
            !a_less_then_b(key, it.stack_.back()->pair2.first))
          {
            return it;
          }
        }
        ++it;
      }
      return end_it;
    }

  private:
    node* root_;
    size_t size_;
    std::vector< node* > deletes_;
  };
}

#endif
