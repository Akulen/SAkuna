#ifndef BOARD_HPP_
#define BOARD_HPP_

#include <cstdio>
#include <string>
#include <vector>

#include "move.hpp"

typedef unsigned long long int Bitboard;

enum Square : int {
  SQ_A1, SQ_B1, SQ_C1, SQ_D1, SQ_E1, SQ_F1, SQ_G1, SQ_H1,
  SQ_A2, SQ_B2, SQ_C2, SQ_D2, SQ_E2, SQ_F2, SQ_G2, SQ_H2,
  SQ_A3, SQ_B3, SQ_C3, SQ_D3, SQ_E3, SQ_F3, SQ_G3, SQ_H3,
  SQ_A4, SQ_B4, SQ_C4, SQ_D4, SQ_E4, SQ_F4, SQ_G4, SQ_H4,
  SQ_A5, SQ_B5, SQ_C5, SQ_D5, SQ_E5, SQ_F5, SQ_G5, SQ_H5,
  SQ_A6, SQ_B6, SQ_C6, SQ_D6, SQ_E6, SQ_F6, SQ_G6, SQ_H6,
  SQ_A7, SQ_B7, SQ_C7, SQ_D7, SQ_E7, SQ_F7, SQ_G7, SQ_H7,
  SQ_A8, SQ_B8, SQ_C8, SQ_D8, SQ_E8, SQ_F8, SQ_G8, SQ_H8,
  SQ_NONE,

  SQUARE_ZERO = 0,
  SQUARE_NB   = 64
};

enum Piece_Type :int {
    pt_pawn, pt_knight, pt_bishop, pt_rook, pt_queen, pt_king, pt_empty
};

void lookup_table_init();

class Board {
    // raw data
    Bitboard pieces[2][6];
    char castling_rights;
    Square en_passant;
    int halfmove_clock;
    // derived data
    bool init_done;
    Bitboard checkers, blockers;
    public:
    Bitboard allPieces[3];
    bool player;
    int fullmove_number;
    Board();
    Board(const std::string&, const std::vector<std::string>&);
    void init();
    Bitboard compute_king_incomplete(Bitboard, Bitboard) const;
    Bitboard compute_knight(Bitboard, Bitboard) const;
    Bitboard compute_pawn(Bitboard, int) const;
    Move* moves(Move*);
    Bitboard attacks(Square, Piece_Type, int player = -1) const;
    Bitboard attacks_empty(Square, Piece_Type, int player = -1);
    Bitboard attacks_on(Square) const;
    Move* make_moves(Move*, Piece_Type);
    bool legal(Move) const;
    Piece_Type piece_on(Square) const;
    void do_move(Move, Board*);
    bool in_check(int) const;
    void display() const;
    double eval() const;
    ~Board() {};
};

#endif
