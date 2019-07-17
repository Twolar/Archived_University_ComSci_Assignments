//159201 assignment 1 Taylor Bennett 16105740
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;

struct Node {  //declaration
	int row;
	int column;
	int value;
	Node *next;
};
Node *A, *B, *C;  //declaration


void AddNode(Node * & listpointer, int mRow, int mCol, int mValue);
void PrintLL(Node *listpointer);
void PrintMatrix(Node *listpointer, int MatrixSize);
int getMatrixSize(Node* &a, char *file_name);
int Search(int i, int j, Node *listpointer);
void AddMatrices(Node *A, Node *B, Node *&C, int mSize);


void read_matrix(Node* &a, char *file_name){
    //reads a matrix from a file
    int col = 0, row = 0, value = 0;  
    ifstream input;
    input.open(file_name);
    if(!input.good()){
        cout << "Cannot open file " << file_name << endl;
        exit(0);
    }
    int c;
    string line;
    //reads the first line to get dimensions
    if(input.good()){
	    getline(input,line);
	    stringstream sline(line);
	    sline >> row >> col;
    }
    //read matrix 
    for(int i = 0; i < row; ++i){
        if(input.good()) {
		getline(input,line);
		stringstream sline(line);
        	for(int j = 0; j < col; ++j){
		sline >> value;
	        if(value == 0) continue;
	    //
            //Include your own add_node(a, i, j, value); function here
            AddNode(a, i, j, value);
          }
        }
    }
    input.close();
}



void AddNode(Node * & listpointer, int mRow, int mCol, int mValue) {
// add a new node to the TAIL of the list
  Node *current;
  current=listpointer;
  if(current!=NULL){
    while (current->next!=NULL){
      current=current->next;
    }
  }// now current points to the last element
  Node *temp;
  temp = new Node;
  temp->row = mRow;
  temp->column = mCol;
  temp->value = mValue;
  temp->next = NULL;
  if(current!=NULL) current->next = temp;
  else listpointer=temp;
}



void PrintLL(Node *listpointer) {
  Node *current;
  current = listpointer;
  if (current == NULL) { /** cout << "the list is empty" << endl;**/ cout << endl; return; }
  while (current != NULL){
    if (current->value > 0){
      cout << current->value << " ";
    }
    //cout << "Row: " << current->row << " Col: " << current->column << " Value: " << current->value << endl;
    current = current->next;
  }
  cout << endl;
}



void PrintMatrix(Node *listpointer, int MatrixSize){
  int count = 0;
  Node *current;
  current = listpointer;
  for(int i=0; i<MatrixSize; ++i){
    for(int j=0; j<MatrixSize; ++j){
      if (count == MatrixSize) { 
        cout << endl;
        count = 0;
      }
      if (Search(i, j, current) > 0){
        cout << Search(i, j, current) << " ";
      } else {
        cout << "0 ";
      }
      count++;
    }
  } 
}


int getMatrixSize(Node* &a, char *file_name){
  int row = 0, col = 0;
    ifstream input;
    input.open(file_name);
    if(!input.good()){
        cout << "Cannot open file in getMatrixSize function" << file_name << endl;
        exit(0);
    }
    string line;
    //reads the first line to get dimensions
    if(input.good()){
      getline(input,line);
      stringstream sline(line);
      sline >> row;
    }
    return row;
    input.close();
}



int Search(int i, int j, Node *listpointer){
  Node *current;
  current = listpointer;
  while(true){
    if (current == NULL){ break; }
    if ((current->row == i) && (current->column == j)){
      return current->value;
    }
    current = current->next;
  }
  return 0;
}



void AddMatrices(Node *A, Node *B, Node *&C, int MatrixSize){
  int result = 0;
  for(int i=0; i<MatrixSize; ++i){
    for(int j=0; j<MatrixSize; ++j){
      result = Search(i, j, A) + Search(i, j, B);
      if (result == 0) {
        continue;
      } else {
        AddNode(C, i, j, result);
      }
    }
  }
}



int main( int argc, char** argv ) {
	if(argc!=3) {printf("needs two matrices \n"); exit(0);}
    read_matrix(A, argv[1]);
	read_matrix(B, argv[2]);

  int MatrixSize = 0;
  MatrixSize = getMatrixSize(A, argv[1]);

  cout << "Matrix 1: ";
  PrintLL(A);
  PrintMatrix(A, MatrixSize);
  cout << endl;

  cout << "Matrix 2: ";
  PrintLL(B);
  PrintMatrix(B, MatrixSize);
  cout << endl;

  AddMatrices(A, B, C, MatrixSize);

  cout << "Matrix Result: ";
  PrintLL(C);
  PrintMatrix(C, MatrixSize);
}