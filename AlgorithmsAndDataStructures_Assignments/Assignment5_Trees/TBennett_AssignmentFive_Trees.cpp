//159201 assignment 5 Taylor Bennett 16105740
#include <stdio.h>
#include <stdlib.h>
#include <stack>
#include <iostream>
#include <fstream>

using namespace std;

/* implement the Tree class here, you can copy the one from week 4 slides */
/* the contents of the Tree nodes can be of type char, so they can store operators (+, *, / and -) as well as digits (0,1,2...9) */
/* all the RPN*.txt test files contain numbers with single digits */
class Tree {
private:
  char data;
  Tree *leftptr, *rightptr;
 
public:
  Tree(char newthing, Tree* L, Tree* R);  // constructor with paramters
  ~Tree() { }
  char RootData() { return data; }  // inline functions
  Tree* Left() { return leftptr; }
  Tree* Right() { return rightptr; }
};

Tree::Tree(char newthing, Tree* L, Tree* R) {
  data = newthing;
  leftptr = L;
  rightptr = R;
}


/* implement your stack here*/
/* remember that the content of the stack is of type Tree */
/*
class Stack {
private:
	Tree *treepointer;
public:
	Stack();
	~Stack();
	void Push(char newthing);
	void Pop();
	char Top();
	bool isEmpty();
};


// constructor
Stack::Stack(){
	treepointer = NULL;
}

// destructor
Stack::~Stack(){}

// add new tree to the top of the stack
void Stack::Push(char newthing){
	Tree *temp;
	temp = new Tree;

}

// remove top tree from the stack
void Stack::Pop(){}

// return the pointer/value of the top tree in the stack (USE isEmpty BEFORE calling this method)
char Stack::Top(){}

// returns true if the stack is empty
bool Stack::isEmpty(){}

*/ // THIS IS COMMENTED OUT BECAUSE USING STD::STACK INSTEAD

stack<Tree*> S; //Declare your stack of Tree* here 

Tree *T1, *T2, *T;

/* implement your recursive funtions for traversals */

void InOrder(Tree *T){
	if(T==NULL) { return; }

	if(T->Left()!=NULL) { cout << "("; }
	InOrder(T->Left());
	cout << T->RootData();
	InOrder(T->Right());
	if(T->Right()!=NULL) { cout << ")"; }

}

void PostOrder(Tree *T){
	if(T==NULL) { return; }

	PostOrder(T->Left()); 
	PostOrder(T->Right());
	cout << T->RootData();
	cout << " ";
}


int main( int argc, char** argv ){//get filename from arguments
	char digit;
	char oper;
	char expression[100];
	ifstream input_file;
	if(argc==2) input_file.open(argv[1]);
	else {printf("The program needs a filename as argument \n");exit(0);}
	/* both operator and digits are of type char */
	while(input_file >> expression){
		if(isdigit(expression[0])){
			sscanf(expression,"%c",&digit);
			//printf("reading a number: %c \n",digit);
			//modify here to deal with the Stack
			S.push(new Tree(digit, NULL, NULL));
		}
		else {
			sscanf(expression,"%c",&oper);
			//printf("reading an operator: %c \n",oper);
			//modify here to deal with the Stack
			T1 = S.top(); S.pop();
			T2 = S.top(); S.pop();
			S.push(new Tree(oper, T2, T1));
		}
	}

	T = S.top();
	
	//Now we can traverse the tree in a certain way and print the expression
	
	//in-order with parenthesis
	cout << "In-fix:" << endl;
	InOrder(T);
	cout << endl;
	
	//post-order
	cout << "Post-fix:" << endl;
	PostOrder(T);
	cout << endl;

}

