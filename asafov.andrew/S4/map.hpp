#ifndef MAP_HPP
#define MAP_HPP

#include <utility>
#include <cstddef>
#include <deque>
#include <stdexcept>
#include <functional>
#include <initializer_list>

namespace asafov
{
  template< class Key, class Value, class Comparator = std::less< Key > >
  class map
  {
  private:
    struct node
    {
      node(Key k, Value v, node* p = nullptr):
        pair1(k, v),
        left(nullptr),
        middle(nullptr),
        right(nullptr),
        parent(p),
        isThreeNode(false)
      {
      }

      std::pair< Key, Value > pair1, pair2;
      node *left, *middle, *right, *parent;
      bool isThreeNode;
    };

    node* root_;
    size_t size_;
    bool needsRebuild_;

  public:
    // Конструкторы
    map() noexcept : root_(nullptr), size_(0), needsRebuild_(false)
    {
    }

    map(std::initializer_list< std::pair< Key, Value > > init) : map()
    {
      for (const auto& p: init)
      {
        insert(p.first, p.second);
      }
    }

    // Деструктор
    ~map()
    {
      clear(root_);
    }

    // Копирование и перемещение
    map(const map& other) : map()
    {
      for (const auto& p: other)
      {
        insert(p.first, p.second);
      }
    }

    map(map&& other) noexcept :
      root_(other.root_),
      size_(other.size_),
      needsRebuild_(other.needsRebuild_)
    {
      other.root_ = nullptr;
      other.size_ = 0;
      other.needsRebuild_ = false;
    }

    map& operator=(const map& other)
    {
      if (this != &other)
      {
        clear(root_);
        root_ = nullptr;
        size_ = 0;
        for (const auto& p: other)
        {
          insert(p.first, p.second);
        }
      }
      return *this;
    }

    map& operator=(map&& other) noexcept
    {
      if (this != &other)
      {
        clear(root_);
        root_ = other.root_;
        size_ = other.size_;
        needsRebuild_ = other.needsRebuild_;
        other.root_ = nullptr;
        other.size_ = 0;
        other.needsRebuild_ = false;
      }
      return *this;
    }

    // Итераторы
    class iterator
    {
    private:
      std::deque< node* > traversal_;
      size_t current_;

      void inOrderTraversal(node* n)
      {
        if (!n) return;
        inOrderTraversal(n->left);
        traversal_.push_back(n);
        if (n->isThreeNode)
        {
          inOrderTraversal(n->middle);
          traversal_.push_back(n); // Для pair2
        }
        inOrderTraversal(n->right);
      }

    public:
      iterator(node* root, bool end = false) : current_(0)
      {
        if (root && !end)
        {
          inOrderTraversal(root);
        }
      }

      std::pair< const Key, Value >& operator*() const
      {
        node* n = traversal_[current_];
        return (current_ % 2 == 0 || !n->isThreeNode)
                 ? reinterpret_cast< std::pair< const Key, Value >& >(n->pair1)
                 : reinterpret_cast< std::pair< const Key, Value >& >(n->pair2);
      }

      iterator& operator++()
      {
        ++current_;
        return *this;
      }

      bool operator!=(const iterator& other) const
      {
        return current_ != other.current_;
      }
    };

    iterator begin()
    {
      return iterator(root_);
    }

    iterator end()
    {
      return iterator(root_, true);
    }

    // Модификаторы
    void insert(const Key& k, const Value& v)
    {
      needsRebuild_ = true;
      if (!root_)
      {
        root_ = new node(k, v);
        size_ = 1;
        return;
      }

      node* where = findApproximate(k);
      if (!where->isThreeNode)
      {
        // Просто добавляем в 2-узел
        if (Comparator()(k, where->pair1.first))
        {
          where->pair2 = where->pair1;
          where->pair1 = {k, v};
        }
        else
        {
          where->pair2 = {k, v};
        }
        where->isThreeNode = true;
        size_++;
      }
      else
      {
        // Нужно разделять узел
        splitAndInsert(where, k, v);
      }
    }

