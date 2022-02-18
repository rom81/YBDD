// This file contains all the methods necessary to create a working 
// BDD package. The methods which need to be filled in are contained
// below. Please do not change the interfaces to these functions, as
// they are called by the command line processor.

#include "bdd_node.h"
#include <cstdio>
#include <climits>

class bdd_functs {

  // These are the two Hash tables, Unique and Op.
  Hash_Table uniqueTable;
  Hash_Table opTable;
    
  // This Hash table is used to keep track of the correspondence
  // between labels and indices.
  Hash_String_Table labelTable;
    
  // Keep track of a couple of special numbers and BDD nodes.
  int ZERO_INDEX;
  int ONE_INDEX;
  int CUR_INDEX;
  int TOTAL_SIZE;
  bdd_node *ZERO_NODE;
  bdd_node *ONE_NODE;

public:
  // This function initializes all of the above variables.
  bdd_functs() : ZERO_INDEX(INT_MAX),
		 ONE_INDEX(INT_MAX-1),
		 CUR_INDEX(0),
		 TOTAL_SIZE(0),
		 ZERO_NODE(Zero()),
		 ONE_NODE(One())
  { }
  // This method does the actual creation of a BDD node.
  bdd_node *createNode(int index, char *label, bdd_node *low,
		       bdd_node *high) {
    if(!(labelTable.lookup(index)))
      labelTable.insert(index, label);

    return new bdd_node(index,label,low,high);
  }

  // This function finds a given BDD node if it exists, and returns it
  // if it does. Otherwise it creates the node and returns it.
  bdd_node *findOrCreateNode(int index, char *label,
			     bdd_node *low, bdd_node *high) {
    if ((index == ONE_INDEX) || (index == ZERO_INDEX)) {
      char hashString[100];
      // This call treats pointers as integers; avert your eyes.
      sprintf(hashString, "%d,%d,%d", index,
	      reinterpret_cast<size_t>(low),
	      reinterpret_cast<size_t>(high));
      bdd_node *it = uniqueTable.lookup(hashString);
      if (it) return it;
      bdd_node *newNode = createNode(index, label, low, high);
      uniqueTable.insert(hashString, newNode);
      return newNode;
    } else {
      if (low == high) return low;
      char hashString[100];
      sprintf(hashString, "%d,%d,%d", index,
	      reinterpret_cast<size_t>(low),
	      reinterpret_cast<size_t>(high));
      bdd_node *it = uniqueTable.lookup(hashString);
      if (it) return it;
      bdd_node *newNode = createNode(index, label, low, high);
      ++TOTAL_SIZE;
      uniqueTable.insert(hashString, newNode);
      return newNode;
    }
  }
    
  // If the Zero Node exists, return it, if not, create it and insert 
  // it into the the appropriate hash table.
  bdd_node *Zero() {
    return findOrCreateNode(ZERO_INDEX, "ZERO", NULL, NULL);
  }
    
  // If the One Node exists, return it, if not, create it and insert 
  // it into the the appropriate hash table.
  bdd_node *One() {
    return findOrCreateNode(ONE_INDEX, "ONE", NULL, NULL);
  }

  inline bdd_node *getOne() { return ONE_NODE; }

  inline bdd_node *getZero() { return ZERO_NODE; }
    
  // The newVar needs to create the new bdd_node and initialize the
  // node using the CUR_INDEX. It also needs to increment CUR_INDEX
  // when it is done.
  bdd_node *newVar(char *label) {
    bdd_node *node = createNode(CUR_INDEX, label, NULL, NULL);
    CUR_INDEX++;
    return node; 
  }

  bdd_node *RESTRICT(bdd_node *F, int v, int k)
  {
    // Case 1: Restrict on root node variable
    if (F->getIndex() == v) {
      if (k == 1) {
        return (F->getHigh() != NULL) ? F->getHigh() : One();
      } 
      else {
        return (F->getLow() != NULL) ? F->getLow() : Zero();
      }
    }

    // Case 2: Restrict on variable less than root node variable
    return F;
  }
    
  // Here is the primary ITE function. It should work just as defined
  // in the lectures.
  bdd_node *ITE(bdd_node *I, bdd_node *T, bdd_node *E) {
    // cout << "calling ITE with " << endl;
    // cout << "\tI->getLabel()=" << I->getLabel() << endl;
    // cout << "\tT->getLabel()=" << T->getLabel() << endl;
    // cout << "\tE->getLabel()=" << E->getLabel() << endl;

    // Terminal cases
    if (I == getOne()) {
      // cout << "I == 1, returning T" << endl;
      return T;
    } else if (I == getZero()) {
      // cout << "I == 0, returning E" << endl;
      return E;
    } else if (T == One() && E == Zero()) {
      // cout << "T == 1 and E == 0, returning I" << endl;
      return I;
    } else if (T == E) {
      // cout << "T == E, returning E" << endl;
      return E;
    }
    
    // Recursive case
    else {

      // Generate hashString
      char hashString[100];
      sprintf(hashString, "%d,%d,%d", reinterpret_cast<size_t>(I),
        reinterpret_cast<size_t>(T),
        reinterpret_cast<size_t>(E));

      // Check if operation table has entry for (I, T, E)
      if (opTable.lookup(hashString) != NULL) {
        return opTable.lookup(hashString);
      }
     
      // Choose splitting variable: smallest var among roots of I, T, E
      int x = std::min(std::min(I->getIndex(), T->getIndex()), E->getIndex());
      // cout << "splitting var: " << x << endl;

      // Find positive and negative cofactors
      bdd_node *PosFactor = ITE(RESTRICT(I,x,1), RESTRICT(T,x,1), RESTRICT(E,x,1));
      bdd_node *NegFactor = ITE(RESTRICT(I,x,0), RESTRICT(T,x,0), RESTRICT(E,x,0));

      // Create new label
      char new_label[100];
      sprintf(new_label, "label%d", CUR_INDEX);
      // cout << "new_label: " << new_label << endl;

      // Create new node for splitting variable
      bdd_node* R = findOrCreateNode(x, new_label, NegFactor, PosFactor);

      // Insert into operation table
      sprintf(hashString, "%d,%d,%d", reinterpret_cast<size_t>(I),
        reinterpret_cast<size_t>(T),
        reinterpret_cast<size_t>(E));
      opTable.insert(hashString, R);

      return R;
    }
  }                   

  // The following are stubs for all the basic logic operations. They 
  // should all be defined in terms of the ITE method.
  bdd_node *NOT(bdd_node *f) {
    return ITE(f, Zero(), One());
  }
    
  bdd_node *AND(bdd_node *f, bdd_node *g) {
    return ITE(f, g, Zero());
  }
    
  bdd_node *OR(bdd_node *f, bdd_node *g) {
    return ITE(f, One(), g);
  }
    
  bdd_node *XOR(bdd_node *f, bdd_node *g) {
    return ITE(f, NOT(g), g);
  }
    
  // This method does the actual execution of the "bdd" command -
  // which prints out the BDD structure.
  void printBDD(bdd_node *f) {
    f->Print("\t");
    f->cleanMarks(); 
  }
    
  // This method finds the size of a given BDD structure.
  int sizeBDD(bdd_node *f) {
    int size = f->Size();
    f->cleanMarks();
    return size;
  }        

  // This method finds the total size of the workspace
  inline void total() {
    cout << " total size: " << TOTAL_SIZE << endl;
  }        
};
