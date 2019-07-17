#include <iostream>
#include <stdexcept>


using namespace std;


template <class T>
class MyVector {
	int VectorCapacity;		///< Max possible items in vector
	int VectorSize;			///< Number of items in Vector
	T* VectorPointer;		///< Vector Pointer

	void DoubleCapacity();		///< Double capacity of vector.

public:
	MyVector(); 	///< Default Constructor
	MyVector(int initialCapacity);		///< Constructor
	MyVector(const MyVector<T> &a);		///< Copy Constructor
	MyVector(MyVector<T>&& other);		///< Move Constructor
	const MyVector& operator=(const MyVector<T> &a);	///< Copy assignment
	MyVector& operator=(MyVector<T>&& other); 	///< Move Assignment
	~MyVector(); 	///< Destructor

	void clear(); 	///< Clear the vector
	void pushBack(const T& item); 	///< Add item at the end of the vector
	void insert(const T& item, int Position);	///< Insert item into vector at defined Position
	T remove(int index);	/// Remove at return item removed at defined vector[index]
	bool isEmpty();
	
	int getVectorCapacity() const;		/// VectorCapacity getter function
	int getVectorSize() const;		/// VectorSize getter function
	T& operator[](int index) const;		/// index operator
};


/**
 * @brief      { default constructor, set capacity & size to 0, doesn't allocate memory }
 */
template <class T>
MyVector<T>::MyVector(): VectorPointer(nullptr), VectorCapacity(0), VectorSize(0){
}


/**
 * @brief      { Constructor to set vectorCapacity to initialCapacity and vectorSize to 0, allocate memory for vector
 * 					if initialCapacity is negative throw invalid_argument exception  }
 *
 * @param[in]  initialCapacity  { The initial capacity of the vector }
 */
template <class T>
MyVector<T>::MyVector (int initialCapacity): VectorCapacity(initialCapacity), VectorSize(0) {
	VectorPointer = nullptr;
	if (VectorCapacity < 0) {
		throw invalid_argument("Capacity cannot be negative");
	} else if (VectorCapacity >= 0) { 
		VectorPointer = new T[VectorCapacity];
	}
}


/**
 * @brief      { Copy constructor }
 *
 * @param[in]  other  { The address of source vector object to be copied. }
 */
template <class T>
MyVector<T>::MyVector(const MyVector<T> &other){
	VectorCapacity = other.VectorCapacity;
	VectorSize = other.VectorSize;
	VectorPointer = new T[VectorCapacity];
	for(int i = 0; i < VectorSize; ++i) {
		VectorPointer[i] = other.VectorPointer[i];
	}
}


/**
 * @brief      { Move Constructor }
 *
 * @param[in]  other  { The address of source vector object. }
 */
template <class T>
MyVector<T>::MyVector(MyVector<T>&& other) {
	VectorCapacity = other.VectorCapacity;
	VectorSize = other.VectorSize;
	VectorPointer = other.VectorPointer;

	other.VectorCapacity = 0;
	other.VectorSize = 0;
	other.VectorPointer = nullptr;
}


/**
 * @brief      { Copy Assignment Operator }
 *
 * @param[in]  other { The address of source vector object to be copied. }
 */
template <class T>
const MyVector<T>& MyVector<T>::operator=(const MyVector<T> &other) {
	if(this != &other) {
		VectorCapacity = other.VectorCapacity;
		VectorSize = other.VectorSize;
		delete[] VectorPointer;
		VectorPointer = new T[VectorCapacity];
		for(int i = 0; i < VectorSize; ++i) {
			VectorPointer[i] = other.VectorPointer[i];
		}
	}
	return *this;
}


/**
 * @brief      { Move Assignment Operator }
 *
 * @param[in]  other { The address of source vector object to be copied. }
 */
template <class T>
MyVector<T>& MyVector<T>::operator=(MyVector<T>&& other){
	if(this != &other){
		delete[] VectorPointer;
		VectorCapacity = other.VectorCapacity;
		VectorSize = other.VectorSize;
		VectorPointer = other.VectorPointer;

		other.VectorCapacity = 0;
		other.VectorSize = 0;
		other.VectorPointer = nullptr;
	}
	return *this;
}


/**
 * @brief      { Vector Object Destructor }
 */
template <class T>
MyVector<T>::~MyVector() {
	delete[] VectorPointer;
	VectorPointer = nullptr;
	VectorSize = 0;
}