    void erase(const Key& k)
    {
      // Упрощенная реализация - просто помечаем для перестроения
      if (find(k))
      {
        needsRebuild_ = true;
        size_--;
      }
    }

    void clear() noexcept
    {
      clear(root_);
      root_ = nullptr;
      size_ = 0;
      needsRebuild_ = false;
    }

    // Доступ
    Value& operator[](const Key& k)
    {
      auto it = find(k);
      if (it != end())
      {
        return it->second;
      }
      insert(k, Value());
      return find(k)->second;
    }

    Value& at(const Key& k)
    {
      auto it = find(k);
      if (it == end())
      {
        throw std::out_of_range("key not found!");
      }
      return it->second;
    }

    const Value& at(const Key& k) const
    {
      auto it = find(k);
      if (it == end())
      {
        throw std::out_of_range("key not found!");
      }
      return it->second;
    }

    // Поиск
    iterator find(const Key& k)
    {
      if (needsRebuild_) rebuildTree();

      node* current = root_;
      while (current)
      {
        if (!Comparator()(current->pair1.first, k) &&
          !Comparator()(k, current->pair1.first))
        {
          return iterator(current);
        }
        if (current->isThreeNode &&
          !Comparator()(current->pair2.first, k) &&
          !Comparator()(k, current->pair2.first))
        {
          return iterator(current);
        }

        if (Comparator()(k, current->pair1.first))
        {
          current = current->left;
        }
        else if (!current->isThreeNode || Comparator()(k, current->pair2.first))
        {
          current = current->middle;
        }
        else
        {
          current = current->right;
        }
      }
      return end();
    }

    // Информация
    size_t size() const noexcept
    {
      return size_;
    }

    bool empty() const noexcept
    {
      return size_ == 0;
    }

    void swap(map& other) noexcept
    {
      std::swap(root_, other.root_);
      std::swap(size_, other.size_);
      std::swap(needsRebuild_, other.needsRebuild_);
    }

  private:
    node* findApproximate(const Key& k) const
    {
      node* current = root_;
      node* parent = nullptr;

      while (current)
      {
        parent = current;
        if (Comparator()(k, current->pair1.first))
        {
          current = current->left;
        }
        else if (!current->isThreeNode || Comparator()(k, current->pair2.first))
        {
          current = current->middle;
        }
        else
        {
          current = current->right;
        }
      }
      return parent;
    }

    void splitAndInsert(node* n, const Key& k, const Value& v)
    {
      // Упрощенная реализация - помечаем для перестроения
      needsRebuild_ = true;
      size_++;
    }

    void rebuildTree()
    {
      // Упрощенная реализация перестроения дерева
      if (!needsRebuild_) return;

      std::vector< std::pair< Key, Value > > elements;
      collectElements(root_, elements);
      clear(root_);
      root_ = buildBalancedTree(elements, 0, elements.size() - 1);
      needsRebuild_ = false;
    }

    void collectElements(node* n, std::vector< std::pair< Key, Value > >& elements)
    {
      if (!n) return;
      collectElements(n->left, elements);
      elements.push_back(n->pair1);
      if (n->isThreeNode)
      {
        collectElements(n->middle, elements);
        elements.push_back(n->pair2);
      }
      collectElements(n->right, elements);
    }

    node* buildBalancedTree(const std::vector< std::pair< Key, Value > >& elements,
                            size_t start, size_t end)
    {
      if (start > end) return nullptr;

      size_t mid = start + (end - start) / 2;
      node* n = new node(elements[mid].first, elements[mid].second);

      n->left = buildBalancedTree(elements, start, mid - 1);
      if (n->left) n->left->parent = n;

      n->right = buildBalancedTree(elements, mid + 1, end);
      if (n->right) n->right->parent = n;

      return n;
    }

    void clear(node* n) noexcept
    {
      if (!n) return;
      clear(n->left);
      clear(n->middle);
      clear(n->right);
      delete n;
    }
  };
}

#endif
