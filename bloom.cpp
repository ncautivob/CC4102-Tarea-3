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
 * Recordar el comando que usamos la otra vez para debuggear
 * g++ -Wall -O2 -fsanitize=address bloom.cpp MurmurHash3.cpp -o bloom
 * y recuerden usar WSL yay 
*/

/** All code associated with murmur hash was done with a bit of investigation
 * And what could be found on the internet.
*/

// Applies the murmur hash to the word using a specific seed.
uint32_t murmur_hash(const string& word, uint32_t seed) {
    uint32_t hash;
    MurmurHash3_x86_32(word.c_str(), word.size(), seed, &hash);
    return hash;
}

// Function that creates hash functions with a certain seed
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

// Function that checks if a file exists
bool existsFile(const string &name) {
  ifstream f(name.c_str());
  return f.good();
}

// Function that, for grep search,
// saves both set of search names and multiset of filmnames in the same file
void names_to_file(unordered_set<string>& search_names,
                  unordered_multiset<string>& search_filmnames,
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

// Function that, for bloom filter search,
// writes all of the names that passed the filter in a file
void names_to_file2(unordered_multiset<string>& search_names,
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

// Function that writes all names in a file, to apply grep search safely over a txt
// rather than the csv itself.
void names_to_file3(vector<string>& names,
                  string& filename) {
  ofstream file(filename);
  if (file.is_open()) {
      for (const auto& name : names) {
          file << name << "\n";
      }
      file.close();
  } else {
      cerr << "An error ocurred :(.\n";
  }
}

// Function that applies the grep search!
void grep_search(string &names_file, string &search_file) {

  /** Explanation of the command:
   * -F is for 'fixed string' because we want fixed strings rather than to use regular expressions
   * And we want them to be interpreted just as they are written
   * Rather than treating special characters as regex things
   * -x is because we're looking for exact matches
   * -f is for file! it tells grep to take the search patterns from a file
   * 
   * then, 'search_file' is the file that has the strings we will look for in 'names_file'
   * 
   * and to correctly retrieve the results afterwards, we redirect them to a txt
  */
  string command = "grep -F -x -f " + search_file + " " + names_file + " > grep_results.txt";
  system(command.c_str());

  // We will now read the results from the txt and print them!
  // note: this might be deleted soon, as prints add a lot to the execution time...
  //ifstream grep_results("grep_results.txt");
  //string result;
  //grep_results.close();

  // We then remove grep results as we don't really need it
  remove("grep_results.txt");
}

// Function that applies bloom filter to a name
int apply_bloom_filter(const string &name, vector<int> &M, vector <function<uint32_t(const string&)>> &h, int k, int m) {
  for (int i = 0; i < k; i++){
    function<uint32_t(const string&)> hash = h[i]; // The hash function in the 'i' position
    int j = (int) (h[i](name) % m) + 1; // We convert the hash result, as it is a uint_32
    // and we also want it to be between 1 and m!
    if(M[j] != 1){
      // If even one of the results is not 1, we can say with certainty that the word is NOT in storage.
      return 0;
    }
  }
  // If all of them were indeed '1', then the name passed the filter!
  return 1;
}

int main() {
  // We'll delete the directories for the results txt's so the results are not overwritten.
  // And then create it again
  deleteDirectory("bloom-results");
  createDirectory("bloom-results");

  deleteDirectory("grep-results");
  createDirectory("grep-results"); 

  // Let's import the strings from the two csv files.
  rapidcsv::Document doc("csv/Popular-Baby-Names-Final.csv");

  // all of the names:
  vector<string> names = doc.GetColumn<string>("Name");
  int names_length = names.size();

  string names_terms_file = "name_file.txt";
  // We'll save it in a txt so it's easier to work with later with grep search
  // This txt will also be called "storage"
  names_to_file3(names, names_terms_file);

  rapidcsv::Document doc2("csv/Film-Names.csv");

  // all of the film names:
  vector<string> film_names = doc2.GetColumn<string>("0");
  int filmnames_length = film_names.size();

  vector<int> m_values = {static_cast<int>(pow(2, 17)),
                          static_cast<int>(pow(2, 18)),
                          static_cast<int>(pow(2, 19))};

  //vector<int> k_values = {1, 4, 8}; // Potencias de 2

  for(int m: m_values){
    //for(int k: k_values){
      vector<int> N_values = {static_cast<int>(pow(2, 10)), static_cast<int>(pow(2, 12)),
                              static_cast<int>(pow(2, 14)), static_cast<int>(pow(2, 16))};

      vector<double> p_values = {0.0, 0.25, 0.5, 0.75, 1.0};

      // We'll do this experiment 20 times.
      // We'll check: search time and error rate (for bloom filter).
      for(int N : N_values) {

        vector<int> k_values = {(int)ceil((log(2) * m / names_length)), 2*(int)ceil(m / names_length)}; // Pensamos que m/N debería ser el de mejor rendimiento!

        for(int k : k_values){

          // [unused position, 1, ..., m]
          vector <int> M(m+1, 0);
          // Hashing functions. We want to create k different hashing functions.
          vector <function<uint32_t(const string&)>> h(k);

          // Create k different hash functions
          // We will use the previous 'create murmur hash function' with a different seed every time
          for (int i = 0; i < k; i++) {
              h[i] = create_murmur_hash_function(i + 1); // the using of the index i makes it unique!
          }

          // Experimentation:

          // We should apply the bloom filter to insert all baby names
          // So we have the final 'M' value.
          // With this M value, we can then do the experimentation.

          // Insertion using Bloom's filter
          for (string &S: names) {
            for (int i = 0; i < k; i++) {
              uint32_t j = (h[i](S) % m) + 1; // We make sure that the result is between 1 and m.
              //int j = h[i](S);
              M[(int)j] = 1; // This casting is ok since j is between 1 and m.
            }
          }

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

            // Unordered multiset that represents the strings from 'film names' that we will search.
            // It's a multiset because we don't have enough film names to allow all of them to be different
            // Considering we will need 2^16 different film names, if we took the other approach
            unordered_multiset <string> search_filmnames;

            while(search_filmnames.size() < N*(1.0-p)){
              int random_number = dis_filmnames(gen); // different distribution due to the different amount
              search_filmnames.insert(film_names[random_number]);
            }

            // Now that we have both sets, we should perform the searches!

            // First, without Bloom Filter:
            // To perform a 'grep' process without any issue,
            // we put all 'search names' in a file.
            // (it is also important to notice that we don't need the output,
            // just the final time of execution)

            // we will do this now:
            string search_terms_file = "search_terms.txt";
            names_to_file(search_names, search_filmnames, search_terms_file);

            // Now, let's perform the experiments for the sequential search.
            string currentFileName =
                "grep-results/time-results-" + to_string(N) + "-" + to_string(p) + ".txt";

            ofstream outputFile;

            /** Si existe, añadimos más contenidos. */
            if (existsFile(currentFileName)) {
              outputFile.open(currentFileName, ios_base::app);
            } else {
              outputFile.open(currentFileName);
            }

            if (outputFile.is_open()) {
              streambuf *originalStdout = cout.rdbuf(outputFile.rdbuf());

              cout << "Tamaño de M: " << to_string(m) << "\n"
              << "Tamaño de k: " << to_string(k) << "\n"
              << "Número de nombres: " << to_string(N)
              << ", Proporción de palabras en csv: " << to_string(p) << "\n";

              // Let's now count!
              auto start = high_resolution_clock::now();

              // Search each name with grep
              grep_search(names_terms_file, search_terms_file);

              auto stop = high_resolution_clock::now();
              auto duration = duration_cast<milliseconds>(stop - start);

              cout <<"Tiempo de ejecución de búsquedas con grep (ms): "
              << to_string(duration.count()) << '\n';

              cout.rdbuf(originalStdout);
              outputFile.close();
            }

            // We will now perform the bloom filter search and check the execution times!
            string currentFileName2 =
                "bloom-results/time-results-" + to_string(N) + "-" + to_string(p) + ".txt";

            ofstream outputFile2;
            /** Si existe, añadimos más contenidos. */
            if (existsFile(currentFileName2)) {
              outputFile2.open(currentFileName2, ios_base::app);
            } else {
              outputFile2.open(currentFileName2);
            }

            if (outputFile2.is_open()) {
              streambuf *originalStdout2 = cout.rdbuf(outputFile2.rdbuf());
              cout << "Tamaño de M: " << to_string(m) << "\n"
              << "Tamaño de k: " << to_string(k) << "\n"
              << "Número de nombres: " << to_string(N)
              << ", Proporción de palabras en csv: " << to_string(p)
              << "\n";


            // Let's now count!
            auto start2 = high_resolution_clock::now();

            // Search each name with the bloom filter

            // originally a set... now a multiset to see what happens!
            unordered_multiset <string> search_after; // MAYBE THE ISSUE IS THAT THIS IS A SET?

            // Let's check for the search names first! They should all pass...
            for (const string &name : search_names) {
                int check = apply_bloom_filter(name, M, h, k, m);
                if(!check){
                  // The name did not pass the bloom filter, everything ok.
                  // This never actually happens!
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
                  //cout << "COLLISION!! YAY!!" << endl;
                  // We will add it to a list of names we will then search in the actual file.
                  search_after.insert(name);
                }
            }

            // Let's now put the search_after names in a file, so we can do a grep search on them afterwards.

            int after_size = search_after.size();
            int collisions = after_size - (N*p); // we substract the amount of baby names in N.

            // To be fair (cause we didn't count this for grep), we will stop time right here
            // And count it again after we pass the elements to the txt :)

            auto stop2 = high_resolution_clock::now();
            auto duration2 = duration_cast<milliseconds>(stop2 - start2);

            string after_terms_file = "after_terms.txt";
            names_to_file2(search_after, after_terms_file);

            // we start again!
            auto start3 = high_resolution_clock::now();

            // Search each name with grep
            grep_search(names_terms_file, after_terms_file);

            auto stop3 = high_resolution_clock::now();
            auto duration3 = duration_cast<milliseconds>(stop3 - start3);

            double error = (collisions*100) / (N*(1.0-p));

            cout << "Tiempo de ejecución de búsquedas con el filtro de bloom (ms): "
                  << to_string(duration2.count() + duration3.count()) << '\n'
                  << "Porcentaje de error del filtro: "
                  << to_string(error) << '\n'
                  << "Número de colisiones: "
                  << to_string(collisions) << '\n';

            cout.rdbuf(originalStdout2);
            outputFile2.close();
            }
          }
        }
      //};
    }
  }
  return 0;
}