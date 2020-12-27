#include "sakuna.hpp"

#include <cmath>
#include <cstdio>
#include <utility>

using namespace std;

SAkuna::SAkuna(uci &_u) : u(_u) {
    initmagicmoves();
    lookup_table_init();
}

void SAkuna::init() {
    srand(time(NULL));
}

void SAkuna::set_position(const string& fen, const vector<string>& moves) {
    board = Board(fen, moves);
}

// check if player is in check
bool SAkuna::valid(Move move) {
    Board newBd;
    board.do_move(move, &newBd);
    return !newBd.in_check(board.player);
}

const int MAX_MOVES = 256;
const int MAX_DEPTH = 6;

pair<Move, double> SAkuna::alphabeta(Board board, int depth=0, double alpha=-numeric_limits<double>::infinity(), double beta=numeric_limits<double>::infinity()) {
    if(depth == MAX_DEPTH) {
        ++nb_states;
        return {Move(-1,-1,-1,-1), board.eval()};
    }
    Move moveList[MAX_MOVES];
    Move *endMoves = board.moves(moveList);
    int nbMoves = endMoves - moveList;
    if(nbMoves == 0) {
        return {Move(-1, -1, -1, -1), board.in_check(board.player) ? (MAX_DEPTH-depth) * -1000000 : 0};
    }
    vector<pair<double, int>> moves;
    Board newBd;
    //random_shuffle(moveList, endMoves);
    for(int i = 0; i < endMoves-moveList; ++i) {
        board.do_move(moveList[i], &newBd);
        moves.push_back({newBd.eval(), i});
        //moves.push_back({0, i});
    }
    sort(moves.begin(), moves.end(), [] (const pair<double, int> &a, const pair<double, int> &b) -> bool {
            return a.first < b.first;
    });
    Move *bestMove = moveList;
    double bestScore = -numeric_limits<double>::infinity();
    for(auto m : moves) {
        board.do_move(moveList[m.second], &newBd);
        double score = -alphabeta(newBd, depth+1, -beta, -alpha).second;
        if(score > bestScore) {
            bestScore = score;
            bestMove = &moveList[m.second];
        }
        alpha = max(alpha, bestScore);
        if(alpha >= beta)
            break;
    }
    return {*bestMove, bestScore};
}

void SAkuna::start_search() {
    nb_states = 0;
    pair<Move, double> result = alphabeta(board);
    fprintf(stderr, "%d\n", nb_states);
    printf("bestmove %s\n", result.first.toString().c_str());
    printf("info depth %d score %d\n", MAX_DEPTH, (int)result.second);
}

int SAkuna::perft(Board board, int depth) {
    if(depth < 1) return 1;
    Move moveList[MAX_MOVES];
    Move* endMoves = board.moves(moveList);
    if(depth == 1) return endMoves - moveList;
    int nb_moves = 0;
    Board newBd;
    for(Move* m = moveList; m < endMoves; ++m) {
        board.do_move(*m, &newBd);
        nb_moves += perft(newBd, depth-1);
    }
    return nb_moves;
}

void SAkuna::divide(int depth) {
    Move moveList[MAX_MOVES];
    Move* endMoves = board.moves(moveList);
    Board newBd;
    int tot = 0, cur;
    for(Move* m = moveList; m < endMoves; ++m) {
        board.do_move(*m, &newBd);
        cur = perft(newBd, depth-1);
        fprintf(stderr, "%s: %d\n", m->toString().c_str(), cur);
        tot += cur;
    }
    fprintf(stderr, "%d\n", tot);
}

SAkuna::~SAkuna() {
}





