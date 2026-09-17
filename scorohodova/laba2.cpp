#include <cassert>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <limits>
#include <random>
#include <vector>

using namespace std;
using namespace chrono;

#define COLUMN 15

void merge(vector<int> &A, int p, int q, int r) {
  int n1 = q - p + 1;
  int n2 = r - q;

  vector<int> L(n1 + 1), R(n2 + 1);

  for (int i = 0; i < n1; i++)
    L[i] = A[p + i];
  for (int j = 0; j < n2; j++)
    R[j] = A[q + 1 + j];

  L[n1] = numeric_limits<int>::max();
  R[n2] = numeric_limits<int>::max();

  int i = 0, j = 0;
  for (int k = p; k <= r; k++) {
    if (L[i] <= R[j]) {
      A[k] = L[i];
      i++;
    } else {
      A[k] = R[j];
      j++;
    }
  }
}

void mergeSort(vector<int> &A, int p, int r) {
  if (p < r) {
    int q = (p + r) / 2;
    mergeSort(A, p, q);
    mergeSort(A, q + 1, r);
    merge(A, p, q, r);
  }
}

int partition(vector<int> &A, int p, int r) {
  int x = A[r];
  int i = p - 1;

  for (int j = p; j <= r - 1; j++) {
    if (A[j] <= x) {
      i++;
      swap(A[i], A[j]);
    }
  }
  swap(A[i + 1], A[r]);
  return i + 1;
}

void quickSort(vector<int> &A, int p, int r) {
  if (p >= r)
    return;
  int q = partition(A, p, r);
  quickSort(A, p, q - 1);
  quickSort(A, q + 1, r);
}

void bubbleSort(vector<int> &A) {
  int n = A.size();
  for (int i = 1; i <= n - 1; i++) {
    for (int j = 0; j < n - i; j++) {
      if (A[j] > A[j + 1]) {
        swap(A[j], A[j + 1]);
      }
    }
  }
}

vector<int> generateRandomArray(int size, int minVal, int maxVal) {
  vector<int> arr(size);
  random_device rd;
  mt19937 gen(rd());
  uniform_int_distribution<> dist(minVal, maxVal);

  for (int i = 0; i < size; i++) {
    arr[i] = dist(gen);
  }
  return arr;
}

bool isSorted(const vector<int> &A) {
  for (size_t i = 1; i < A.size(); i++) {
    if (A[i - 1] > A[i])
      return false;
  }
  return true;
}

template <typename Func> double measureTime(Func sortFunc, vector<int> arr) {
  auto start = chrono::high_resolution_clock::now();
  sortFunc(arr);
  auto end = chrono::high_resolution_clock::now();

  assert(isSorted(arr));

  chrono::duration<double, milli> elapsed = end - start;
  return elapsed.count();
}

int main() {
  vector<int> sizes = {1000, 5000, 10000, 20000, 50000, 100000};

  cout << fixed << setprecision(3);
  cout << left << setw(COLUMN) << "Size" << setw(COLUMN) << "MergeSort"
       << setw(COLUMN) << "QuickSort" << setw(COLUMN) << "BubbleSort" << endl;
  cout << string(COLUMN * 4, '-') << endl;

  for (int size : sizes) {
    vector<int> original = generateRandomArray(size, -100000, 100000);

    double mergeTime = measureTime(
        [](vector<int> &a) { mergeSort(a, 0, a.size() - 1); }, original);

    double quickTime = measureTime(
        [](vector<int> &a) { quickSort(a, 0, a.size() - 1); }, original);

    double bubbleTime =
        measureTime([](vector<int> &a) { bubbleSort(a); }, original);

    cout << left << setw(COLUMN) << size << setw(COLUMN) << mergeTime
         << setw(COLUMN) << quickTime << setw(COLUMN) << bubbleTime << endl;
  }

  return 0;
}
