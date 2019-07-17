//159201 assignment 6 Taylor Bennett 16105740
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <locale>
#include <sstream>
#include <string>
#include <vector>
#include <limits.h>
using namespace std;

/* Implement the Graph class  */

struct Node {
  char key;
  int distance;
  Node *next;
};

struct GraphNode {
  char key;
  Node *listpointer;
};

void AddNodetoFront(Node*& listpointer, char newkey, int newdistance){
  Node *temp;
  temp = new Node;
  temp->key = newkey;
  temp->distance=newdistance;
  temp->next = listpointer;
  listpointer = temp;
}

void PrintLLnodes (Node*& listpointer){
  Node *temp;
  temp = listpointer;
  while (temp!=NULL){
    cout << "to node " << temp->key << " dist: " << temp->distance << endl;
    temp = temp->next;
  }
}

class Graph {
private:
  vector<GraphNode> adjlist;
public:
  Graph(){};
  ~Graph(){};
  void AddNewGraphNode(char newgraphnode);
  void AddNewEdgeBetweenGraphNodes(char A, char B, int distance);
  void PrintAllGraphNodesWithCosts();
  void Dijkstra();
  int returnIndexOfKey(char X);
  int calculateCost(int current, char v);
};

void Graph::AddNewGraphNode(char newgraphnode){
  GraphNode temp;
  temp.key = newgraphnode;
  temp.listpointer = NULL;
  adjlist.push_back(temp);
}

void Graph::AddNewEdgeBetweenGraphNodes(char A, char B, int distance){
  int a;
  for (a=0; adjlist.size(); a++){
    if (A == adjlist[a].key) { break; }
  }
  AddNodetoFront(adjlist[a].listpointer, B, distance);
}

void Graph::PrintAllGraphNodesWithCosts(){
  for (int a=0; a<adjlist.size(); a++){
    cout << "From Node " << adjlist[a].key << ":" << endl;
    PrintLLnodes(adjlist[a].listpointer);
  }
}

int Graph::returnIndexOfKey(char X){
  for (int i = 0; i < adjlist.size(); i++){
    if (X == adjlist[i].key){
      return i;
    }
  }
  return -1;
}

int Graph::calculateCost(int current, char v){
  int finalCost = 32767;
  Node *temp;

  temp = adjlist[current].listpointer;

  while (temp != NULL){
    if (temp->key == v){
      return temp->distance;
    }
    temp = temp->next;
  }
  return finalCost;
}

/*
* Dijkstra ALGORITHM
* int_max indicates infinity in this case as we are only working with positive integers
*/
void Graph::Dijkstra(){
  int current;
  //Node *temp;
  vector<int> d;
  vector<char> s;
  int d_curr = INT_MAX;
  int minimumDindex = INT_MAX;

  //cout << "Source Node: " << returnIndexOfKey('A') << endl;

  d.push_back(0);
  s.push_back('p');
  current = returnIndexOfKey('A');
  //temp = adjlist[returnIndexOfKey('A')].listpointer;

  for (int i=0; i<adjlist.size(); i++){ 
    if(adjlist[i].key != adjlist[0].key){
      d.push_back(INT_MAX);
      s.push_back('t');
    }
  }
  
  
  bool isTemp = true;
  while (isTemp) {
    isTemp = false;
    /*
    cout << "S = ";
    for (int p = 0; p < s.size(); p++){
    cout  << s[p] << ", ";
    }
    cout << endl;

    cout << "D = " ;
    for (int l = 0; l < d.size(); l++){
    cout  << d[l] << ", ";
    }
    cout << endl;
*/
    // cout << "d[v] = ";
    
    for(int v = 0; v < adjlist.size(); v++){
      if(s[v] == 'p') {continue;} // if state is permanent then skip
      // if (calculateCost(current, adjlist[v].key) == -1) { continue; }
      d[v] = min(d[v], d[current] + calculateCost(current, adjlist[v].key)); // calculate path cost

      // find shortest path and store the index
      //cout << v << " - " << d[v] << ", ";
      if(d[v] < d_curr) { d_curr = d[v]; minimumDindex = v; }
    }
    // cout << endl << endl;


    d_curr = INT_MAX; // reset d_curr to be used next time

    if (s[minimumDindex] == 't') {
      current = minimumDindex;
      s[current] = 'p';
    
    }
    for (int x=0; x<s.size(); x++){
      if(s[x] == 't') { isTemp = true; break;}
    }
  }


/*
  cout << "S = ";
  for (int p = 0; p < s.size(); p++){
    cout  << s[p] << ", ";
  }
  cout << endl;

  cout << "D = " ;
  for (int l = 0; l < d.size(); l++){
    cout  << d[l] << ", ";
  }
  cout << endl;
*/


  // PRINT OUT OF FINAL RESULTS FROM A TO ?
  for(int z = 0; z<d.size(); z++){
    if(z == returnIndexOfKey('A')) { 
      continue;
    } else {
      cout << "From A to " << adjlist[z].key << ":" << d[z] << endl;
    }
  }
}

/* declare a new Graph */
Graph G;

int main( int argc, char** argv ){//get filename from arguments
  //read an input text file
  string expression;
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
     char node_name;
     if(expression[0] =='N' && expression[1]=='o') {//read a Node name, single character
       getline(line, token,' ');
       getline(line, token,' ');
       node_name=(token.c_str())[0];
       //cout << "Node " << node_name << endl;//Comment this out on the final version
       /* Insert nodes into the graph */
       G.AddNewGraphNode(node_name);
       /* */
     }
     else{
       char nodefrom;
       getline(line, token,' ');
       nodefrom=(token.c_str())[0];
       //cout << "From " << nodefrom;//Comment this out on the final version
       char nodeto;
       getline(line, token,' ');
       nodeto=(token.c_str())[0];
       //cout << " to " << nodeto; //Comment this out on the final version
       int cost;
       getline(line, token,' ');
       cost = stoi(token);//only works with option -std=c++11
       cost = atoi(token.c_str());//use this one if your compiler is not C++11
       //cout << " costs " << cost << endl;//Comment this out on the final version
       /* Insert these edges into the graph */
       G.AddNewEdgeBetweenGraphNodes(nodefrom, nodeto, cost);
       /*  */
     }
  }
  /* After loading the graph from the text file in the while loop above, run your implementation of Dijkstra here*/
  /* Dijkstra can be implemented either as a function or as a method of the class Graph*/

 
  /* call Dijkstra  */

  G.Dijkstra();
  /* Print answer in the required format */


  
}
