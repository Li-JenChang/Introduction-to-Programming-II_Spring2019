#pragma once

#include <BattleShipGame/Board.h>
#include <BattleShipGame/Wrapper/AI.h>
#include <GUI/GUIInterface.h>

#include <iostream>
#include <cassert>
#include <chrono>
#include <cstdarg>
#include <future>
#include <type_traits>

namespace TA
{
    class BattleShipGame
    {
    public:
        BattleShipGame(
            int size,
            std::chrono::milliseconds runtime_limit = std::chrono::milliseconds(1000)
        ):
            m_size(size),
            m_runtime_limit(runtime_limit),
            m_P1(nullptr),
            m_P2(nullptr),
            m_P1Board(size),
            m_P2Board(size)
        {
            gui = new ASCII;
            m_ship_size = {3,3,5,7};
        }

        void setPlayer1(AIInterface *ptr) { assert(checkAI(ptr)); m_P1 = ptr; }
        void setPlayer2(AIInterface *ptr) { assert(checkAI(ptr)); m_P2 = ptr; }

        void run()
        {
            gui->title();
            if( !prepareState() ) return ;

            updateGuiGame();

            //Todo: Play Game
            std::vector<std::pair<int,int>> prev_atk;
            std::pair<int, int> atk;
            std::vector<std::pair<int, int>> tmp_new_pos;
            int alive_ship, sz;
            bool is_over = false;
            while(1){
                m_P1->callbackReportEnemy(prev_atk);
                prev_atk.clear();
                alive_ship = 0;
                for( Ship s : m_P1Ship ){
                    if(s.state != Ship::State::Sink) alive_ship++;
                }
                //atttack
                while(alive_ship--){
                    atk = m_P1->queryWhereToHit(m_P2Board);
                    
                    //check whether atk is legal
                    if(atk.first > 19 || atk.first < 0 || atk.second > 19 || atk.second < 0){
                        //P1 lose
                        putToGui("P2 win!(illegal atttack : out of board)\n");
                        is_over = true;
                        break;
                    }else if(m_P2Board[atk.first][atk.second] != Board::State::Unknown){
                        //P1 lose
                        putToGui("P2 win!(illegal atttack : repeated atk position)\n");
                        is_over = true;
                        break;
                    }
                    
                    prev_atk.push_back(atk);
                    putToGui("P1 atttak attt (%d, %d)\n", atk.first, atk.second);
                    //ohya means whether atk atttack attt ship
                    bool ohya = false;
                    m_P2Board[atk.first][atk.second] = Board::State::Empty;
                    for( Ship &s : m_P2Ship ){
                        if(ohya == true) break;
                        for(int i = 0; i < s.size; i++){
                            if(ohya == true) break;
                            for(int j = 0;j < s.size;j++){
                                if(ohya == true) break;
                                if(s.x+i == atk.first && s.y+j == atk.second){
                                    m_P2Board[atk.first][atk.second] = Board::State::Hit;
                                    
                                    if(i == s.size/2 && j == s.size/2){
                                        putToGui("Poor P2:(\n");
                                        s.state = Ship::State::Sink;
                                    }else{
                                        if(s.state != Ship::State::Sink)
                                            s.state = Ship::State::Hit;
                                    }
                                        
                                    ohya = true;
                                }
                            }
                        }
                    }
                    m_P1->callbackReportHit(ohya);
                    updateGuiGame();
                    if(checkGameover()){
                        is_over = true;
                        break;
                    }
                }
                if(is_over) break;
                
                //moving
                tmp_new_pos.clear();
                tmp_new_pos = m_P1->queryHowToMoveShip(m_P1Ship);
                sz = tmp_new_pos.size();
                if(sz != 4){
                    //P1 lose
                        putToGui("P2 win!(illegal movement)\n");
                        is_over = true;
                        break;
                }
                for(int i = 0; i < sz; i++){
                    int dx = tmp_new_pos[i].first - m_P1Ship[i].x;
                    int dy = tmp_new_pos[i].second - m_P1Ship[i].y;
                    if(dx != 0 && dy != 0){
                        putToGui("P2 win!(illegal movement)\n");
                        is_over = true;
                        break;
                    }else if(dx < -1 || dx > 1 || dy > 1 || dy < -1){
                        putToGui("P2 win!(illegal movement)\n");
                        is_over = true;
                        break;
                    }else{
                        if(dx!=0 || dy != 0){
                            if(m_P1Ship[i].state != Ship::State::Available){
                                putToGui("P2 win!(illegal movement)\n");
                                is_over = true;
                                break;
                            } 
                            putToGui("P1 is moving\n");
                            m_P1Ship[i].x = tmp_new_pos[i].first;
                            m_P1Ship[i].y = tmp_new_pos[i].second;
                        }
                    }
                }
                if(is_over) break;
                if(!checkShipPosition(m_P1Ship)){
                        //P1 lose
                        putToGui("P2 win!(illegal movement)\n");
                        is_over = true;
                        break;
                }
                updateGuiGame();

                //-----------------------------------------------------------------

                m_P2->callbackReportEnemy(prev_atk);
                prev_atk.clear();
                alive_ship = 0;
                for( Ship s : m_P2Ship ){
                    if(s.state != Ship::State::Sink) alive_ship++;
                }
                while(alive_ship--){
                    atk = m_P2->queryWhereToHit(m_P1Board);
            
                     //check whether atk is legal
                    if(atk.first > 19 || atk.first < 0 || atk.second > 19 || atk.second < 0){
                        //P2 lose
                        putToGui("P1 win!(illegal atttack : out of board)\n");
                        is_over = true;
                        break;
                    }else if(m_P1Board[atk.first][atk.second] != Board::State::Unknown){
                        //P2 lose
                        putToGui("P1 win!(illegal atttack : repeated atk position)\n");
                        is_over = true;
                        break;
                    }
                    if(is_over) break;
                    prev_atk.push_back(atk);
                
                    putToGui("P2 atttak attt (%d, %d)\n", atk.first, atk.second);
                    bool ohya = false;
                    m_P1Board[atk.first][atk.second] = Board::State::Empty;
                    for( Ship &s : m_P1Ship ){
                        if(ohya == true) break;
                        for(int i = 0; i < s.size; i++){
                            if(ohya == true) break;
                            for(int j = 0;j < s.size;j++){
                                if(ohya == true) break;
                                if(s.x+i == atk.first && s.y+j == atk.second){
                                    m_P1Board[atk.first][atk.second] = Board::State::Hit;
                                    
                                    if(i == s.size/2 && j == s.size/2){
                                        putToGui("Poor P1:(\n");
                                        s.state = Ship::State::Sink;
                                    }else{
                                        if(s.state != Ship::State::Sink)
                                            s.state = Ship::State::Hit;
                                    }                                  
                                    ohya = true;
                                }
                            }
                        }
                    }
                    m_P2->callbackReportHit(ohya);
                    
                    updateGuiGame();
                    if(checkGameover()){
                        is_over = true;
                        break;
                    }
                }
                if(is_over) break;
                //moving
                tmp_new_pos.clear();
                tmp_new_pos = m_P2->queryHowToMoveShip(m_P2Ship);
                sz = tmp_new_pos.size();
                if(sz != 4){
                    //P2 lose
                        putToGui("P1 win!(illegal movement)\n");
                        is_over = true;
                        break;
                }
                for(int i = 0; i < sz; i++){
                    int dx = tmp_new_pos[i].first - m_P2Ship[i].x;
                    int dy = tmp_new_pos[i].second - m_P2Ship[i].y;
                    if(dx != 0 && dy != 0){
                        putToGui("P1 win!(illegal movement)\n");
                        is_over = true;
                        break;
                    }else if(dx < -1 || dx > 1 || dy > 1 || dy < -1){
                        putToGui("P1 win!(illegal movement)\n");
                        is_over = true;
                        break;
                    }else{
                        if(dx!=0 || dy != 0){
                            if(m_P2Ship[i].state != Ship::State::Available){
                                putToGui("P1 win!(illegal movement)\n");
                                is_over = true;
                                break;
                            }
                            putToGui("P2 is moving\n");
                            m_P2Ship[i].x = tmp_new_pos[i].first;
                            m_P2Ship[i].y = tmp_new_pos[i].second;
                        }
                    }
                }
                if(is_over) break;
                if(!checkShipPosition(m_P2Ship)){
                        //P2 lose
                        putToGui("P1 win!(illegal movement)\n");
                        is_over = true;
                        break;
                }
                updateGuiGame();
                
            }

        }

