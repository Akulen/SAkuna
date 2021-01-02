#include "sakuna.hpp"

#include <chrono>
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
    fprintf(stderr, "%d => %s\n", (int)moves.size(), fen.c_str());
    repetition.clear();
    vector<string> sub_moves;
    for(int i = 0; i <= (int)moves.size(); ++i) {
        board = Board(fen, sub_moves);
        if(repetition.count(board) == 0)
            repetition[board] = 0;
        ++repetition[board];
        if(i < (int)moves.size())
            sub_moves.push_back(moves[i]);
    }
}

// check if player is in check
bool SAkuna::valid(Move move) {
    Board newBd;
    board.do_move(move, &newBd);
    return !newBd.in_check(board.player);
}

const int MAX_MOVES = 256;

pair<Move, double> SAkuna::alphabeta(Board board, int max_depth, int depth=0, double alpha=-numeric_limits<double>::infinity(), double beta=numeric_limits<double>::infinity()) {
    Move moveList[MAX_MOVES];
    Move *endMoves = board.moves(moveList);
    int nbMoves = endMoves - moveList;
    if(nbMoves == 0) {
        return {Move(-1, -1, -1, -1), board.in_check(board.player) ? (depth+1) * -1'000'000'000LL : 0};
    }
    if(board.halfmove_clock == 50)
        return {Move(-1, -1, -1, -1), 0};
    if(repetition.count(board) && repetition[board] > 2)
        return {Move(-1, -1, -1, -1), 0};
    if(depth == max_depth) {
        ++nb_states;
        return {Move(-1,-1,-1,-1), board.eval()};
    }
    if(transposition.count(board) && transposition[board].first >= max_depth-depth) {
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
    if(depth == max_depth-1) {
        random_shuffle(moveList+skipFirst, endMoves);
        for(int i = 0; i < endMoves-moveList; ++i) {
            moves[i] = {0, i};
        }
    } else {
        for(int i = 1; i < endMoves-moveList; ++i) {
            board.do_move(moveList[i], &newBd);
            if(transposition.count(newBd) == 0)
                transposition[newBd] = {0, {Move(-1, -1, -1, -1), newBd.eval()}};
            moves[i] = {transposition[newBd].second.second, i};
        }
        sort(moves.begin()+skipFirst, moves.end(), [] (const pair<double, int> &a, const pair<double, int> &b) -> bool {
                return a.first < b.first;
        });
    }
    Move *bestMove = moveList;
    double bestScore = -numeric_limits<double>::infinity();
    for(auto m : moves) {
        board.do_move(moveList[m.second], &newBd);
        if(repetition.count(newBd) == 0)
            repetition[newBd] = 0;
        ++repetition[newBd];
        double score = -alphabeta(newBd, max_depth, depth+1, -beta, -alpha).second;
        --repetition[newBd];
        if(score > bestScore) {
            bestScore = score;
            bestMove = &moveList[m.second];
        }
        alpha = max(alpha, bestScore);
        if(alpha >= beta)
            break;
    }
    transposition[board] = {max_depth-depth, {*bestMove, bestScore}};
    return {*bestMove, bestScore};
}

void SAkuna::start_search(int wtime, int btime) {
    transposition.clear();
    pair<Move, double> bestResult =
        {Move(-1, -1, -1, -1), -numeric_limits<double>::infinity()};
    auto start_time = chrono::steady_clock::now();
    auto cur_time = chrono::steady_clock::now();
    nb_states = 0;
    if(board.player) swap(wtime, btime);
    int max_depth;
    for(max_depth = 2; chrono::duration_cast<chrono::milliseconds>(cur_time-start_time).count() < wtime/200; max_depth+=2) {
        pair<Move, double> result = alphabeta(board, max_depth);
        cur_time = chrono::steady_clock::now();
        if(abs(result.second) < 1'000'000'000LL)
            printf("info depth %d score cp %lld nodes %d nps %d\n", max_depth, (long long int)result.second, nb_states, (int)((double)nb_states / chrono::duration_cast<chrono::nanoseconds>(cur_time-start_time).count()*1'000'000'000));
        else
            printf("info depth %d score mate %lld nodes %d nps %d\n", max_depth, ((long long int)result.second / 1'000'000'000 + 1 + (board.player ? 1 : 0))/2, nb_states, (int)((double)nb_states / chrono::duration_cast<chrono::nanoseconds>(cur_time-start_time).count()*1'000'000'000));
        if(result.second >= bestResult.second)
            bestResult = result;
    }
    max_depth -= 2;
    vector<Move> pv;
    Move cur_move = bestResult.first;
    pv.push_back(cur_move);
    Board newBd, curBd = board;
    for(int i = 0; i < max_depth; ++i) {
        curBd.do_move(cur_move, &newBd);
        if(transposition.count(newBd)) {
            cur_move = transposition[newBd].second.first;
            pv.push_back(cur_move);
        } else {
            break;
        }
        curBd = newBd;
    }
    if(abs(bestResult.second) < 1'000'000'000LL)
        printf("info depth %d score cp %lld nodes %d nps %d", max_depth, (long long int)bestResult.second, nb_states, (int)((double)nb_states / chrono::duration_cast<chrono::nanoseconds>(cur_time-start_time).count()*1'000'000'000));
    else
        printf("info depth %d score mate %lld nodes %d nps %d", max_depth, ((long long int)bestResult.second / 1'000'000'000 + 1 + (board.player ? 1 : 0))/2, nb_states, (int)((double)nb_states / chrono::duration_cast<chrono::nanoseconds>(cur_time-start_time).count()*1'000'000'000));
    printf(" pv");
    for(auto m : pv)
        printf(" %s", m.toString().c_str());
    printf("\n");
    fprintf(stderr, "%d\n", (int)board.player);
    printf("bestmove %s\n", bestResult.first.toString().c_str());

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





