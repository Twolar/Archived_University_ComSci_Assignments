//159201 assignment 3 Taylor Bennett 16105740
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <locale>
#include <sstream>
#include <string>
using namespace std;

/* implement your Queue class here */
struct Node {
  int data;
  Node *next;
};

/* implement your Queue class here */
class Queue {
private:
  Node *front, *rear;
  int counter;
public:
  Queue();
  ~Queue();
  void Join(int newthing);
  void Leave();
  int Front();
  bool isEmpty();
  void PrintAllElements(); // CHECK IF THIS FUNCTION IS NEEDED AT THE END!!!!!!
  int Length();
};


// CONSTRUCTOR
Queue::Queue(){
  front = NULL;
  rear = NULL;
}

// DESTRUCTOR
Queue::~Queue(){}


// QUEUE JOIN (PLACE NEWTHING AT THE REAR OF THE QUEUE/LL)
void Queue::Join(int newthing){
  Node *temp;
  temp = new Node;
  temp->data = newthing;
  temp->next = NULL;
  if (rear != NULL){
    rear->next = temp;
  }
  rear = temp;
  if (front == NULL){
    front = temp;
  }
  counter++;
}


// QUEUE LEAVE (REMOVE THE FRONT ITEM FROM THE QUEUE/LL)
void Queue::Leave(){
  Node *temp;
  if (front == NULL){
    return;
  }
  temp = front;
  front = front->next;
  if (front == NULL){
    rear = NULL;
  }
  delete temp;
  if (counter > 0) {counter--;}
}


// FRONT (RETURN VALUE AT THE FRONT OF THE QUEUE/LL)
int Queue::Front(){
  if (front != NULL){
    return front->data;
  }
  return 0;
}


// isEmpty (RETURN TRUE IF THE QUEUE/LL IS EMPTY)
bool Queue::isEmpty(){
  if (front == NULL){
    return true;
  }
  return false;
}


// PRINT ALL ELEMENTS
void Queue::PrintAllElements(){
  cout << "Linked-List: ";
  Node *current;
  current = front;
  while(current != NULL){
    cout << current->data << " ";
    current = current->next;
  }
  cout << endl;
}


int Queue::Length(){
  return counter;
}
////////////////////////////////////////////////////////////////////////

#define TIMEDELAY 3 //DO NOT CHANGE THIS VALUE!!!
#define N 128 //DO NOT CHANGE THIS VALUE!!!
int OutQueues_current[N];
int Congestion_Size[N];

/*include your array of queues declarations somewhere here, for example: */
Queue InputQueues[N];
Queue OutputQueues[N];


void init_simulation(){
  for(int a=0;a<N;a++){
    OutQueues_current[a]=0;
    Congestion_Size[a]=0;
  }
}

int sum_elements_array(int array[]){
  int sum=0;
  for(int a=0;a<N;a++){
    sum=sum+array[a];
  }
  return sum;
}

int number_of_ports=0;

