#include <bits/stdc++.h>
#include <iostream>
#include <cstdint>
#include "MurmurHash3.h"
#include "rapidcsv.h"
#include <fstream>
#include <filesystem>
#include <random>
#include <cstdlib>
#include <chrono>
using namespace std;
using namespace std::chrono;

// HOLI
// INSTRUCCIONES POR SI LEEN ESTO
/**
 * Primero: tengo un poco la escoba con los archivos de los resultados
 * No sé por qué no se imprime el primer mensaje xd pero bueno
 * 
 * Tengo comentada una línea del grep search, (alrededor de la 226)
 * Hay que devolverla a como estaba para ver que eso funque
 * Actualmente el error es que se queda pegado y no cacho por qué
 * Pero es en algún lado de la parte del bloom filter XD hay que poner prints
 * 
 * Finalmente, recordar el comando que usamos la otra vez para debuggear
 * g++ -Wall -O2 -fsanitize=address bloom.cpp MurmurHash3.cpp -o bloom
 *
 * y recuerden usar WSL yay 
*/



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

// Function that creates a new directory
void createDirectory(const string &ruta) {
  filesystem::path dir(ruta);
  filesystem::create_directory(dir);
}

// Function that deletes a directory
void deleteDirectory(const string &ruta) {
  filesystem::path dir(ruta);
  filesystem::remove_all(dir);
}

bool existsFile(const string &name) {
  ifstream f(name.c_str());
  return f.good();
}

// Function that writes all names in a file
void names_to_file(unordered_set<string>& search_names,
                  unordered_set<string>& search_filmnames,
                  string& filename) {
  ofstream file(filename);
  if (file.is_open()) {
      for (const auto& name : search_names) {
          file << name << "\n";
      }
      for (const auto& name2 : search_filmnames) {
          file << name2 << "\n";
      }
      file.close();
  } else {
      cerr << "An error ocurred :(.\n";
  }
}

// Function that writes all names in a file
void names_to_file2(unordered_set<string>& search_names,
                  string& filename) {
  ofstream file(filename);
  if (file.is_open()) {
      for (const auto& name : search_names) {
          file << name << "\n";
      }
      file.close();
  } else {
      cerr << "An error ocurred :(.\n";
  }
}

void grep_search(vector<string>& names, string& search_file) {
  for(const auto& name : names){
    // we want for this to search every name in search_file in the 'names' vector.
    //string command = "grep -F -f " + search_file + " <<< \"" + name + "\"";
    string command = "grep -F -f " + search_file + " -e \"" + name + "\"";
    system(command.c_str());
  }
}

// Function that applies bloom filter to a name
int apply_bloom_filter(const string &name, vector<int> &M, vector <function<uint32_t(const string&)>> &h, int k, int m) {
  for (int i = 0; i < k; i++){
    function<uint32_t(const string&)> hash = h[i];
    int j = (int) (h[i](name) % m) + 1;
    if(M[j] != 1){
      return 0;
    }
  }
  // If all of them were indeed '1', then the name passed the filter!
  return 1;
}

