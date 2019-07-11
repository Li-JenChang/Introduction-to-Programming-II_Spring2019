#pragma once

#include <vector>
#include <set>

using State = std::vector<int>;

struct Node
{
    State st;
    Node* parent;

    Node( State input ): st( input ), parent( nullptr ) {}
};

class Pouring
{
private:
    std::vector<int> capacities;    // ex: { 3, 5 }

public:
    Pouring( std::vector<int> cp ): capacities{ cp } {}

    State Empty( State s, int jug_no );

    State Fill( State s, int jug_no );

    State Pour( State s, int from, int to );

    std::set<State> extend( State s );

    bool found( State s, int target );

    void show_state( State s );

    void show_solutions( Node* current, State st );

    void solve( int target );
};
