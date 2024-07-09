#include <bits/stdc++.h>
using namespace std;

// TODO: this doesn't work
int stringToAscii(string S) {
  int result = 0;
  for (int i = 0; i < S.length(); i++) {
    result += static_cast<int>(S[i]);
  }
  return result;
}

int main() { 
  int m, k;
  cin >> m >> k;

  int search_size = pow(2, 10);

  // [unused position, 1, ..., m]
  vector <int> M(m+1, 0);
  // Hashing functions
  vector <function<int(string)>> h(k);
  vector <int> p(k);
  // Babies' names
  vector <string> names;

  // TODO: library to decompose an Excel file. Idea: write in a txt file and read using if/ofstream.
  // TODO: library to generate distinct prime numbers, in order to create the hashing functions

  // Hashing functions creation
  for (int i = 0; i < k; i++) {
    // TODO: document why we transform a string to an int using stringToAscii(...).
    // TODO: implement string hashing https://cp-algorithms.com/string/string-hashing.html
    auto func = [m, i, &p](string S) { return ((stringToAscii(S) % p[i]) % m) + 1; }; 
    h[i] = func;
  }

  // Insertion using Bloom's filter
  for (string S: dict) {
    for (int i = 0; i < k; i++) {
      int j = h[i](S);
      M[j] = 1;
    }
  }

  // To get the S' vector of search names that passed Bloom's filter.
  vector <string> search_names(search_size);
  // To know whether a name is already used or not in O(1).
  // 0 := it's NOT used, 1 := it's already used.

  // We can use size() method because at this point that vector is already initialized.
  vector <int> used_names(names.size(), 0);

  int inserted = 0;
  while (inserted < search_size) {
    int random_index = rand() % names.size();
    if (used_names[random_index]) continue;

    // Indicates if the current name passed Bloom's filter.
    bool pass = true;

    for (int i = 0; i < k; i++) {
      // If one of the hashing results is zero, this string is not in the database.
      if (!h[i](names[random_index])) {
        pass = false;
        break;
      }
    }

    if (pass) {
      search_names[inserted] = names[random_index];
      used_names[random_index] = 1;
      inserted++;
    }
  }

  return 0;
}