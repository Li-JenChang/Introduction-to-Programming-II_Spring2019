#include <iostream>
#include <vector>
#include <set>
#include <queue>
#include "Pouring.h"

using namespace std;

State Pouring::Empty( State s, int jug_no )
{
    s[jug_no] = 0;
    return s;
}

State Pouring::Fill( State s, int jug_no )
{
    s[jug_no] = capacities[jug_no];
    return s;
}

State Pouring::Pour( State s, int from, int to )
{
    State t = s;
    int diff = capacities[to] - s[to];
    if ( diff < s[from] ) {
        t[to] = capacities[to];
        t[from] = s[from] - diff;
    }
    else {
        t[from] = 0;
        t[to] = s[to] + s[from];
    }
    return t;
}

set<State> Pouring::extend( State s )
{
    set<State> SS;
    for ( unsigned int i = 0; i < capacities.size(); ++i ) {
        SS.insert( Empty( s, i ) );
        SS.insert( Fill( s, i ) );
        for ( unsigned int j = 0; j < capacities.size(); ++j ) {
            if ( i != j )
                SS.insert( Pour( s, i, j ) );
        }
    }
    return SS;
}

bool Pouring::found( State s, int target )
{
    for ( auto t : s ) {
        if ( t == target ) return true;
    }
    return false;
}

void Pouring::show_state( State s )
{
    unsigned int i, ss = s.size() - 1;
    cout << "(";
    for ( i = 0; i < ss; i++ )
        cout << s[i] << "," ;
    cout << s.back() << ")";
}

void Pouring::show_solutions( Node* current, State st )
{
    Node* temp = current;
    vector<State> sol;

    while ( temp != nullptr ) {     // back tracking
        sol.push_back( temp->st );
        temp = temp->parent;
    }

    for ( int i = sol.size() - 1; i >= 0; i-- ) {
        show_state( sol[i] );
        cout << " -> ";
    }
    show_state( st );
    cout << endl;
}


void Pouring::solve( int target )
{
    queue<Node*>  q;
    set<State> explored;      // the states appeared before
    Node* current = new Node( State( capacities.size() ) );
    Node* child;
    bool done = false;

    q.push( current );
    explored.insert( current->st );

    while ( q.size() != 0 && !done ) {
        current = q.front();
        auto nextStates = extend( current->st );      // find all possible states
        for ( auto s : nextStates ) {
            if ( found( s, target ) ) {               // found a solution
                show_solutions( current, s );
                done = true;
            }
            else {                                    // not a solution
                auto next = explored.find( s );
                if ( next == explored.cend() ) {      // a new node
                    child = new Node( s );
                    child->parent = current;
                    explored.insert( s );
                    q.push( child );
                }
            }
        }
        q.pop();
    }
}
