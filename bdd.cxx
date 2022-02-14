#include <cstring>
#include "bdd_functs.h"

#define BUF_SIZE 1024
#define DATA_TOKEN       " {}(),=:;\t\n\0"
#define MAIN_ENTRY 0
#define SOURCE_FILE 1
#define MAX_LETTERS_PER_LINE  1024

Hash_Table funcTable;
bdd_functs bdd;


void helpCmd()
{
  char *arg;

  arg = strtok(NULL, DATA_TOKEN);

  if (arg==NULL) {
    cout<<"Available commands are:"<<endl;
    cout<<"quit, help, source, boolean, eval, bdd, size, total"<<endl;
  }
  else if (!strcmp(arg, "quit"))
    cout<<"The command to quit YBDD system."<<endl;
  else if (!strcmp(arg, "source"))
    cout<<"The command to open a command file and execute it."<<endl;
  else if (!strcmp(arg, "boolean"))
    cout<<"The command to set up boolean variables."<<endl;
  else if (!strcmp(arg, "size"))
    cout<<"The command to print the size (in bdd nodes) of a formula."<<endl;
  else if (!strcmp(arg, "eval"))
    cout<<"The command to create boolean expressions."<<endl;
  else if (!strcmp(arg, "bdd"))
    cout<<"The command to print the bdd structure."<<endl;
  else if (!strcmp(arg, "total"))
    cout<<"The command to print the total size in the workspace."<<endl;
}

void booleanCmd()
{
  char *varName;
  while((varName = strtok(NULL, DATA_TOKEN)) != NULL) {
    if (!(funcTable.lookup(varName))) {
      bdd_node *thisVar = bdd.newVar(varName);
      if (thisVar != NULL) {
	funcTable.insert(varName, thisVar);
	cout << "New variable: " << varName << endl;
      }
      else{
	cout << "Can't create variable: " << varName << endl;}
    }
    else if (!strcmp(varName, "!")||!strcmp(varName, "^")||
	    !strcmp(varName, "&")||!strcmp(varName, ":")||
	    !strcmp(varName, "+")) {
      cout << "Can't create variable: " << varName <<
	". That is a reserved character." << endl;
      }
    else{
      cout << "Can't create variable: " << varName <<
	". Variable already exists." << endl;}
  }
}

void evalCmd()
{
  char *funcName;
  if (!(funcName = strtok(NULL, DATA_TOKEN))) {
    cout << "Must specify a destination function name" <<
      " with the eval command." << endl;
    return;
  }

  if (funcTable.lookup(funcName)) {
    cout << "Warning: Reallocating function named:" << funcName <<
      endl;
    funcTable.remove(funcName);
  }

  char *varName;		//The first argument
  if (!(varName = strtok(NULL, DATA_TOKEN))) {
    cout << "Must specify an expression for the function" <<
      " with the eval command." << endl;
    return;
  }

  if (varName[0]=='!') {	//NOT operation
    char *argName;
    if (strlen(varName)>1) {
      argName = new char[strlen(varName)];
      for(unsigned i=1; i<strlen(varName); i++)
	argName[i-1] = varName[i];
      argName[strlen(varName)-1] = '\0';
    }
    else{
      argName = strtok(NULL, DATA_TOKEN);
      if (!argName) {
	cout << "Can't perform NOT, no variable supplied." << endl;
	return;
      }
    }
    bdd_node *ThisVar = funcTable.lookup(argName);
    if (ThisVar) {
      bdd_node *funcNode = bdd.NOT(ThisVar);
      funcTable.insert(funcName, funcNode);
    } else {
      cout << "Can't perform !" << argName << " because " <<
	argName << " doesn't exist." << endl;
      return;
    }
  } else {  //binary operators
    char *vars = new char[MAX_LETTERS_PER_LINE];
    strcpy(vars, varName);
    char *argName;
    while ((argName = strtok(NULL, DATA_TOKEN))) {
      strcat(vars, argName);
    }

    if ((!strchr(vars, '+')) && (!strchr(vars, '&')) &&
       (!strchr(vars, '^')) ) {
      cout << "Can't perform eval, you didn't provide" <<
	" an operator with args:" << vars << endl;
      delete[] vars;
      return;
    }

    char op;
    char *pos;
    if ((pos=strchr(vars, '+')))
      op = '+';
    else if ((pos=strchr(vars, '&')))
      op = '&';
    else if ((pos=strchr(vars, '^')))
      op = '^';
    unsigned len1 = pos-vars;
    if (len1==0 || len1==(strlen(vars)-1)) {
      cout << "Can't perform this eval because" <<
	" I can't find an operator." << endl;
      delete[] vars;
      return;
    }

    char *var1, *var2;
    var1 = new char[len1+1];
    for(unsigned i=0; i<len1; i++)
      *(var1+i) = *(vars+i);
    var1[len1] = '\0';
    var2 = new char[strlen(vars)-len1];
    strcpy(var2, pos+1);

    bdd_node *node1, *node2, *funcNode;
    if (!(node1=funcTable.lookup(var1))) {
      cout<<"Can't perform this eval because I don't"<<
	" know the variable or function: "<<var1<<endl;
      delete[] vars;
      delete[] var1;
      delete[] var2;
      return;
    }
    if (!(node2=funcTable.lookup(var2))) {
      cout<<"Can't perform this eval because I don't"<<
	" know the variable or function: "<<var2<<endl;
      delete[] vars;
      delete[] var1;
      delete[] var2;
      return;
    }

    if (op=='+')
      funcNode = bdd.OR(node1, node2);
    else if (op=='&')
      funcNode = bdd.AND(node1, node2);
    else
      funcNode = bdd.XOR(node1, node2);

    funcTable.insert(funcName, funcNode);
    delete[] vars;
    delete[] var1;
    delete[] var2;
  }
}