   private:
        void updateGuiGame()
        {
            gui->updateGame(m_P1Board, m_P1Ship, m_P2Board, m_P2Ship);
        }

        bool checkGameover()
        {
            bool flag = true;

            for( Ship s : m_P1Ship )
                if( s.state != Ship::State::Sink )
                    flag = false;
            if( flag )
            {
                putToGui("P2 win!\n");
                return true;
            }
            for( Ship s : m_P2Ship )
                if( s.state != Ship::State::Sink )
                    return false;
            putToGui("P1 win!\n");
            return true;
        }

        bool prepareState()
        {
            std::vector<Ship> initPos;

            putToGui("P1 Prepareing...\n");
            initPos = call(&AIInterface::init, m_P1, m_size, m_ship_size, true, m_runtime_limit);
            if( !checkShipPosition(initPos) )
            {
                putToGui("P1 Init() Invaild...\n");
                return false;
            }
            for( auto &ship : initPos )
                ship.state = Ship::State::Available;
            m_P1Ship = initPos;
            putToGui("Done.\n");

            initPos.clear();

            putToGui("P2 Prepareing...\n");
            initPos = call(&AIInterface::init, m_P2, m_size, m_ship_size, false, m_runtime_limit);
            if( !checkShipPosition(initPos) )
            {
                putToGui("P2 Init() Invaild...\n");
                return false;
            }
            for( auto &ship : initPos )
                ship.state = Ship::State::Available;
            m_P2Ship = initPos;
            putToGui("Done.\n");
            return true;
        }

