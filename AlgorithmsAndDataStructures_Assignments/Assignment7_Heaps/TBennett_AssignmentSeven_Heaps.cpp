//159201 assignment 7 Taylor Bennett 16105740
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>

using namespace std;

vector<int> vector_random;
vector<int> vector_reversed;
vector<int> vector_sorted;

/*  Implement a Heap class here (from the slides)  */
class Heap {
private:
	vector<int> data;
	int last;
	int DeleteRootComparisons;

public:
	Heap(){
		last = -1;
		DeleteRootComparisons = 0;
		for(int i=0; i<data.size(); i++){
			data[i] = 0;
		}
	}
	~Heap() {};
	int InsertHeap(int newthing);
	int DeleteRoot();
	void PrintHeap();
	int getDeleteRootComparisons(){ return DeleteRootComparisons;}
};


int Heap::InsertHeap(int newthing){
	data.push_back(newthing);
	last = last+1;
	int numOfComparisons = 0;


	//cout << endl;
	//cout << "New thing = " << newthing << endl;
	//cout << "Last = " << last << endl;

	int temp;
	int swindex = last;
	int parindex = 0;
	bool swapping = true;

	//cout << " while start" << endl;
	while(swapping){
		swapping = false;

		if(swindex % 2 == 0){ parindex = swindex/2-1; } 
		else { parindex = swindex/2; }

		
		if (parindex >= 0){
			numOfComparisons++;
			if(data[swindex] > data[parindex]){
				
				swap(data[swindex], data[parindex]);
				swapping = true;
				swindex = parindex;
			}
		}
	}
	//cout << "while finish" << endl << endl;
	return numOfComparisons;
}

int Heap::DeleteRoot(){
	int a = 0;

	int numOfComparisons = 0;

	if (last < 0) { cout << "The heap is already empty" << endl; exit(123);}
	//for(a = 0; a<last; a++){ if (data[a] == valueToDelete) { break;}}
	//cout << "Index to delete is " << a << " for item: " << valueToDelete << endl; // COMMENT OUT FINAL VERSION YO
	//cout << "data[" << a << "] = " << data[a] << endl;
	//cout << "data[" << a+1 << "] = " << data[a+1] << endl;
	int temp = data[a];

	data[a] = data[last]; // Swap node with last and delete last;
	data.pop_back();
	last = last - 1;



	// HEAP FIX (RE-SORT)
	int parindex = a;
	int leftindex = parindex*2+1; // LEFT Child (2i + 1)
	int rightindex = parindex*2+2; // RIGHT Child (2i + 2)

	// CALCULATE COMPARISONS BASED OF HOW MANY CHILDREN NODE HAS
	if ((leftindex <= last) && (rightindex <= last)) {
		DeleteRootComparisons++;
		DeleteRootComparisons++;
	}
	if ((leftindex <= last)&&(rightindex>last)){
		DeleteRootComparisons++;
	}

	bool swapping = true;
	while ((data[parindex] < data[leftindex] || data[parindex] < data[rightindex]) && (swapping == true)){
		swapping = false;

		
		if (data[rightindex] < data[leftindex]){ // Follow LEFT
			// COMMENT OUT cout IN FINAL VERSION
			//cout << "rightIDX > leftIDX Swap data[" << leftindex << "] = " << data[leftindex] << " with data[" << parindex << "] = " << data[parindex] << endl;
			swap(data[leftindex], data[parindex]);
			parindex = leftindex;
			swapping = true;
		} else { // ELSE Follow RIGHT
			// COMMENT OUT cout in FINAL VERSION
			if (last > 0){
				//cout << "leftIDX > rightIDX Swap data[" << rightindex << "] = " << data[rightindex] << " with data[" << parindex << "] = " << data[parindex] << endl;
				swap(data[rightindex], data[parindex]);
				parindex = rightindex;
				swapping = true;
			}
		}

		leftindex = parindex*2+1;
		rightindex = parindex*2+2;
		if(leftindex > last) {
			//cout << "LeftIDX > Last (" << leftindex << " > " << last << ")" << endl; 
			break;
		} else {
			DeleteRootComparisons++;
			if (rightindex > last) {
				//cout << "RightIDX > Last (" << rightindex << " > " << last << ")" << endl; 
				if (data[parindex] < data[leftindex]) {
					// cout << "(swapping data[" << parindex << "] = " << data[parindex] << ") < (data[" << leftindex << "] = " << data[leftindex] << ")" << endl;
					swap(data[parindex], data[leftindex]);
					break;
				}
			} else {
				DeleteRootComparisons++;
			}
		}
	}

	//cout << "DeleteRootComparisons =" << DeleteRootComparisons << endl;
	/*
	cout << "VectorHeap = ";
	for(int i=0; i<=data.size(); i++) {
		cout << data[i] << " ";
	} 
	cout << endl;
	cout << "Size of last = " << last << endl << endl;
	*/

	return temp;
}

