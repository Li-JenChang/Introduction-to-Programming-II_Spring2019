#pragma once

#include <BattleShipGame/Wrapper/AI.h>
#include <BattleShipGame/Ship.h>
#include <algorithm>
#include <random>
#include <ctime>
#include <queue>

class AI : public AIInterface
{
    std::vector<std::pair<int,int>> random;
    std::vector<std::pair<int,int>> hit;     // record the enemy's squares that are of Hit state
    std::queue<std::pair<int,int>> target;
    std::pair<int,int> lastHit;
    bool lastResult;
    bool shipSink;
    std::vector<std::vector<bool>> attacked;  // true: attacked, false: unattacked
    unsigned int nEnemyShips;
public:
    virtual std::vector<TA::Ship> init(int size ,std::vector<int> ship_size, bool order, std::chrono::milliseconds runtime) override
    {
        (void)ship_size;
        (void)runtime;

        // Initialize ships
        std::vector<TA::Ship> tmp;
        tmp.push_back( { 3,  2,  1, TA::Ship::State::Available } ); // 2 3
        tmp.push_back( { 3, 14, 14, TA::Ship::State::Available } ); // 14 14
        tmp.push_back( { 5,  4, 13, TA::Ship::State::Available } ); // 5 13
        tmp.push_back( { 7, 11,  2, TA::Ship::State::Available } ); // 10 2
        
        for( int i = 0; i < size; ++i )
            for(int j = 0; j < size; ++j )
                random.emplace_back( i, j );

        std::mt19937 mt;
        mt.seed( std::time( nullptr ) + 7122 + ( order ? 1 : 0 ) );
        std::shuffle( random.begin(), random.end(), mt );
        
        // Initial aimed targets
        target.emplace(  4,  4 );
        target.emplace( 14, 15 );
        target.emplace(  4, 15 );
        target.emplace( 14,  4 );
        target.emplace(  9,  4 );
        target.emplace(  9, 15 );
        target.emplace(  4,  9 );
        target.emplace( 14,  9 );
        target.emplace(  9,  9 );

        attacked = *( new std::vector<std::vector<bool>>( size, std::vector<bool> ( size, false ) ) );
        nEnemyShips = 4;
        lastResult = false;
        shipSink = false;
        lastHit = std::pair<int,int>( -1, -1 );
        return tmp;
    }

    virtual void callbackReportEnemy( std::vector<std::pair<int,int>> enemyHit ) override
    {
        if ( !enemyHit.empty() ) {
            for ( auto p : enemyHit ) {
                attacked[p.first][p.second] = true;
            }
            if ( enemyHit.size() < nEnemyShips ) shipSink = true;
            else shipSink = false;
            nEnemyShips = enemyHit.size();
        }
    }

    virtual std::pair<int,int> queryWhereToHit( TA::Board enemyBoard ) override
    {
        if ( lastHit != std::pair<int,int>( -1, -1 ) ) {
            // Check Special Cases
            checkSpecialCases( enemyBoard );
            // Successfully hit a ship last time
            if ( lastResult ) {
                // Attack the surrounding squares
                int x = lastHit.first, y = lastHit.second;
                if ( x > 0 ) target.emplace( x - 1, y );
                if ( x < enemyBoard.size() - 1 ) target.emplace( x + 1, y );
                if ( y > 0 ) target.emplace( x, y - 1 );
                if ( y < enemyBoard.size() - 1 ) target.emplace( x, y + 1 );
            }
            if ( shipSink ) {
                target.emplace( random.back() );
                random.pop_back();
            }
        }
        
        while ( !target.empty() && enemyBoard[target.front().first][target.front().second] != TA::Board::State::Unknown ) {
            target.pop();
        }

        if ( target.empty() ) {
            target.emplace( random.back() );
            random.pop_back();
        }

        auto ret = target.front();
        lastHit = ret;
        target.pop();

        for ( std::vector<std::pair<int,int>>::iterator it = random.begin(); it != random.end(); ++it ) {
            if ( *it == ret ) {
                random.erase( it );
                break;
            }
        }
        
        return ret;
    }

    virtual void callbackReportHit( bool result )  override
    {
        lastResult = result;
        if ( result ) hit.emplace_back( lastHit );
    }

