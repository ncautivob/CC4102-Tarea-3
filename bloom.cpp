#include <bits/stdc++.h>
#include <iostream>
#include <cstdint>
#include "MurmurHash3.h"
using namespace std;
#include "rapidcsv.h"

// Applies the murmur hash to the word using a specific seed.
uint32_t murmur_hash(const string& word, uint32_t seed) {
    uint32_t hash;
    MurmurHash3_x86_32(word.c_str(), word.size(), seed, &hash);
    return hash;
}

// Function that creates hash functions
function<uint32_t(const string&)> create_murmur_hash_function(uint32_t seed) {
    return bind(murmur_hash, placeholders::_1, seed);
}

int main() { 
  int m, k;
  cin >> m >> k;

  int search_size = pow(2, 10); // = |N|

  // [unused position, 1, ..., m]
  vector <int> M(m+1, 0);
  // Hashing functions. We want to create k different hashing functions.
  vector <function<uint32_t(string&)>> h(k);

  // Create k different hash functions
  for (int i=0; i<k; ++i) {
      h[i] = create_murmur_hash_function(i + 1); // Seed i + 1 for uniqueness
  }

  // Let's import the strings from the two csv files.
  rapidcsv::Document doc("csv/Popular-Baby-Names-Final.csv");

  // all of the names:
  vector<string> names = doc.GetColumn<string>("Name");

  rapidcsv::Document doc2("csv/Film-Names.csv");

  // all of the film names:
  vector<string> film_names = doc2.GetColumn<string>("0");

  for(int j = 0; j<film_names.size(); j++){
    cout << film_names[j] << endl;
  }


  // Experimentation:
  // We should apply the bloom filter to all baby names
  // So we have the final 'M' value.
  // With this M value, we can then do the experimentation.
  // For every N value, we will do this:
  // N/

  // IDEA: We should load all baby names and film names at first,
  // that way we don't have to load them every time and we just select a random few of them
  // for every iteration.

  // Insertion using Bloom's filter
  for (string &S: names) { // this shouldn't say 'names'
    for (int i=0; i<k; i++) {
      uint32_t j = (h[i](S) % m) + 1; // We make sure that the result is between 1 and m.
      //int j = h[i](S);
      M[(int)j] = 1; // This casting is ok since j is between 1 and m.
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