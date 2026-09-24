#include <cassert>
#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <type_traits>

class Set {

  class Node {
    unsigned int height = 1;
    unsigned int leftHeight() { return left ? left->Height() : 0; }
    unsigned int rightHeight() { return right ? right->Height() : 0; }

  public:
    int data;
    Node *parent = nullptr;
    Node *left = nullptr;
    Node *right = nullptr;

    Node(int val = 0) { data = val; }
    ~Node() {}

    unsigned int Height() { return height; }

    void FixHeight() {
      if (leftHeight() > rightHeight())
        height = leftHeight();
      else
        height = rightHeight();
      height++;
    }

    int Factor() { return rightHeight() - leftHeight(); }
  };

  Node *root = nullptr;
  unsigned int size = 0;
  std::function<bool(int, int)> compare;

  bool isEqual(int a, int b) const { return !compare(a, b) && !compare(b, a); }

  Node *rotateLeft(Node *n) {
    Node *pn = n->parent;
    Node *b = n->right;

    n->right = b->left;
    if (n->right)
      n->right->parent = n;
    n->parent = b;

    b->left = n;
    b->parent = pn;

    n->FixHeight();
    b->FixHeight();
    return b;
  }

  Node *rotateRight(Node *n) {
    Node *pn = n->parent;
    Node *a = n->left;

    n->left = a->right;
    if (n->left)
      n->left->parent = n;
    n->parent = a;

    a->right = n;
    a->parent = pn;

    n->FixHeight();
    a->FixHeight();
    return a;
  }

  Node *insert(Node *r, Node *t, Node *p) {
    if (!r) {
      t->parent = p;
      size++;
      return t;
    }

    if (isEqual(t->data, r->data)) {
      delete t;
      return r;
    }

    if (compare(t->data, r->data))
      r->left = insert(r->left, t, r);
    else
      r->right = insert(r->right, t, r);

    r = balance(r);
    return r;
  }

  void print(Node *r) const {
    if (!r) {
      return;
    }

    print(r->left);

    if (r->left)
      std::cout << r->left->data;
    else
      std::cout << "null";
    std::cout << "\t";

    std::cout << r->data;
    std::cout << "\t";

    if (r->right)
      std::cout << r->right->data;
    else
      std::cout << "null";
    std::cout << std::endl;

    print(r->right);
  }

  Node *balance(Node *r) {
    r->FixHeight();
    if (r->Factor() == 2) {
      if (r->right->Factor() < 0)
        r->right = rotateRight(r->right);
      r = rotateLeft(r);
    } else if (r->Factor() == -2) {
      if (r->left->Factor() > 0)
        r->left = rotateLeft(r->left);
      r = rotateRight(r);
    }

    return r;
  }

  Node *clear(Node *r) {
    if (!r)
      return nullptr;

    clear(r->left);
    clear(r->right);

    delete r;

    return nullptr;
  }

  Node *remove(Node *r, int val) {
    if (!r)
      return nullptr;

    if (isEqual(r->data, val)) {

      // 0 sau 1 child
      if (!r->left || !r->right) {
        size--;
        Node *t = nullptr;
        if (r->left)
          t = r->left;
        else if (r->right)
          t = r->right;

        if (t)
          t->parent = r->parent;

        delete r;
        return t;
      }

      Node *t = r->left;
      while (t->right)
        t = t->right;

      r->data = t->data;
      r->left = remove(r->left, r->data);

      r = balance(r);
      return r;

    } else {
      if (compare(val, r->data))
        r->left = remove(r->left, val);
      else
        r->right = remove(r->right, val);
    }

    r = balance(r);
    return r;
  }

  bool contains(Node *r, int val) const {
    if (!r)
      return false;

    if (isEqual(r->data, val))
      return true;

    if (compare(val, r->data))
      return contains(r->left, val);

    return contains(r->right, val);
  }

  Node *find(Node *r, int val) const {
    if (!r)
      return nullptr;

    if (isEqual(r->data, val))
      return r;

    if (compare(val, r->data))
      return find(r->left, val);

    return find(r->right, val);
  }

  Node *lower_bound(Node *r, int val) const {
    Node *ans = nullptr;
    while (r) {
      if (isEqual(val, r->data)) {
        ans = r;
        break;
      }
      if (compare(val, r->data)) {
        ans = r;
        r = r->left;
      } else {
        r = r->right;
      }
    }
    return ans;
  }

  Node *upper_bound(Node *r, int val) const {
    Node *ans = nullptr;
    while (r) {
      if (compare(val, r->data)) {
        ans = r;
        r = r->left;
      } else {
        r = r->right;
      }
    }
    return ans;
  }

  bool checkBalance(Node *r) const {
    if (!r)
      return true;
    return r->Factor() <= 1 && r->Factor() >= -1 && checkBalance(r->left) &&
           checkBalance(r->right);
  }

public:
  class Iterator {
    Node *curr;
    const Set *owner;

