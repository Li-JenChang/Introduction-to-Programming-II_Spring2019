#pragma once

#include <vector>

namespace TA
{
    class Board
    {
    public:
        enum class State
        {
            Unknown,     // squares that have not been attacked
            Empty,       // squares that have been attacked but are empty
            Hit          // squares occupied by a boat that have been attacked
        };
        Board(int size)
            :m_size(size)
            ,m_board(size, std::vector<State>(size, State::Unknown))
        {}

        int size() const { return m_size;  }
        auto& operator[](int x) { return m_board[x]; }
        const auto& operator[](int x) const { return m_board[x]; }

    private:
        int m_size;
        std::vector<std::vector<State>> m_board;  // 2D array of State recording public information
    };
} // Namespace TA