        template<typename Func ,typename... Args,
            std::enable_if_t< std::is_void<
                    std::invoke_result_t<Func, AIInterface, Args...>
                >::value , int> = 0 >
        void call(Func func, AIInterface *ptr, Args... args)
        {
            std::future_status status;
            auto val = std::async(std::launch::async, func, ptr, args...);
            status = val.wait_for(std::chrono::milliseconds(m_runtime_limit));

            if( status != std::future_status::ready )
            {
                exit(-1);
            }
            val.get();
        }

        template<typename Func ,typename... Args,
            std::enable_if_t< std::is_void<
                    std::invoke_result_t<Func, AIInterface, Args...>
                >::value == false, int> = 0 >
        auto call(Func func, AIInterface *ptr, Args... args)
            -> std::invoke_result_t<Func, AIInterface, Args...>
        {
            std::future_status status;
            auto val = std::async(std::launch::async, func, ptr, args...);
            status = val.wait_for(std::chrono::milliseconds(m_runtime_limit));

            if( status != std::future_status::ready )
            {
                exit(-1);
            }
            return val.get();
        }

        void putToGui(const char *fmt, ...)
        {
            va_list args1;
            va_start(args1, fmt);
            va_list args2;
            va_copy(args2, args1);
            std::vector<char> buf(1+std::vsnprintf(nullptr, 0, fmt, args1));
            va_end(args1);
            std::vsnprintf(buf.data(), buf.size(), fmt, args2);
            va_end(args2);

            if( buf.back() == 0 ) buf.pop_back();
            gui->appendText( std::string(buf.begin(), buf.end()) );
        }

        bool checkAI(AIInterface *ptr)
        {
            return ptr->abi() == AI_ABI_VER;
        }

        bool checkShipPosition(std::vector<Ship> ships)
        {

            if( ships.size() != m_ship_size.size() )
            {
                putToGui("Ship number not match : real %zu ,expect %zu,\n",ships.size(), m_ship_size.size());
                return false;
            }

            std::sort(ships.begin(), ships.end(), [](Ship a, Ship b){
                return a.size < b.size;
            });

            std::vector<std::vector<int>> map(m_size, std::vector<int>(m_size));

            int id = -1;
            for( auto [size, x, y, state] : ships )
            {
                id++;
                if( size != m_ship_size[id] )
                {
                    putToGui("Ship %d size not match : real %zu ,expect %zu,\n", id, size, m_ship_size[id]);
                    return false;
                }

                for( int dx = 0 ; dx < size ; dx++ )
                    for( int dy = 0 ; dy < size ; dy++ )
                        {
                            int nx = x + dx;
                            int ny = y + dy;

                            if( nx < 0 || nx >= m_size || ny < 0 || ny >= m_size )
                            {
                                putToGui("Ship %d out of range at (%d,%d),\n", id, nx, ny);
                                return false;
                            }

                            if( map[nx][ny] != 0 )
                            {
                                putToGui("Ship %d and %d overlapping at (%d,%d),\n", id,  map[nx][ny]-1, nx, ny);
                                return false;
                            }
                            map[nx][ny] = id + 1;
                        }
            }
            return true;
        }

        int m_size;
        std::vector<int> m_ship_size;
        std::chrono::milliseconds m_runtime_limit;

        AIInterface *m_P1;
        AIInterface *m_P2;
        GUIInterface *gui;

        std::vector<Ship> m_P1Ship;
        std::vector<Ship> m_P2Ship;
        Board m_P1Board;
        Board m_P2Board;
    } ;
}
