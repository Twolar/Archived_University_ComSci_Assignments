//159201 assignment 2 Taylor Bennett 16105740
#include <stdio.h>
#include <stdlib.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <locale>
#include <sstream>

using namespace std;

/* implement your stack here */
struct Node {
	int data;
	Node *next;
};


class Stack {
private:
	Node *listpointer;
public:
	Stack();
	~Stack();
	void Push(int newthing);
	void Pop();
	int Top();
	bool IsEmpty();
};


// constructor
Stack::Stack() {
	listpointer = NULL;
}


// destructor
Stack::~Stack(){}


// place new item at the top of the stack
void Stack::Push(int newthing){
	Node *temp;
	temp = new Node;
	temp->data = newthing;
	temp->next = listpointer;
	listpointer = temp;
}


// remove top item from the stack
void Stack::Pop(){
	Node *p;
	p = listpointer;
	if(listpointer != NULL){
		listpointer = listpointer->next;
		delete p;
	}
}


// return value at the top of the stack
int Stack::Top(){
	return listpointer->data;
}


// returns true if the stack is empty
bool Stack::IsEmpty(){
	if(listpointer == NULL){
		return true;
	}
	return false;
}



Stack S;

int main( int argc, char** argv ){//get arguments from command line, i.e., yourexec filename
	int result, op1,op2,number,i;
	char oper;
	string expression;
	ifstream input_file;
	if(argc!=2) {
	  cout << "needs a filename as argument  " << endl;
	  exit(0);
	}
	input_file.open(argv[1]);
	if(!input_file.good()){
	  cout << "cannot read file " << argv[1] << endl; 
	  exit(0);
	}
	while(!input_file.eof()){
		getline(input_file,expression);
		if(isdigit(expression[0])){
		  stringstream line(expression);
		  line >> number;
		  cout << "reading number " << number << endl;
		  //modify here to deal with the Stack
		  //PUSH number
		  S.Push(number);
		}
		else {
		  if(expression[0]=='+' || expression[0]=='-'|| expression[0]=='/'||expression[0]=='*') {
		    stringstream line(expression);
		    line >> oper;
		    cout << "reading operator " << oper << endl;
		    //op2 = TOP
		    //POP
		    if(S.IsEmpty() == false){
		    	op2 = S.Top();
		    	S.Pop();
		    } else {
		    	cout << "too many operators" << endl;
		    	exit(12);
		    }
		    
		    //op1 = TOP 
		    //POP
		    if(S.IsEmpty() == false){
		    	op1 = S.Top();
		    	S.Pop();
		    } else {
		    	cout << "too many operators" << endl;
		    	exit(12);
		    }
		    
		    //compute result
		    //PUSH result
		    
		    if(oper=='+') { result=op2 + op1;}
		    if(oper=='*') { result=op2 * op1;}
		    if(oper=='-') { result=op1 - op2;}
		    if(oper=='/') { result=op1 / op2;}
		    //cout << "Pushing result: " << result << endl;
		    S.Push(result);
		  }
		}
	}


	int temp = 0;
	if(S.IsEmpty() == false){
		temp = S.Top();
	} else {
		cout << "something went wrong" << endl;
		exit (13);
	}

	S.Pop();

	if(S.IsEmpty() == true){
		cout << "The result is " << temp << endl;
	} else {
		cout << "too many numbers" << endl;
	}
}