    virtual std::vector<std::pair<int,int>> queryHowToMoveShip( std::vector<TA::Ship> current ) override
    {
        
        bool up, down, left, right;
        std::vector<std::pair<int,int>> ret;
        std::vector<TA::Ship> update = current;
        for ( auto ship : update ) {
            if ( ship.state != TA::Ship::State::Available ) {
                ret.emplace_back( ship.x, ship.y );
                continue;
            }
            up = false; 
            down = false; 
            left = false; 
            right = false;
            if ( ship.x > 0 && ship.x < 20 - ship.size ) {
                for ( auto j = 0; j < ship.size; j++ ) {
                    if ( attacked[ship.x - 1][ship.y + j] ) {
                        up = true;
                        break;
                    }
                }
                for ( auto j = 0; j < ship.size; j++ ) {
                    if ( attacked[ship.x + ship.size][ship.y + j] ) {
                        down = true;
                        break;
                    }
                }
            }
            if ( ship.y > 0 && ship.y < 20 - ship.size ) {
                for ( auto i = 0; i < ship.size; i++ ) {
                    if ( attacked[ship.x + i][ship.y - 1] ) {
                        left = true;
                        break;
                    }
                }
                for ( auto i = 0; i < ship.size; i++ ) {
                    if ( attacked[ship.x + i][ship.y + ship.size] ) {
                        right = true;
                        break;
                    }
                }
            }
            if ( up && !down && !checkOverlap( update, ship, ship.x + 1, ship.y ) ) {
                ret.emplace_back( ship.x + 1, ship.y );
                ship.x = ship.x + 1;
            }
            else if ( !up && down && !checkOverlap( update, ship, ship.x - 1, ship.y ) ) {
                ret.emplace_back( ship.x - 1, ship.y );
                ship.x = ship.x - 1;
            }
            else if ( left && !right && !checkOverlap( update, ship, ship.x, ship.y  + 1 ) ) {
                ret.emplace_back( ship.x, ship.y + 1 );
                ship.y = ship.y + 1;
            }
            else if ( !left && right && !checkOverlap( update, ship, ship.x, ship.y - 1 ) ) {
                ret.emplace_back( ship.x, ship.y - 1 );
                ship.y = ship.y - 1;
            }
            else {
                ret.emplace_back( ship.x, ship.y );
            }
        }
        return ret;
    }

    bool checkOverlap( std::vector<TA::Ship> update, TA::Ship s, int new_x, int new_y ) 
    {
        for ( auto ship : update ) {
            if ( ship.x == s.x && ship.y == s.y ) continue;
            for ( int i = ship.x; i < ship.x + ship.size; i++ ) {
                for ( int j = ship.y; j < ship.y + ship.size; j++ ) {
                    if ( i >= new_x && i < new_x + s.size && j >= new_y && j < new_y + s.size ) return true;
                }
            }
        }
        return false;
    }

    void checkSpecialCases( TA::Board enemyBoard ) 
    {
        // For special cases, attack diagonally to hit the center of the enemy's ship in at most three steps.
        for ( auto h : hit ) {
            int i = h.first;
            int j = h.second;
            // the edges of the board
            if ( i == 0 ) {
                if ( j == 0 || ( j > 0 && j < enemyBoard.size() - 2 && enemyBoard[i][j - 1] == TA::Board::State::Empty ) ) {
                    target.emplace( i + 1, j + 1 );
                    target.emplace( i + 2, j + 2 );
                    if ( j + 3 < enemyBoard.size() ) 
                        target.emplace( i + 3, j + 3 );
                }
                else if ( j == enemyBoard.size() - 1 || ( j > 1 && j < enemyBoard.size() - 1 && enemyBoard[i][j + 1] == TA::Board::State::Empty ) ) {
                    target.emplace( i + 1, j - 1 );
                    target.emplace( i + 2, j - 2 );
                    if ( j - 3 >= 0 ) 
                        target.emplace( i + 3, j - 3 );
                }
            }
            else if ( i == enemyBoard.size() - 1 ) {
                if ( j == 0 || ( j > 0 && j < enemyBoard.size() - 2 && enemyBoard[i][j - 1] == TA::Board::State::Empty ) ) {
                    target.emplace( i - 1, j + 1 );
                    target.emplace( i - 2, j + 2 );
                    if ( j + 3 < enemyBoard.size() ) 
                        target.emplace( i - 3, j + 3 );
                }
                else if ( j == enemyBoard.size() - 1 || ( j > 1 && j < enemyBoard.size() - 1 && enemyBoard[i][j + 1] == TA::Board::State::Empty ) ) {
                    target.emplace( i - 1, j - 1 );
                    target.emplace( i - 2, j - 2 );
                    if ( j - 3 >= 0 ) 
                        target.emplace( i - 3, j - 3 );
                }
            }
            // up and left
            else if ( enemyBoard[i - 1][j] == TA::Board::State::Empty && enemyBoard[i][j - 1] == TA::Board::State::Empty ) { 
                target.emplace( i + 1, j + 1 );
                target.emplace( i + 2, j + 2 );
                if ( i + 3 < enemyBoard.size() && j + 3 < enemyBoard.size() ) 
                    target.emplace( i + 3, j + 3 );
            }
            // down and left
            else if ( enemyBoard[i + 1][j] == TA::Board::State::Empty && enemyBoard[i][j - 1] == TA::Board::State::Empty ) {
                target.emplace( i - 1, j + 1 );
                target.emplace( i - 2, j + 2 );
                if ( i - 3 >= 0 && j + 3 < enemyBoard.size() ) 
                    target.emplace( i - 3, j + 3 );
            }
            // down and right
            else if ( enemyBoard[i + 1][j] == TA::Board::State::Empty && enemyBoard[i][j + 1] == TA::Board::State::Empty ) {
                target.emplace( i - 1, j - 1 );
                target.emplace( i - 2, j - 2 );
                if ( i - 3 >= 0 && j - 3 >= 0 ) 
                    target.emplace( i - 3, j - 3 );
            }
            // up and right
            else if ( enemyBoard[i - 1][j] == TA::Board::State::Empty && enemyBoard[i][j + 1] == TA::Board::State::Empty ) {
                target.emplace( i + 1, j - 1 );
                target.emplace( i + 2, j - 2 );
                if ( i + 3 < enemyBoard.size() && j - 3 >= 0 ) 
                    target.emplace( i + 3, j - 3 );
            }
        }
    }
};