void bddCmd()
{
  char *varName;
  while ((varName = strtok(NULL, DATA_TOKEN))) {
    bdd_node *it;
    if (!(it=funcTable.lookup(varName))) {
      cout << "Can't print a BDD representation of " <<
	varName << " because it doesn't exist." << endl;
    } else {
      bdd.printBDD(it);
    }
  }
}

void sizeCmd()
{
  char *varName;
  while ((varName = strtok(NULL, DATA_TOKEN))) {
    bdd_node *it;
    if (!(it=funcTable.lookup(varName))) {
      cout << "Can't get size of " << varName <<
	" because it doesn't exist." << endl;
    } else {
      int size = bdd.sizeBDD(it);
      cout << "Size of " << varName << ": " << size << " Nodes"
	   << endl;
    }
  }
}

int cmdline(char *fileName, int execType)
{
  FILE *fp;
  if (fileName==NULL)
    fp = stdin;
  else if (!(fp = fopen(fileName, "r"))) {
    if (execType==MAIN_ENTRY) {
      cout << "Cannot open file and use stdin as input" << endl;
      fp = stdin;
    } else {
      cout << "Cannot find file to source" << endl;
      return 1;
    }
  }

  char data_buf[BUF_SIZE];
  char *curCmd, *arg;
  if (execType == MAIN_ENTRY) cout << "YBDD>";
  while (fgets(data_buf, BUF_SIZE, fp)) {
    curCmd = strtok(data_buf, DATA_TOKEN);
    if (curCmd == NULL) continue;
    if (curCmd[0] == '#') continue;

    if (!strcmp(curCmd, "quit")) {
      fclose(fp);
      return 0;
    } else if (!strcmp(curCmd, "boolean")) {
      booleanCmd();
    } else if (!strcmp(curCmd, "eval")) {
      evalCmd();
    } else if (!strcmp(curCmd, "help")) {
      helpCmd();
    } else if (!strcmp(curCmd, "source")) {
      if (!(arg = strtok(NULL, DATA_TOKEN)))
	cout<<"Need a filename to open"<<endl;
      else{
	cmdline(arg, SOURCE_FILE);
	cout<<endl;
      }
    } else if (!strcmp(curCmd, "size")) {
      sizeCmd();
    } else if (!strcmp(curCmd, "bdd")) {
      bddCmd();
    } else if (!strcmp(curCmd, "total")) {
      bdd.total();
    }

    /*Put code for new command here
      else if (!strcmp(curCmd, "user_command")) {

      }*/

    else
      cout << "I'm sorry, I don't know the command: " << curCmd
	   << ".  So I can't help." << endl;

    if (execType==MAIN_ENTRY) cout << "YBDD>";
  }

  return 1; // Should never reach this line
}

int main(int argc, char **argv)
{
  funcTable.insert("1", bdd.getOne());
  funcTable.insert("0", bdd.getZero());

  if (argc > 2)
    cout << "Usage: cmdline (filename)" << endl;
  else return cmdline(argc == 2 ? argv[1] : NULL, MAIN_ENTRY);

  return 1;
}
