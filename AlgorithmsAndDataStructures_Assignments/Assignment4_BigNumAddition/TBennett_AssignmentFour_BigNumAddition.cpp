//159201 assignment 4 Taylor Bennett 16105740
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <locale>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

using namespace std;

/////// PART A

//copy the template class List here
template <class T>
class List {
private:
  struct Node {
    T data;
    Node *next;
  };
  Node *front, *current;

public:
  List();
  ~List();
  void AddtoFront(T newthing);
  bool FirstItem(T & item);
  bool NextItem(T & item);
};

template <class T>
List<T>::List() {
  front = NULL;  current = NULL;
}

template <class T>
List<T>::~List() {

}

template <class T>
void List<T>::AddtoFront(T newthing) {
  Node *temp;
  temp = new Node;
  temp->data = newthing;
  temp->next = front;
  front = temp;
}

template <class T>
bool List<T>::FirstItem(T & item) {
  if (front == NULL) { return false; }
  item = front->data;
  current = front;
  return true;
}

template <class T>
bool List<T>::NextItem(T & item) {
  if (current->next == NULL) { return false;}
  current = current->next;
  if (current == NULL) { return false; }
  item = current->data;
  return true;
}


/////// PART B
class BigNumber {
private:
//complete here...
//include here a List of integers, or shorts etc
  List<int> listofdigits;

public:
//complete here...
//what methods do you need?  
  BigNumber();
  ~BigNumber();
  void ReadFromString (string decstring);
  void PrintBigNumber ();
  void ReverseBigNumber(BigNumber rBN);
  void AddBigNumbers(BigNumber B1,BigNumber B2);
};

BigNumber::BigNumber(){
// anything here?
}

BigNumber::~BigNumber(){
//you can keep that empty
}

void BigNumber::ReadFromString (string decstring) {
  //cout<< "testing string passing: "<< decstring << endl;
	//read a string, adding a new node per digit of the decimal string
	// To translate 'digits' to integers: myinteger=decstring[index]-48
	
  for(int i = 0; i < decstring.size(); i++) {
    if(decstring[i]!='\n' && decstring[i]!='\r'){
      int temp=decstring[i]-48;
      //cout << "Digit " << i << " is " << temp << endl;  //Comment this out for your final version
      //You need to use the AddtoFront() 
      listofdigits.AddtoFront(temp);
    }
  }
}

void BigNumber::PrintBigNumber () {
//complete here, print the list (i.e., use FirstItem() and NextItem() )
//remember that the print out may be inverted, depending on the order you enter the 'digits'
  bool ok;
  int x;

  ok = listofdigits.FirstItem(x);
  while(ok){
    cout << x;
    ok = listofdigits.NextItem(x);
  }
  cout << endl;
}

void BigNumber::ReverseBigNumber(BigNumber rBN){
  bool ok;
  int x;
  
  ok = rBN.listofdigits.FirstItem(x);
  while (ok){
    listofdigits.AddtoFront(x);
    ok = rBN.listofdigits.NextItem(x);
  }
}

void BigNumber::AddBigNumbers(BigNumber B1,BigNumber B2){
//complete here.
//use FirstItem(), NextItem() and AddNode()
//to add two big numbers, what do you have to do? Be careful about the carry
//Remember to add the last carry, the resulting number can have one more digit than B1 or B2
  int digit1 = 0, digit2 = 0, result = 0, carry = 0;

  bool ok1 = B1.listofdigits.FirstItem(digit1);
  bool ok2 = B2.listofdigits.FirstItem(digit2);
  
  while ((ok1) || (ok2)){


    if (ok1 != true) { digit1 = 0;}
    if (ok2 != true) { digit2 = 0;}

    result = digit1 + digit2;
    result = result + carry;

    if (result > (10 - 1)) { 
      result = result - 10;
      carry = 1;
    } else { carry = 0; }

    ok1 = B1.listofdigits.NextItem(digit1);
    ok2 = B2.listofdigits.NextItem(digit2);

    listofdigits.AddtoFront(result);

  }
  // ADD FINAL CARRY IF THERE IS ONE
  if (carry == 1){
    listofdigits.AddtoFront(1);
  }
}

/////// PART C

BigNumber B1,B2,RES,temp1,temp2;

int main (int argc, char ** argv) {
  string numberstring;
  int stringseq=0;
  ifstream input_file;
  if(argc==2) input_file.open(argv[1]);
  else { cout<< "cannot read the file " << argv[1] << endl; exit(0);}
  while(!input_file.eof()){
    getline(input_file,numberstring);
    if(!input_file.eof()){
      //cout << "reading a big number from file:" << numberstring << endl;//Comment this out for your final version
      if(stringseq==0){
        B1.ReadFromString(numberstring);
        stringseq=1;
      }
      else B2.ReadFromString(numberstring);
    }
  }

  //compute the addition
  RES.AddBigNumbers(B1,B2);
  
  // REVERSE B1
  temp1.ReverseBigNumber(B1);
  B1 = temp1;

  // REVERSE B2
  temp2.ReverseBigNumber(B2);
  B2 = temp2;


  //print
  B1.PrintBigNumber();//PROBLEM: Is the printing inverted? How can you solve this problem? Before or after using AddBigNumbers()?
  cout << "+" << endl;
  B2.PrintBigNumber();
  cout << "=" << endl;
//print the result
  RES.PrintBigNumber();

  
}
