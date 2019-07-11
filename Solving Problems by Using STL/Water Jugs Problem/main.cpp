#include <iostream>
#include "Pouring.h"

using namespace std;

int main()
{
    int capacity1, capacity2, target;

    cout << "Capacity of Bucket 1: ";
    cin >> capacity1;
    cout << "Capacity of Bucket 2: ";
    cin >> capacity2;
    cout << "Target Volume of Water: ";
    cin >> target;

    vector<int> jugs = { capacity1, capacity2 };

    Pouring problem( jugs );
    problem.solve( target );

    return 0;
}
