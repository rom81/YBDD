// Multifarious and sundry includes
#include <vector>
using std::vector;
#include <cstdlib>
#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
#include <ext/hash_map>
using __gnu_cxx::hash_map;
#include <utility>
using std::pair;
#include <list>
using std::list;

// This class represents our BDD nodes. It has all the variables (and more)
// plus all the methods that we need to represent our BDD nodes.
class bdd_node {

  // Variable descriptions:
  // index: is used to determine variable ordering for the BDD.
  // marked: used in BDD traversals (for size and print)
  // label: The name this node shows to the outside world.
  // low: The bdd_node which is the 'low' node
  // high: The bdd_node which is the 'high' node
  int index;
  int marked;
  char label[100];
  bdd_node* low;
  bdd_node* high;

public:

  // This is the constructor for the bdd_node.
  bdd_node(int indexIN, char* labelIN, bdd_node* lowIN,
	   bdd_node* highIN) {
    index = indexIN;
    strcpy(label, labelIN);
    low = lowIN;
    high = highIN;
    marked = false;
  }

  // Returns the index of the current node.
  inline int getIndex() { return index; }

  // Returns the 'low' BDD of this node.
  inline bdd_node* getLow() { return low; }

  // Returns the 'high' BDD of this node
  inline bdd_node* getHigh() { return high; }

  // Returns the label of this node.
  inline char* getLabel() { return label; }

  // Recursively traverses the BDD tree to determine the size of a given
  // BDD function. Sets the marked field once a node has been counted, so
  // that a node isn't counted twice.
  int Size() {
    if (marked) {
      return 0;
    } else {
      marked = true;
      if ((low != NULL) && (high != NULL)) {
	return 1 + high->Size() + low->Size();
      } else {
	if ((low == NULL) && (high == NULL)) {
	  return 1;
	} else {
	  if (low != NULL) {
	    return 1 + low->Size();
	  } else {
	    return 1 + high->Size();
	  }
	}
      }
    }
  }

  // Clears all the marked fields in a BDD tree, after the fields have been
  // set by Size() or Print().
  void cleanMarks() {
    if (marked) {
      marked = false;
      if (low != NULL) low->cleanMarks();
      if (high != NULL) high->cleanMarks();
    }
  }

  // This recursively prints the structure of a BDD tree. It uses the marked
  // field similarly to the Size() method. It takes in a String: TabsIn, to
  // keep track of how far to tab the current node in.
  void Print(char* TabsIn) {
    if ((low == NULL) && (high == NULL)) {
      cout << TabsIn << "[" << label << "]" << endl;
    } else {
      cout << TabsIn << label << ":bdd_node@" << this << endl;
    }
    if (!(marked)) {
      marked = true;
      char *str = new char[strlen(TabsIn)+strlen("\t")+1];
      strcpy(str, TabsIn);
      strcat(str, "\t");
      if (low != NULL) low->Print(str);
      if (high != NULL) high->Print(str);
    }
  }
};


// A hash table whose key type is a C string and whose data type is
// either (1) another C string or (2) a bdd_node
template <typename Data>
class HashTable {
private:
  // The internal, actual hash table
  hash_map<int, list<pair<char *, Data> > > hashTable;

  typedef typename list<pair<char *, Data> >::iterator list_iter;
  typedef typename hash_map<int, list<pair<char *, Data> > >::iterator
    hash_iter;

  // Number of entries
  int size;

public:
  HashTable() : size(0) { }

  ~HashTable() {
    for (hash_iter hitr = hashTable.begin(); hitr != hashTable.end();
	 ++hitr) {
      for (list_iter litr = (*hitr).second.begin(); litr !=
	     (*hitr).second.end(); ++litr)
	delete[] (*litr).first;

      // We have to leave it up to the user of this class to call the
      // appropriate version of delete on the pointer stored as data
      (*hitr).second.clear();
    }
  }

  const inline int getSize() const { return size; }

