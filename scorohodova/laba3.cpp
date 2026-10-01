#include <iostream>
#include <vector>
using namespace std;

int LEFT(int i) { return 2 * i; }
int RIGHT(int i) { return 2 * i + 1; }

void printArray(const vector<int> &A, int length) {
  for (int k = 1; k <= length; k++)
    cout << A[k] << " ";
  cout << "\n";
}

void printTree(const vector<int> &A, int heapSize) {
  int h = 0;
  while ((1 << h) <= heapSize)
    h++;

  const int w = 4;

  cout << string(60, '.') << "\n";
  for (int level = 0; level < h; level++) {
    int start = 1 << level;
    int span = (1 << (h - 1 - level)) * w;
    string line;

    for (int k = start; k <= min(2 * start - 1, heapSize); k++) {
      string s = to_string(A[k]);
      int center = (k - start) * span + span / 2;
      int pos = center - (int)s.size() / 2;
      if (pos < (int)line.size())
        pos = line.size();
      line.append(pos - line.size(), ' ');
      line += s;
    }
    cout << line << "\n";
  }
  cout << string(60, '.') << "\n";
}

void heapify(vector<int> &A, int heapSize, int i) {
  int l = LEFT(i);
  int r = RIGHT(i);
  int largest = i;
  if (l <= heapSize && A[l] > A[i])
    largest = l;
  if (r <= heapSize && A[r] > A[largest])
    largest = r;

  if (largest != i) {
    swap(A[i], A[largest]);
    heapify(A, heapSize, largest);
  }
}

void buildHeap(vector<int> &A, int &heapSize) {
  heapSize = A.size() - 1;
  for (int i = (A.size() - 1) / 2; i >= 1; i--)
    heapify(A, heapSize, i);
}

void heapSort(vector<int> &A, int &heapSize) {
  buildHeap(A, heapSize);
  for (int i = A.size() - 1; i >= 2; i--) {
    swap(A[1], A[i]);
    heapSize--;
    heapify(A, heapSize, 1);
  }
}

int main() {
  vector<int> H = {1, 99, 33, 5, 3, 37, 7, 44, 32, 57, 15};

  int heapSize = 0;

  buildHeap(H, heapSize);
  cout << "Heap: ";
  printArray(H, heapSize);
  printTree(H, heapSize);

  heapSort(H, heapSize);
  cout << "Sorted array is: ";
  printArray(H, H.size() - 1);

  return 0;
}