/**
 * @brief      { Clear Function, clear vector object of all its data and memory. }
 */
template <class T>
void MyVector<T>::clear(){
	VectorCapacity = 0;
	VectorSize = 0;
	delete[] VectorPointer;
	VectorPointer = nullptr;
}

/**
 * @brief      { Push Back, add item at end of vector. }
 *
 * @param[in]  item  { The item }
 */
template <class T>
void MyVector<T>::pushBack(const T& item){
	
	if((VectorCapacity == 0) || (VectorSize >= VectorCapacity)) {
		DoubleCapacity();
	}
	
	VectorPointer[VectorSize] = item;
	VectorSize += 1;
}


/**
 * @brief      { Insert Function, add item into vector at desired location/position and shuffle other objects along }
 *
 * @param[in]  item      { The item }
 * @param[in]  Position  { The position }
 */
template <class T>
void MyVector<T>::insert(const T& item, int Position){
	if ((Position < 0) || (Position > VectorSize)) {
		throw invalid_argument("Index out of bounds");
	}
	
	if((VectorCapacity == 0) || (VectorSize >= VectorCapacity)) {
		DoubleCapacity();
	}

	if(Position == VectorSize) { 
		pushBack(item);
	} else {
		for(int i = VectorSize; i > Position; --i) {
			VectorPointer[i] = VectorPointer[i - 1];
		}
	VectorPointer[Position] = item;
	VectorSize += 1;
	}

	if((VectorCapacity == 0) || (VectorSize >= VectorCapacity)) {
		DoubleCapacity();
	}
}


/**
 * @brief      { Remove Function, remove item from vector at the specificed index. }
 *
 * @param[in]  index  { The vector index }
 *
 * @return     { the removed object }
 */
template <class T>
T MyVector<T>::remove(int index) {
	if((index >= VectorSize) || (index < 0)) {
		throw invalid_argument("Index out of bounds");
	}

	T removed = VectorPointer[index];
	VectorSize -= 1;
	for (int i = index; i < VectorSize; ++i) {
		VectorPointer[i] = VectorPointer[i + 1];
	}
	return removed;
}


/**
 * @brief      { Determines if empty. }
 *
 * @tparam     T     { description }
 *
 * @return     { True if empty, False otherwise. }
 */
template <class T>
bool MyVector<T>::isEmpty(){
	if(VectorSize == 0){
		return true;
	} else { return false; }
}


/**
 * @brief     { Gets the vector capacity. }
 *
 * @tparam     T     { description }
 *
 * @return     { The vector capacity. }
 */
template <class T>
int MyVector<T>::getVectorCapacity() const {return VectorCapacity;}


/**
 * @brief      { Gets the vector size. }
 *
 * @return     { The vector size. }
 */
template <class T>
int MyVector<T>::getVectorSize() const {return VectorSize;}


/**
 * @brief      { Index Operator Overload, used for access to vector object }
 *
 * @param[in]  index  { The index }
 *
 * @return     { item at specified index location of vector object }
 */
template <class T>
T& MyVector<T>::operator[](int index) const {
	if((index >= VectorSize) || (index < 0)) {
		throw invalid_argument("Index out of bounds");
	}
	return VectorPointer[index];
}


/**
 * @brief      { Output Operator Overload }
 *
 * @param      os     { The output stream }
 * @param[in]  other  { The other }
 */
template <class T>
ostream &operator<<(ostream &os, const MyVector<T> &other) {
	int a = other.getVectorSize();
	for(int i = 0; i < a; ++i){
		os << other[i] << " ";
	}
	return os;
}


/**
 * @brief      { Double Capacity, if capacity is 0 set it to 1, size is larger or equal to capacity, double capacity }
 */
template <class T>
void MyVector<T>::DoubleCapacity(){
		if (VectorCapacity == 0){
			VectorCapacity = 1;
		} else {
			VectorCapacity *= 2;
		}

		T* temp = new T[VectorCapacity];
		
		for(int i = 0; i < VectorSize; ++i){
			temp[i] = VectorPointer[i];
		}

		delete[] VectorPointer;
		VectorPointer = temp;
		temp = nullptr;
	}


/**
 * @brief      { Output Student Info }
 */
void printStudentInfo(){
	cout << "*****************************************" << endl;
	cout << "* Assignment 2 Part 2 *" << endl;
	cout << "* Taylor Bennett *" << endl;
	cout << "* 16105740 *" << endl;
	cout << "*****************************************" << endl;
}