int main() { 
  int m, k;
  cin >> m >> k;

  // [unused position, 1, ..., m]
  vector <int> M(m+1, 0);
  // Hashing functions. We want to create k different hashing functions.
  vector <function<uint32_t(const string&)>> h(k);

  // Create k different hash functions
  for (int i=0; i<k; ++i) {
      h[i] = create_murmur_hash_function(i + 1); // Seed i + 1 for uniqueness
  }

  // Let's import the strings from the two csv files.
  rapidcsv::Document doc("csv/Popular-Baby-Names-Final.csv");

  // all of the names:
  vector<string> names = doc.GetColumn<string>("Name");
  int names_length = names.size();

  rapidcsv::Document doc2("csv/Film-Names.csv");

  // all of the film names:
  vector<string> film_names = doc2.GetColumn<string>("0");
  int filmnames_length = film_names.size();

  // Experimentation:
  // We should apply the bloom filter to all baby names
  // So we have the final 'M' value.
  // With this M value, we can then do the experimentation.

  // Insertion using Bloom's filter
  for (string &S: names) {
    for (int i=0; i<k; i++) {
      uint32_t j = (h[i](S) % m) + 1; // We make sure that the result is between 1 and m.
      //int j = h[i](S);
      M[(int)j] = 1; // This casting is ok since j is between 1 and m.
    }
  }

  vector<int> N_values = {static_cast<int>(pow(2, 10)), static_cast<int>(pow(2, 12)),
                          static_cast<int>(pow(2, 14)), static_cast<int>(pow(2, 16))};

  vector<double> p_values = {0.0, 0.25, 0.5, 0.75, 1.0};

  // We'll delete the directories for the results txt's so the results are not overwritten.
  // And then create it again
  deleteDirectory("bloom-results");
  createDirectory("bloom-results");

  deleteDirectory("grep-results");
  createDirectory("grep-results");

  // We'll do this experiment 20 times.
  // We'll check: search time and error rate.
  for(int N : N_values) {
    for(double p : p_values){
      // Unordered set that represents the strings from 'names' that we will search.
      unordered_set <string> search_names;

        random_device rd;
        mt19937 gen(rd());
        uniform_int_distribution<> dis_names(0, names_length);
        uniform_int_distribution<> dis_filmnames(0, filmnames_length);

      while(search_names.size() < N*p){
        // Choose a random number between 0 and names_length-1
        // Then take that name from the 'names' vector and add it to search_names
        int random_number = dis_names(gen);
        search_names.insert(names[random_number]);
      }

      // Unordered set that represents the strings from 'film names' that we will search.
      unordered_set <string> search_filmnames;

      while(search_filmnames.size() < N*(1-p)){
        int random_number = dis_filmnames(gen);
        search_filmnames.insert(film_names[random_number]);
      }

      // Now that we have both sets, we should perform the searches!

      // First, without Bloom Filter:
      // We found that, to perform a 'grep' process,
      // it was necessary to put all 'search names' in a file.
      // (it is also important to notice that we don't need the output,
      // just the final time of execution)

      // we will do this now:
      string search_terms_file = "search_terms.txt";
      names_to_file(search_names, search_filmnames, search_terms_file);

      // Now, let's perform the experiments for the sequential search.
      string currentFileName =
          "grep-results/time-results-" + to_string(N) + "-" + to_string(p) + ".txt";

      ofstream outputFile;
      streambuf *originalStdout = cout.rdbuf(outputFile.rdbuf());

      /** Si existe, añadimos más contenidos. */
      if (existsFile(currentFileName)) {
        outputFile.open(currentFileName, ios_base::app);
      } else {
        outputFile.open(currentFileName);
      }

      if (outputFile.is_open()) {
        streambuf *originalStdout = cout.rdbuf(outputFile.rdbuf());
        cout << "Número de nombres: " << to_string(N)
        << ", Proporción de palabras en csv: " << to_string(p)
        << "\n";

        // Let's now count!
        auto start = high_resolution_clock::now();

        // Search each name with grep
        //grep_search(names, search_terms_file);

        auto stop = high_resolution_clock::now();
        auto duration = duration_cast<milliseconds>(stop - start);

        cout << "Tiempo de ejecución de búsquedas con grep (ms): "
              << to_string(duration.count()) << '\n';

        cout.rdbuf(originalStdout);
        outputFile.close();
      }

      // We will now perform the search and check the execution times!
      string currentFileName2 =
          "bloom-results/time-results-" + to_string(N) + "-" + to_string(p) + ".txt";

      ofstream outputFile2;
      streambuf *originalStdout2 = cout.rdbuf(outputFile2.rdbuf());

      /** Si existe, añadimos más contenidos. */
      if (existsFile(currentFileName2)) {
        outputFile.open(currentFileName2, ios_base::app);
      } else {
        outputFile2.open(currentFileName2);
      }

      if (outputFile2.is_open()) {
        streambuf *originalStdout2 = cout.rdbuf(outputFile2.rdbuf());
        cout << "Número de nombres: " << to_string(N)
        << ", Proporción de palabras en csv: " << to_string(p)
        << "\n";


      // Let's now count!
      auto start2 = high_resolution_clock::now();

      // Search each name with the bloom filter

      unordered_set <string> search_after;

      // Let's check for the search names first! They should all pass...
      for (const string &name : search_names) {
          int check = apply_bloom_filter(name, M, h, k, m);
          if(!check){
            // The name did not pass the bloom filter, everything ok.
            continue;
          }
          else{
            // The name passed the bloom filter!
            // We will add it to a list of names we will then search in the actual file.
            search_after.insert(name);
          }
      }
      // Now for the film names
      for (const string &name : search_filmnames) {
          int check = apply_bloom_filter(name, M, h, k, m);
          if(!check){
            // The name did not pass the bloom filter, everything ok.
            continue;
          }
          else{
            // The name passed the bloom filter!
            // We will add it to a list of names we will then search in the actual file.
            search_after.insert(name);
          }
      }

      // Let's now put the search_after names in a file, so we can do a grep search on them afterwards.

      int after_size = search_after.size();
      int collisions = after_size - (N*p); // we substract the amount of baby names in N.

      string after_terms_file = "after_terms.txt";
      names_to_file2(search_after, after_terms_file);

      // Search each name with grep
      grep_search(names, after_terms_file);

      auto stop2 = high_resolution_clock::now();
      auto duration2 = duration_cast<milliseconds>(stop2 - start2);

      int error = collisions*100 / (N*(1-p));

      cout << "Tiempo de ejecución de búsquedas con el filtro de bloom (ms): "
            << to_string(duration2.count()) << '\n'
            << "Porcentaje de error del filtro: "
            << to_string(error) << '\n';

      cout.rdbuf(originalStdout2);
      outputFile2.close();
      }
    }};

  return 0;
}