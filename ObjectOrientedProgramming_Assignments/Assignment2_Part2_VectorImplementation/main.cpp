#include <iostream>
#include "a2p2.h"

using namespace std;

int main() {
    printStudentInfo();

    MyVector<int> v1;

    v1.pushBack(10);
    v1.pushBack(11);
    v1.pushBack(13);


    cout << "Should print out 10 11 13" << endl << v1 << endl;

    try {
        cout << "Should throw an exception" << endl;
        v1.insert(14, -1);
    }
    catch (invalid_argument &e) {
        cout << "Exception thrown: " << e.what() << endl;
    }

    v1.insert(12, 2);
    cout << "Should print out 10 11 12 13" << endl << v1 << endl;

    // removed should be 10
    int removed = v1.remove(0);

    cout << "Should print out 10" << endl << removed << endl;
    cout << "Should print out 11 12 13" << endl << v1 << endl;

    v1[1] = 120;
    cout << "Should print out 11 120 13" << endl << v1 << endl;

    v1.clear();
    cout << "Should print out nothing" << endl << v1 << endl;


    /// ALL GOOD UP TO HERE

    // Capacity = 7, size = 0
    v1 = MyVector<int>(7);

    for (int i = 1; i < 10; i++) {
        v1.pushBack(i);
    }

    cout << "Should print out 1 2 3 4 5 6 7 8 9" << endl << v1 << endl;

    // v2 must be a deep copy
    MyVector<int> v2 = v1;

    // v3 must be a shallow copy, and v2 must be nullified
    MyVector<int> v3(move(v2));

    cout << "Should print out 1 2 3 4 5 6 7 8 9" << endl << v1 << endl;
    cout << "Should print out nothing" << endl << v2 << endl;

    v3[0] = 0; v3.insert(1, 1);
    cout << "Should print out 0 1 2 3 4 5 6 7 8 9" << endl << v3 << endl;

    return 0;
}