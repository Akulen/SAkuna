#ifndef SAKUNA_HPP_
#define SAKUNA_HPP_

#include <string>
#include <unordered_map>
#include <vector>

#include "board.hpp"
#include "magicmoves.hpp"
#include "move.hpp"
#include "piece.hpp"
#include "uci.hpp"

class SAkuna {
    uci &u;
    Board board;
    int nb_states;
    std::unordered_map<Board, std::pair<int, std::pair<Move, double>>> transposition;
    std::unordered_map<Board, int> repetition;
    public:
    SAkuna(uci&);
    void init();
    void set_position(const std::string&, const std::vector<std::string>&);
    bool check();
    bool valid(Move);
    std::pair<Move, double> alphabeta(Board, int, double, double);
    void start_search();
    void display_board();
    int perft(Board, int);
    void divide(int);
    ~SAkuna();
};

#endif
