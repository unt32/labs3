#include <algorithm>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <list>
#include <memory>
#include <random>
#include <sstream>
#include <string>
#include <unordered_set>
#include <vector>

using namespace std;

const int NIL = -1; // пустая ячейка
const int DELETED =
    -2; // «надгробие» для удалённых элементов (открытая адресация)

struct InputEnd {}; // конец ввода (EOF)

// ---------- Ввод ----------

template <class T> T readValue(const string &prompt, T lo, T hi) {
  while (true) {
    cout << prompt;
    T v;
    if (!(cin >> v)) {
      if (cin.eof())
        throw InputEnd();
      cin.clear();
      cin.ignore(numeric_limits<streamsize>::max(), '\n');
      cout << "  Ошибка ввода, повторите.\n";
      continue;
    }
    if (v < lo || v > hi) {
      cout << "  Допустимый диапазон: [" << lo << ", " << hi << "]\n";
      continue;
    }
    return v;
  }
}

// ---------- Хеш-функции ----------

int hashDiv(int k, int m) { return k % m; }

int hashMul(int k, int m, double A) {
  double p = k * A;
  return static_cast<int>(floor(m * (p - floor(p))));
}

// ---------- Функции проб ----------

int hashLinear(int k, int i, int m) {
  return static_cast<int>((k % m + (long long)i) % m);
}

int hashQuadratic(int k, int i, int m, int c1, int c2) {
  long long v = k % m + (long long)c1 * i + (long long)c2 * i * i;
  return static_cast<int>(v % m);
}

int hashDouble(int k, int i, int m) {
  long long h1 = k % m;
  long long h2 = 1 + k % (m - 1);
  return static_cast<int>((h1 + (long long)i * h2) % m);
}

// ---------- Базовый класс таблицы ----------

class Table {
public:
  virtual ~Table() {}
  // Возвращает позицию (индекс ячейки / номер цепочки) или -1 (NIL / ошибка)
  virtual int insert(int k) = 0;
  virtual int search(int k) const = 0;
  virtual bool remove(int k) = 0;
  virtual void print() const = 0;
  virtual string describe() const = 0;
  virtual long long maxKey() const = 0; // максимально допустимый ключ
  virtual int size() const = 0;         // размер таблицы
  int count() const { return n; }

protected:
  int n = 0; // число элементов
};

// ---------- Прямая адресация ----------

class DirectTable : public Table {
  vector<int> T;

public:
  explicit DirectTable(int u) : T(u, NIL) {}

  int insert(int k) override { // DIRECT-ADDRESS-INSERT: T[k] <- k
    if (T[k] == NIL)
      n++;
    T[k] = k;
    return k;
  }
  int search(int k) const override { // DIRECT-ADDRESS-SEARCH: return T[k]
    return T[k] == NIL ? -1 : k;
  }
  bool remove(int k) override { // DIRECT-ADDRESS-DELETE: T[k] <- NIL
    if (T[k] == NIL)
      return false;
    T[k] = NIL;
    n--;
    return true;
  }
  void print() const override {
    int m = (int)T.size();
    cout << "  Элементов: " << n << " из " << m << "\n";
    for (int i = 0; i < m; i++) {
      if (T[i] == NIL && m > 100)
        continue;
      cout << "  [" << i << "] "
           << (T[i] == NIL ? string("-") : to_string(T[i])) << "\n";
    }
  }
  string describe() const override {
    return "прямая адресация, размер универсума U = " + to_string(T.size());
  }
  long long maxKey() const override { return (long long)T.size() - 1; }
  int size() const override { return (int)T.size(); }
};

// ---------- Закрытая адресация (цепочки) ----------

class ChainTable : public Table {
  vector<list<int>> T;
  int m;
  bool useMul;
  double A;

  int h(int k) const { return useMul ? hashMul(k, m, A) : hashDiv(k, m); }

public:
  ChainTable(int size, bool mul, double a)
      : T(size), m(size), useMul(mul), A(a) {}

