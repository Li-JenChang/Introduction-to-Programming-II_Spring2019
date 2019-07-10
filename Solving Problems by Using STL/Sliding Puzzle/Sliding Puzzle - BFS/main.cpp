#include <iostream>
#include <array>
#include "SlidingPuzzle.h"

#define B_SIZE 16

using namespace std;

int main()
{
    int step;

    array<int, B_SIZE> init;
    cout << "Input the initial status of the board in row major:" << endl;
    for ( auto i = 0; i < B_SIZE; i++ ) cin >> init[i];

    SlidingPuzzle<B_SIZE> puzzle( init );

    step = puzzle.solve();

    if ( step >= 0 ) {
        if ( step > 0 ) puzzle.show_solutions();
        cout << "Number of Steps = " << step << endl;
    }
    else
        cout << "No Solution!" << endl;

    return 0;
}