    Node *findMax(Node *r) {
      if (!r)
        return nullptr;

      while (r->right)
        r = r->right;

      return r;
    }

  public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type = int;
    using difference_type = std::ptrdiff_t;
    using pointer = const int *;
    using reference = const int &;

    Iterator(Node *n, const Set *const t) {
      curr = n;
      owner = t;
    }

    reference operator*() const { return curr->data; }

    pointer operator->() const { return &(curr->data); }

    Iterator operator--(int) {
      Iterator t = *this;
      --(*this);
      return t;
    }
    Iterator &operator--() {
      if (!curr) {
        curr = findMax(owner->root);
        return *this;
      }

      if (curr->left) {
        curr = findMax(curr->left);
      } else {
        while (curr->parent && curr == curr->parent->left)
          curr = curr->parent;
        curr = curr->parent;
      }

      return *this;
    }

    Iterator operator++(int) {
      Iterator t = *this;
      ++(*this);
      return t;
    }
    Iterator &operator++() {
      if (!curr)
        return *this;

      if (curr->right) {
        curr = curr->right;
        while (curr->left)
          curr = curr->left;

      } else {
        while (curr->parent && curr == curr->parent->right)
          curr = curr->parent;
        curr = curr->parent;
      }

      return *this;
    }

    bool operator==(const Iterator &other) const { return curr == other.curr; }
    bool operator!=(const Iterator &other) const { return !(*this == other); }
  };

  bool Insert(int val) {
    unsigned int t = Size();
    root = insert(root, new Node(val), nullptr);
    return Size() - t == 1;
  }

  bool Remove(int val) {
    unsigned int t = Size();
    root = remove(root, val);
    return t - Size() == 1;
  }

  void Clear() {
    root = clear(root);
    size = 0;
  }

  bool Contains(int val) const { return contains(root, val); }

  unsigned int Size() const { return size; }

  void Print() const {
    print(root);
    std::cout << std::endl;
  }

  bool CheckBalance() const { return checkBalance(root); }

  Set() { compare = std::less<int>(); }

  Set(std::function<bool(int, int)> comp) { compare = comp; }

  Set(Iterator first, Iterator last, std::function<bool(int, int)> comp) {
    compare = comp;
    while (first != last)
      Insert(*(first++));
  }
  Set(const Set &other) { *this = other; }

  Set(Set &&other) { *this = other; }

  Set(std::initializer_list<int> init, std::function<bool(int, int)> comp) {
    compare = comp;
    for (int val : init)
      Insert(val);
  };

  ~Set() { Clear(); }

  bool operator==(const Set &other) const {
    if (this == &other)
      return true;

    Iterator ti = begin();
    Iterator oi = other.begin();

    while (ti != end() && oi != other.end())
      if (*(ti++) != *(oi++))
        return false;

    return ti == end() && oi == other.end();
  }

  bool operator!=(const Set &other) const { return !(*this == other); }

  Set &operator=(const Set &other) {
    if (this == &other)
      return *this;

    Clear();
    compare = other.compare;
    for (int val : other)
      Insert(val);

    return *this;
  }

  Set &operator=(Set &&other) {
    if (this == &other)
      return *this;

    Clear();
    root = other.root;
    size = other.size;
    compare = other.compare;

    other.root = nullptr;
    other.size = 0;

    return *this;
  }

  using iterator = Iterator;
  using reverse_iterator = std::reverse_iterator<Iterator>;

  Iterator begin() const {
    if (Size() == 0)
      return end();

    Node *t = root;
    while (t->left)
      t = t->left;

    return Iterator(t, this);
  }

  Iterator end() const { return Iterator(nullptr, this); }
  reverse_iterator rbegin() const { return reverse_iterator(end()); }
  reverse_iterator rend() const { return reverse_iterator(begin()); }

  Iterator Find(int key) const { return Iterator(find(root, key), this); }
  Iterator Lower_bound(int key) const {
    return Iterator(lower_bound(root, key), this);
  }
  Iterator Upper_bound(int key) const {
    return Iterator(upper_bound(root, key), this);
  }
};

int main() {
  auto cmp = std::less<int>();
  Set t{cmp};
  for (int i = 0; i < 50; i++) {
    t.Insert(i);
    assert(t.CheckBalance());
  }

  t.Remove(31);
  Set b(t.begin(), t.end(), cmp);

  b.Remove(1);
  Set a = b;
  auto ai = a.begin();

  // a.Print();

  std::cout << "t == a\t" << (t == a) << std::endl;
  a.Insert(1);
  std::cout << "t != a\t" << (t != a) << std::endl;

  return 0;
}

// https://github.com/AbsoluteVirtue/fcim_poo_21.6/tree/master/lab