  int insert(int k) override { // CHAINED-HASH-INSERT: в начало списка
    int j = h(k);
    T[j].push_front(k);
    n++;
    return j;
  }
  int search(int k) const override { // CHAINED-HASH-SEARCH
    int j = h(k);
    for (int x : T[j])
      if (x == k)
        return j;
    return -1;
  }
  bool remove(int k) override { // CHAINED-HASH-DELETE
    list<int> &l = T[h(k)];
    for (auto it = l.begin(); it != l.end(); ++it)
      if (*it == k) {
        l.erase(it);
        n--;
        return true;
      }
    return false;
  }
  void print() const override {
    cout << "  Элементов: " << n << ", m = " << m << ", alpha = " << fixed
         << setprecision(3) << (double)n / m << "\n";
    for (int i = 0; i < m; i++) {
      if (T[i].empty() && m > 100)
        continue;
      cout << "  [" << i << "]";
      if (T[i].empty())
        cout << " -";
      for (int x : T[i])
        cout << " -> " << x;
      cout << "\n";
    }
  }
  string describe() const override {
    string s = "цепочки, m = " + to_string(m) + ", хеш-функция: ";
    if (useMul) {
      ostringstream os;
      os << "HASH-MUL (A = " << A << ")";
      return s + os.str();
    }
    return s + "HASH-DIV";
  }
  long long maxKey() const override { return numeric_limits<int>::max(); }
  int size() const override { return m; }
};

// ---------- Открытая адресация ----------

enum Probe { LINEAR = 1, QUADRATIC = 2, DOUBLE = 3 };

class OpenTable : public Table {
  vector<int> T;
  int m;
  Probe probe;
  int c1, c2;

  int h(int k, int i) const {
    if (probe == LINEAR)
      return hashLinear(k, i, m);
    if (probe == QUADRATIC)
      return hashQuadratic(k, i, m, c1, c2);
    return hashDouble(k, i, m);
  }

public:
  OpenTable(int size, Probe p, int a, int b)
      : T(size, NIL), m(size), probe(p), c1(a), c2(b) {}

  // HASH-INSERT. Удалённые ячейки считаются свободными.
  int insert(int k) override {
    int i = 0;
    do {
      int j = h(k, i);
      if (T[j] == NIL || T[j] == DELETED) {
        T[j] = k;
        n++;
        return j;
      }
      i++;
    } while (i != m);
    return -1; // «хеш-таблица переполнена»
  }

  // HASH-SEARCH
  int search(int k) const override {
    int i = 0, j;
    do {
      j = h(k, i);
      if (T[j] == k)
        return j;
      i++;
    } while (T[j] != NIL && i != m);
    return -1;
  }

  bool remove(int k) override {
    int j = search(k);
    if (j < 0)
      return false;
    T[j] = DELETED;
    n--;
    return true;
  }

  void print() const override {
    cout << "  Элементов: " << n << ", m = " << m << ", alpha = " << fixed
         << setprecision(3) << (double)n / m << "\n";
    for (int i = 0; i < m; i++) {
      if (T[i] < 0 && m > 100)
        continue;
      cout << "  [" << i << "] ";
      if (T[i] == NIL)
        cout << "-";
      else if (T[i] == DELETED)
        cout << "(удалено)";
      else
        cout << T[i];
      cout << "\n";
    }
  }
  string describe() const override {
    string s = "открытая адресация, m = " + to_string(m) + ", ";
    if (probe == LINEAR)
      return s + "HASH-LINEAR";
    if (probe == QUADRATIC)
      return s + "HASH-QUADRATIC (c1 = " + to_string(c1) +
             ", c2 = " + to_string(c2) + ")";
    return s + "HASH-DOUBLE";
  }
  long long maxKey() const override { return numeric_limits<int>::max(); }
  int size() const override { return m; }
};

// ---------- Случайные ключи ----------

// cnt уникальных ключей из [0, hi] (алгоритм Флойда), сид случайный
vector<int> randomKeys(int cnt, long long hi, mt19937_64 &gen) {
  unordered_set<long long> used;
  vector<int> keys;
  for (long long j = hi - cnt + 1; j <= hi; j++) {
    uniform_int_distribution<long long> d(0, j);
    long long t = d(gen);
    if (!used.insert(t).second) {
      t = j;
      used.insert(t);
    }
    keys.push_back((int)t);
  }
  shuffle(keys.begin(), keys.end(), gen);
  return keys;
}

// ---------- Команды ----------