  // We refuse to insert multiple instances of the same key, so we
  // return a bool indicating whether we actually inserted anything
  bool insert(const char *key, Data element) {
    if (key == NULL) {
      cerr << "Error: attempted to insert() with a NULL string as a"
	   << " hash table key!" << endl;
      exit(1);
    }
    int hashVal = hash_djb2(key);
    pair<char *, Data> p;
    if (!lookup(hashVal, key)) {
      char *str = new char[strlen(key) + 1];
      strcpy(str, key);
      p.first = str;
      p.second = element;
      hashTable[hashVal].push_back(p);
      ++size;
      return true;
    } else return false;
  }

  // Attempts to delete the data associated with key; fails silently.
  void remove(const char *key) {
    if (key == NULL) {
      cerr << "Error: called remove() with a NULL string as a hash"
	   << " table key!" << endl;
      exit(1);
    }
    int hashVal = hash_djb2(key);
    list_iter itr;
    if (lookup(hashVal, key, itr)) {
      delete[] (*itr).first;
      hashTable[hashVal].erase(itr);
      if (hashTable[hashVal].size() == 0)
	hashTable.erase(hashVal);
      --size;
    }
  }

  // Returns either (1) the Data associated with key, if (key, data)
  // is in the hash table, or (2) NULL
  Data lookup(const char *key) {
    if (key == NULL) {
      cerr << "Error: called lookup() with key == NULL" << endl;
      exit(1);
    }
    list_iter itr;
    if (lookup(hash_djb2(key), key, itr))
      return (*itr).second;
    else return reinterpret_cast<Data>(NULL);
  }

private:

  // A fast and good C string |-> integer hash function
  int hash_djb2(const char *str) const {
    int c, hashVal = 5381;

    while (c == *str++)
      hashVal = ((hashVal << 5) + hashVal) + c; /* hashVal * 33 + c */

    return hashVal;
  }

  // "Internal" lookup function.  Returns whether the search was
  // successful; in the event of a successful search, an iterator to
  // the list elt is returned by reference into itr
  bool lookup(const int hashVal, const char *key, list_iter &itr) {
    if (hashTable.count(hashVal) > 0)
      for (itr = hashTable[hashVal].begin();
	   itr != hashTable[hashVal].end(); ++itr)
	if (!strcmp((*itr).first, key)) return true;
    return false;
  }

  // A lookup function for when we don't care where the element is in
  // the list
  inline bool lookup(const int hashVal, const char *key) {
    list_iter useless;
    return lookup(hashVal, key, useless);
  }
};


// Define the types actually used in bdd_functs.h
typedef HashTable<bdd_node *> Hash_Table;


// Rather than defining Hash_String_Table the way it was defined
// before, there's something smarter (and correct) that we can do
//
// This class is an int |-> C string map.
//
// Note; this class assumes (correctly, given its usage in ybdd) that
// the keys are inserted in order; first `0' is inserted, then `1',
// and so on
class Hash_String_Table {
public:
  ~Hash_String_Table() {
    hash_map<int, char *>::iterator itr, end = table.end();
    for (itr = table.begin(); itr != end; ++itr)
      if ((*itr).second) delete[] (*itr).second;
  }

  const inline int getSize() const { return table.size(); }

  // We refuse to insert multiple instances of the same key, so we
  // return a bool indicating whether we actually inserted anything
  bool insert(const int key, char *data) {
    if (table.count(key) == 0) {
      char *cpy = new char[strlen(data) + 1];
      strcpy(cpy, data);
      table[key] = cpy;
      return true;
    }
    return false;
  }

  // Attempts to delete the data associated with key; fails silently.
  inline void remove(const int key) {
    if (table.count(key) > 0) table.erase(key);
  }

  // Returns either (1) the Data associated with key, if (key, data)
  // is in the hash table, or (2) NULL
  const char *lookup(const int key) {
    if (table.count(key) > 0)
      return table[key];
    return NULL;
  }

private:
  hash_map<int, char *> table;
};