int main( int argc, char** argv ){//get arguments from command line, the name of the simulation text file
  //read the file, print the input ports contents
  int portnumber=0;
  int destination=0;
  //char oper;
  string expression;
  string geninput;
  ifstream input_file;
  if(argc!=2) {cout<< "Type a file name. " << endl << argv[1] << endl; exit(0);}
  input_file.open(argv[1]);
  if(input_file.is_open()==false) {cout << "Could not read file: " << endl << argv[1] << endl; exit(0);}
  string token;
  while(!input_file.eof()){
     getline(input_file,expression);
     stringstream line(expression);
     if(input_file.eof()) break;
     if(expression[0] =='#') continue;//jump line, this is a line of comments
     if(expression[0] =='P') {
       getline(line, token,' ');
       getline(line, token,' ');
       number_of_ports=atoi(token.c_str());
       //cout << "Found the number of ports to be " << number_of_ports << endl;
       continue;//got the number of ports
     }
     
     portnumber++;//get data for the next port
     
     
     //cout << "Port " << portnumber << ": " << endl;//DEBUG, comment for final version
     
     while(getline(line, token,' ')){
       int destination;
       //destination = stoi(token);//only works with option -std=c++11
       destination = atoi(token.c_str());//use this one if your compiler is not C++11
       if (destination < 0 || destination > number_of_ports || number_of_ports < portnumber) {cout << "ERROR in the format of the text file" << endl; exit(0);}
    //
    // include the input queue code here
       InputQueues[portnumber-1].Join(destination);
    //
       //cout << "destination " << destination << endl;//DEBUG, comment for final version
     }
     int sumofinputpackets=0;
     sumofinputpackets= sumofinputpackets + InputQueues[portnumber-1].Length();
     //cout<< "Input packets at input queue for port "<< portnumber << " = " << sumofinputpackets << endl;//DEBUG, comment for final version
     //cout<< "if this value is still zero, it is because you did not implement the queues yet..." << endl;//DEBUG, comment for final version
  }
  //now carry on with the simulation 
  //cout << endl << "Start of the simulation, press a key " << endl << endl;//DEBUG, comment for final version
  //getline(cin, geninput);// pause //DEBUG, comment for final version
  init_simulation();
  unsigned long int clock=0;
  unsigned long int currentsum=99999999;//sum_elements_array(OutQueues_current);
  portnumber=0;
  while(currentsum>0){
    //cout << "Input portnumber " << portnumber+1 << endl;//DEBUG, comment for final version
    /*if(QUEUE[portnumber]){

      decide what to do with the queues here
      
    }*/
    /*
     //DEBUG
    cout << "Clock: " << clock << endl;

 // DEBUG IS THE INPUT LIST EMPTY?
    if (InputQueues[portnumber].isEmpty() == true){
      cout << "INPUT LL QUEUE @ Port (" << portnumber+1 << "): IS EMPTY" << endl;
    }
    // DEBUG PRINT LL INPUT QUEUE CONTENTS
    cout << "INPUT LL QUEUE @ Port (" << portnumber+1 << "): ";
    InputQueues[portnumber].PrintAllElements();
    cout << endl;
    */
    
    if (InputQueues[portnumber].isEmpty() == false){
      OutputQueues[InputQueues[portnumber].Front()-1].Join(InputQueues[portnumber].Front());
      InputQueues[portnumber].Leave(); 
    }
    

    /*
    // DEBUG IS THE OUTPUT LIST EMPTY?
    if (OutputQueues[portnumber].isEmpty() == true){
      cout << "OUTPUT LL QUEUE @ Port (" << portnumber+1 << "): IS EMPTY" << endl;
    }

    // DEBUG PRINT LL OUTPUT QUEUE CONTENTS
    cout << "OUTPUT LL QUEUE @ Port (" << portnumber+1 << "): ";
    OutputQueues[portnumber].PrintAllElements();
    cout << endl;
    */

    portnumber++;
    if(portnumber > (number_of_ports-1)) portnumber=0;

    clock++;
    
    if(clock % (TIMEDELAY*number_of_ports) == 0 && clock!=0) { //DO NOT MODIFY THIS LINE!
      //cout << "Packets can leave the output queues at " << clock << " microsec " << endl;//DEBUG, comment for final version
      for(int a=0;a<number_of_ports;a++){
        OutQueues_current[a] = OutputQueues[a].Length();
        OutputQueues[a].Leave();
       
	//Delete 1 packet from each queue and count number of packets again
	     
      }
      currentsum = sum_elements_array(OutQueues_current);
    }
    //
    // include the queue updates for the simulation
    //
    //
    //compute the current state of the output queues
    //cout << "Current sum: " << currentsum << endl;//DEBUG, comment for final version
    if(currentsum > sum_elements_array(Congestion_Size)){
      for(int a=0;a<number_of_ports;a++){
	     Congestion_Size[a]=OutQueues_current[a];
      }
    }
  }
  //FINAL PRINTOUT, remember to comment out all the other debugging printouts above 
  for(int a=0;a<number_of_ports;a++){
    cout << "output port " << a+1 << ": " << Congestion_Size[a] << " packets" << endl; 
  }
}