void create(unique_ptr<Table> &table) {
  cout << "Тип адресации:\n"
       << "  1 - прямая\n  2 - закрытая (цепочки)\n  3 - открытая\n";
  int type = readValue<int>("Выбор: ", 1, 3);

  if (type == 1) {
    int u = readValue<int>("Размер универсума U (ключи 0..U-1): ", 1, 10000000);
    table = make_unique<DirectTable>(u);
  } else if (type == 2) {
    int m = readValue<int>("m = ", 1, 1000000);
    cout << "Хеш-функция: 1 - HASH-DIV, 2 - HASH-MUL\n";
    int f = readValue<int>("Выбор: ", 1, 2);
    double A = 0.61803;
    if (f == 2)
      A = readValue<double>("A (0 < A < 1) = ", 1e-9, 1.0 - 1e-9);
    table = make_unique<ChainTable>(m, f == 2, A);
  } else {
    int m = readValue<int>("m (>= 2) = ", 2, 1000000);
    cout << "Функция проб: 1 - линейная, 2 - квадратичная, 3 - двойная\n";
    int p = readValue<int>("Выбор: ", 1, 3);
    int c1 = 1, c2 = 3;
    if (p == 2) {
      c1 = readValue<int>("c1 = ", 0, 1000000);
      c2 = readValue<int>("c2 = ", 0, 1000000);
    }
    table = make_unique<OpenTable>(m, static_cast<Probe>(p), c1, c2);
  }
  cout << "Создана таблица: " << table->describe() << "\n";

  int fill = readValue<int>(
      "Заполнить случайными значениями? (1 - да, 0 - нет): ", 0, 1);
  if (!fill)
    return;

  long long hi = table->maxKey();
  long long limit = hi + 1;
  if (type == 3)
    limit = min(limit, (long long)table->size());
  int cnt = (int)min<long long>(limit, 10000000);
  cnt = readValue<int>(
      "Сколько ключей добавить (макс. " + to_string(cnt) + "): ", 0, cnt);

  mt19937_64 gen(random_device{}());
  vector<int> keys = randomKeys(cnt, hi, gen);
  int fails = 0;
  for (int k : keys)
    if (table->insert(k) < 0)
      fails++;
  cout << "Добавлено: " << cnt - fails << ", отказов: " << fails
       << " (диапазон ключей: 0.." << hi << ")\n";
}

void add(Table &t) {
  int k = readValue<int>("Ключ (0.." + to_string(t.maxKey()) + "): ", 0,
                         (int)t.maxKey());
  if (t.search(k) >= 0) {
    cout << "  Ключ уже есть в таблице.\n";
    return;
  }
  int pos = t.insert(k);
  if (pos < 0)
    cout << "  Ошибка: хеш-таблица переполнена (или цикл проб).\n";
  else
    cout << "  Ключ добавлен, позиция: " << pos << "\n";
}

void del(Table &t) {
  int k = readValue<int>("Ключ (0.." + to_string(t.maxKey()) + "): ", 0,
                         (int)t.maxKey());
  if (t.remove(k))
    cout << "  Ключ удалён.\n";
  else
    cout << "  Ключ не найден.\n";
}

void findKey(const Table &t) {
  int k = readValue<int>("Ключ (0.." + to_string(t.maxKey()) + "): ", 0,
                         (int)t.maxKey());
  int pos = t.search(k);
  if (pos < 0)
    cout << "  Ключа " << k << " в таблице нет.\n";
  else
    cout << "  Ключ " << k << " есть в таблице, позиция: " << pos << "\n";
}

void printHelp() {
  cout << "\th, help   - справка\n"
       << "\tc, create - создать таблицу\n"
       << "\ta, add    - добавить элемент\n"
       << "\td, del    - удалить элемент\n"
       << "\tf, find   - проверить, есть ли ключ в таблице\n"
       << "\tp, print  - вывести таблицу\n"
       << "\te, exit   - выход\n";
}

int main() {
  unique_ptr<Table> table;
  try {
    // printHelp();
    string command;
    while (true) {
      cout << "> ";
      if (!(cin >> command))
        break;
      if (command == "h" || command == "help")
        printHelp();
      else if (command == "c" || command == "create")
        create(table);
      else if (command == "e" || command == "exit")
        return 0;
      else if (command == "a" || command == "add" || command == "d" ||
               command == "del" || command == "f" || command == "find" ||
               command == "p" || command == "print") {
        if (!table) {
          cout << "Сначала создайте таблицу (c).\n";
          continue;
        }
        if (command == "a" || command == "add")
          add(*table);
        else if (command == "d" || command == "del")
          del(*table);
        else if (command == "f" || command == "find")
          findKey(*table);
        else {
          cout << table->describe() << "\n";
          table->print();
        }
      } else
        cout << "Неизвестная команда. Введите help.\n";
    }
  } catch (const InputEnd &) {
    cout << "\nКонец ввода.\n";
  }
  return 0;
}
