#pragma once

#include <iostream>
#include <iomanip>
#include <vector>
#include <array>
#include <cmath>
#include <set>
#include <queue>

template<int SIZE> struct Node
{
    using State = std::array<int, SIZE>;

    State st;         // the state of sliding puzzle
    Node* parent;     // parent in the search tree
    int level;        // level of the search tree ( number of steps )

    // constructors
    Node( State input ): st( input ), level( 0 ), parent( nullptr ) {}
    Node( State input, Node* p ): st( input ), level( p->level + 1 ), parent( p ) {}
};

template<int SIZE> class SlidingPuzzle
{
    using State = std::array<int, SIZE>;
private:
    State st;           // initial state
    Node<SIZE>* sol;    // the solution
    int LEN;            // LEN = sqrt(SIZE)

public:
    // constructor
    SlidingPuzzle( State input ): st( input ), sol( nullptr ) {
        LEN = sqrt( SIZE );
    }

    // Move a block from the "from" position to the "to" position
    State MoveBlock( State s, int from, int to )
    {
        State t = s;
        // Swap from and to
        int temp = t[from];
        t[from] = t[to];
        t[to] = temp;

        return t;
    }

    // Search the possible states from the current state s.
    std::set<State> extend( State s )
    {
        std::set<State> SS;
        int pos, x, y;

        // Find where 0 is
        for ( pos = 0; pos < SIZE; pos++ ) {
            if ( s[pos] == 0 ) {
                x = pos % LEN;
                y = pos / LEN;
                break;
            }
        }

        // Move up
        if ( y < LEN - 1 )
            SS.insert( MoveBlock( s, pos + LEN, pos ) );
        // Move down
        if ( y > 0 )
            SS.insert( MoveBlock( s, pos - LEN, pos ) );
        // Move left
        if ( x < LEN - 1 )
            SS.insert( MoveBlock( s, pos + 1, pos ) );

        // Move right
        if ( x > 0 )
            SS.insert( MoveBlock( s, pos - 1, pos ) );

        return SS;
    }

    // Check if the state st is the solution, which is
    // 1, 2, ... , N-1, 0
    bool found( State st )
    {
        for ( int i = 0; i < SIZE - 1; i++ ) {
            if ( st[i] != i + 1 )
                return false;
        }
        return true;
    }

    // Solve the puzzle by using BFS
    int solve()
    {
        std::queue<Node<SIZE>*>  q;
        std::set<State> explored;                      // the states appeared before
        Node<SIZE>* current = new Node<SIZE> ( st );   // current solution
        Node<SIZE>* child;                             // expanded solution
        bool done = false;                             // flag for termination

        // Early exit check
        if ( found( current->st ) ) {
            show_state( current->st );
            return 0;
        }
        // Start the search
        q.push( current );
        explored.insert( current->st );

        while( q.size() != 0 && !done ) {
            current = q.front();
            auto nextStates = extend( current->st );      // find all possible states
            for ( auto s : nextStates ) {
                if ( found( s ) ) {     // found a solution
                    sol = new Node <SIZE> ( s, current );
                    done = true;
                }
                else {     // not a solution
                    auto next = explored.find( s );
                    if ( next == explored.cend() ) {     // a new node
                        child = new Node <SIZE> ( s, current );
                        explored.insert( s );
                        q.push( child );
                    }
                }
            }
            q.pop();
        }

        return done ? sol->level : -1;
    }

    // Show the status of the board
    void show_state( State s )
    {
        int i, j;
        for ( i = 0; i < LEN; i++ ) {
            for ( j = 0; j < LEN; j++ )
                std::cout << std::setw( 4 ) << s[i * LEN + j] << " ";
            std::cout << std::endl;
        }
    }

    // Find the difference between two consecutive states
    int diff( State &a, State &b )
    {
        int i;
        for ( i = 0; i < SIZE; i++ ) {
            if ( a[i] != b[i] ) {
                return ( a[i] == 0 ) ? b[i] : a[i];
            }
        }
        return -1;
    }

    // Show the solving sequence
    void show_solutions()
    {
        Node<SIZE>* temp = sol;
        std::vector<State> tracking;
        int i;

        while ( temp != nullptr ) {     // back tracking
            tracking.push_back( temp->st );
            temp = temp->parent;
        }

        for ( i = tracking.size() - 1; i > 0; i-- ) {
            show_state( tracking[i] );
            std::cout << "move "<< diff( tracking[i - 1], tracking[i] ) <<"->\n";
        }
        show_state( tracking[0] );
    }
};
