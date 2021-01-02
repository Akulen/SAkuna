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
    srand(42);
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
const int MAX_DEPTH = 8;

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
    if(board.halfmove_clock == 50)
        return {Move(-1, -1, -1, -1), 0};
    if(transposition.count(board) && transposition[board].first <= depth) {
        return transposition[board].second;
    }
    vector<pair<double, int>> moves(endMoves-moveList);
    Board newBd;
    bool skipFirst = false;
    if(transposition.count(board) && transposition[board].second.first.r0 != -1) {
        for(int i = 0; i < endMoves-moveList; ++i) {
            if(moveList[i] == transposition[board].second.first) {
                swap(moveList[0], moveList[i]);
                skipFirst = true;
                break;
            }
        }
    }
    if(depth == MAX_DEPTH-1) {
        random_shuffle(moveList+skipFirst, endMoves);
        for(int i = 0; i < endMoves-moveList; ++i) {
            moves[i] = {0, i};
        }
    } else {
        for(int i = 1; i < endMoves-moveList; ++i) {
            board.do_move(moveList[i], &newBd);
            moves[i] = {newBd.eval(), i};
        }
        sort(moves.begin()+skipFirst, moves.end(), [] (const pair<double, int> &a, const pair<double, int> &b) -> bool {
                return a.first < b.first;
        });
    }
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
    transposition[board] = {depth, {*bestMove, bestScore}};
    return {*bestMove, bestScore};
}

void SAkuna::start_search() {
    nb_states = 0;
    pair<Move, double> result = alphabeta(board);
    //fprintf(stderr, "%d\n", nb_states);
    printf("bestmove %s\n", result.first.toString().c_str());
    printf("info depth %d score cp %d nodes %d\n", MAX_DEPTH, (int)result.second, nb_states);

    //// count collisions
    //size_t collisions = 0, empty = 0;
    //for (auto bucket = transposition.bucket_count(); bucket--;) {
    //    if (transposition.bucket_size(bucket) == 0)
    //        empty++;
    //    else
    //        collisions += transposition.bucket_size(bucket) - 1;
    //}
    //fprintf(stderr, "collisions = %f %f %lu %lu %lu\n",
    //        transposition.max_load_factor(), transposition.load_factor(),
    //        transposition.bucket_count(), collisions, empty);
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