void Heap::PrintHeap(){
	for(int i=0; i<=last; i++){
		cout << data[i] << " ";
	}
	cout << endl;
}



/* this is a possible prototype for the heapsort function */
/* the char* filename is just for printing the name, the file is opened and dealt with in the main() */
void heapsort(vector<int> &sortingvector,int number_of_elements, char* filename){
  Heap myHeap; //declare a Heap instance here *
  int InsertHeapComparisons = 0;

  /* Using the sortingvector, INSERT elements into the Heap */
  /* STORE how many comparisons were made */
  for (int i=0; i<number_of_elements; i++) {
  	cout << "File: " << filename << " sortingvector = " << sortingvector[i] << endl;
  	InsertHeapComparisons = myHeap.InsertHeap(sortingvector[i]) + InsertHeapComparisons;
  }

  // HEAP BEFORE SORTING: ....
  /* After building the heap from the file, PRINT the current state of the heap before sorting */
  cout << "Heap before sorting: " << filename << endl;
  myHeap.PrintHeap();
  
  /* Print how many InsertHeap comparisons were made */
  cout << "InsertHeap: " << InsertHeapComparisons << " comparisons" << endl;
 
  /* DELETE elements from the Heap, copying it back to the vector in a way that it is sorted */
  /* STORE how many comparisons were made for the deletion process */
  for (int z=number_of_elements; z>0; z--){
  	sortingvector[z] = myHeap.DeleteRoot();
  	// cout << "PostDelete-sortingvector[" << z << "] = " << sortingvector[z] << endl << endl;
  }

  /* PRINT the number of comparisons for the Deletion tasks */
  cout << "DeleteRoot: " << myHeap.getDeleteRootComparisons() << " comparisons" << endl;
  
  /* Print the state of the vector after sorting */
  cout << "Vector after sorting:" << endl;
  for (int a=1; a <= sortingvector.size(); a++){
  	cout <<  sortingvector[a] << " ";
  }
  cout << endl;
  
  
  
  
  
}




int main( int argc, char** argv ){//get filename from arguments
	char expression[100];
	int number;
	ifstream input_file_random;
	ifstream input_file_reversed;
	ifstream input_file_sorted;
	if(argc==4) {
	  input_file_random.open(argv[1]);
	  input_file_reversed.open(argv[2]);
	  input_file_sorted.open(argv[3]);
	}
	else {printf("The program needs 3 filenames, in this order: random, reversed and sorted.\n");exit(0);}
	int number_of_elements_random=0;
	while(input_file_random >> number){
		sscanf(expression,"%d",&number);
		/*Comment out this printout, this is just to check that the file can be read */
		printf("%d ",number );	
		vector_random.push_back(number);
		number_of_elements_random++;
	}
	/*Comment out this printout, this is just to check that the array was copied */
	printf("File %s:\n", argv[1]);
	for(int count=0;count<number_of_elements_random;count++){
		printf("%d ",vector_random[count]);
	}
	printf("\n");
	/*end printout*/

	int number_of_elements_reversed=0;
	while(input_file_reversed >> number){
		sscanf(expression,"%d",&number);
		/*Comment out this printout, this is just to check that the file can be read */
//		printf("%d ",number );	
		vector_reversed.push_back(number);
		number_of_elements_reversed++;
	}
	/*Comment out this printout, this is just to check that the array was copied */
	printf("File %s:\n", argv[2]);
	for(int count=0;count<number_of_elements_reversed;count++){
		printf("%d ",vector_reversed[count]);
	}
	printf("\n");
	/*end printout*/


	int number_of_elements_sorted=0;
	while(input_file_sorted >> number){
		sscanf(expression,"%d",&number);
		/*Comment out this printout, this is just to check that the file can be read */
//		printf("%d ",number );	
		vector_sorted.push_back(number);
		number_of_elements_sorted++;
	}
	/*Comment out this printout, this is just to check that the array was copied */
	printf("File %s:\n", argv[3]);
	for(int count=0;count<number_of_elements_sorted;count++){
		printf("%d ",vector_sorted[count]);
	}
	printf("\n");
	/*end printout*/


	/* Implement or call your Heap sort here, the Heap class with methods should be copied/implemented before main() */
	heapsort(vector_random, number_of_elements_random, argv[1]);
	cout << endl;
	heapsort(vector_reversed, number_of_elements_reversed, argv[2]);
	cout << endl;
	heapsort(vector_sorted, number_of_elements_sorted, argv[3]);
	
}
