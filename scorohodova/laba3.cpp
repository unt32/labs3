#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using namespace std;

vector<int> heap_array(1, 0);
int heap_size = 0;
bool heap_built = false;

int array_size() { return static_cast<int>(heap_array.size()) - 1; }

int parent(int i) { return i / 2; }
int left_child(int i) { return 2 * i; }
int right_child(int i) { return 2 * i + 1; }

void printArray() {
  cout << "  [n = " << array_size() << ", heap = " << heap_size << "]\n";
  for (int i = 1; i <= array_size(); i++)
    cout << ' ' << heap_array[i];
  cout << endl;
}

bool isHeap() {
  for (int i = 2; i <= heap_size; i++)
    if (heap_array[parent(i)] < heap_array[i])
      return false;
  return true;
}

void HEAPIFY(int i) {
  int l = left_child(i);
  int r = right_child(i);
  int largest;
  if (l <= heap_size && heap_array[l] > heap_array[i])
    largest = l;
  else
    largest = i;
  if (r <= heap_size && heap_array[r] > heap_array[largest])
    largest = r;
  if (largest != i) {
    swap(heap_array[i], heap_array[largest]);
    HEAPIFY(largest);
  }
}

void BUILD_HEAP() {
  heap_size = array_size();
  for (int i = array_size() / 2; i >= 1; i--)
    HEAPIFY(i);
  heap_built = true;
}

void HEAP_SORT() {
  BUILD_HEAP();
  for (int i = array_size(); i >= 2; i--) {
    swap(heap_array[1], heap_array[i]);
    heap_size--;
    HEAPIFY(1);
  }
  heap_built = false;
}

int HEAP_EXTRACT_MAX() {
  if (heap_size < 1)
    throw runtime_error("Очередь пуста");
  int maximum = heap_array[1];
  heap_array[1] = heap_array[heap_size];
  heap_array[heap_size] = maximum;
  heap_size--;
  HEAPIFY(1);
  return maximum;
}

void HEAP_INSERT(int key) {
  if (heap_size == array_size())
    heap_array.push_back(0);
  heap_size++;
  int i = heap_size;
  while (i > 1 && heap_array[parent(i)] < key) {
    heap_array[i] = heap_array[parent(i)];
    i = parent(i);
  }
  heap_array[i] = key;
  heap_built = true;
}

void ensureHeap() {
  if (array_size() > 0 && !heap_built)
    BUILD_HEAP();
}

int readInt(const string &prompt) {
  int x;
  while (true) {
    cout << prompt;
    if (cin >> x)
      return x;
    cout << "Ошибка ввода. Введите целое число.\n";
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
  }
}

void setArray(const vector<int> &v) {
  heap_array.assign(1, 0);
  heap_array.insert(heap_array.end(), v.begin(), v.end());
  heap_size = array_size();
  heap_built = false;
}

void inputArray() {
  int n = readInt("n = ");
  if (n < 1) {
    cout << "n должно быть >= 1.\n";
    return;
  }
  vector<int> v(n);
  for (int i = 0; i < n; i++)
    v[i] = readInt("A[" + to_string(i + 1) + "] = ");
  setArray(v);
}

void randomArray() {
  const int n = 10;
  const int lo = 0;
  const int hi = 100;
  static mt19937 gen(random_device{}());
  uniform_int_distribution<int> dist(lo, hi);
  vector<int> v(n);
  for (int &x : v)
    x = dist(gen);
  setArray(v);
}

void printHelp() {
  cout << "\thelp, h     - справка\n"
       << "\tread, r     - ввод массива\n"
       << "\tgenerate, g - случайный массив\n"
       << "\tprint, p    - показать массив\n"
       << "\tbuild, b    - построить кучу\n"
       << "\theapify, hf - просеять элемент\n"
       << "\tsort, s     - сортировка\n"
       << "\textract, x  - извлечь максимум\n"
       << "\tinsert, i   - вставить число\n"
       << "\tcheck, c    - проверить кучу\n"
       << endl;
}

int main() {
  while (true) {
    string command;
    cout << "> ";
    if (!(cin >> command))
      break;

    try {
      if (command == "help" || command == "h") {
        printHelp();
      } else if (command == "read" || command == "r") {
        inputArray();
      } else if (command == "generate" || command == "g") {
        randomArray();
      } else if (command == "print" || command == "p") {
        printArray();
      } else if (command == "build" || command == "b") {
        if (array_size() == 0) {
          cout << "Массив пуст.\n";
          continue;
        }
        BUILD_HEAP();
      } else if (command == "heapify" || command == "hf") {
        if (heap_size < 1) {
          cout << "Куча пуста.\n";
          continue;
        }
        ensureHeap();
        int i = readInt("i = ");
        if (i < 1 || i > heap_size) {
          cout << "Индекс вне диапазона.\n";
          continue;
        }
        HEAPIFY(i);
      } else if (command == "sort" || command == "s") {
        if (array_size() == 0) {
          cout << "Массив пуст.\n";
          continue;
        }
        HEAP_SORT();
      } else if (command == "extract" || command == "x") {
        if (array_size() == 0) {
          cout << "Куча пуста.\n";
          continue;
        }
        ensureHeap();
        int mx = HEAP_EXTRACT_MAX();
        cout << "max = " << mx << "\n";
      } else if (command == "insert" || command == "i") {
        ensureHeap();
        int key = readInt("key = ");
        HEAP_INSERT(key);
      } else if (command == "check" || command == "c") {
        cout << (isHeap() ? "Куча корректна.\n" : "Свойство кучи нарушено.\n");
      } else if (command == "exit" || command == "quit" || command == "e") {
        return 0;
      } else {
        cout << "Неизвестная команда. Введите help." << endl;
      }
    } catch (const exception &e) {
      cout << "Ошибка: " << e.what() << "\n";
    }
  }
}
