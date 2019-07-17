/// Taylor TEST CPP file.

#include <iostream>
#include "a2p2.h"

using namespace std;

int main() {
    printStudentInfo();

    MyVector<int> v1;

    cout << v1 << endl;

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
}