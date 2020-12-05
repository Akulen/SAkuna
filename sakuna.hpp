#ifndef SAKUNA_HPP_
#define SAKUNA_HPP_

#include <string>
#include <vector>

#include "uci.hpp"

enum PieceType { clear, pawn, knight, bishop, rook, queen, king };

class Piece;

struct Move {
    int r0, c0, r1, c1;
    Move(int r0, int c0, int r1, int c1) : r0(r0), c0(c0), r1(r1), c1(c1) {}
    std::string toString(std::vector<std::vector<Piece*>>&);
};

class Piece {
    public:
    int row, col, player;
    PieceType pt;
    std::vector<Piece*> memory;
    std::pair<int, Piece*> promotion;
    Piece(int r, int c, int p, PieceType pt) :
        row(r), col(c), player(p), pt(pt) {}
    virtual void move(int, int, std::vector<std::vector<Piece*>>&);
    virtual void shadow_move(int, int, int, std::vector<std::vector<Piece*>>&);
    virtual void shadow_unmove(int, int, int, std::vector<std::vector<Piece*>>&);
    virtual void shadow_promote(int, int, int, Piece*, std::vector<std::vector<Piece*>>&);
    virtual std::pair<char, unsigned long long int> moves(std::vector<std::vector<Piece*>>&, char, char);
    virtual bool can_take(Piece*, std::vector<std::vector<Piece*>>&, char);
    virtual ~Piece();
};

class Clear : public virtual Piece {
    public:
    Clear(int r, int c) :
        Piece(r, c, -1, clear) {}
    virtual ~Clear();
};

class Pawn : public virtual Piece {
    public:
    Pawn(int r, int c, int p) :
        Piece(r, c, p, pawn) {}
    virtual void move(int, int, std::vector<std::vector<Piece*>>&);
    virtual void shadow_move(int, int, int, std::vector<std::vector<Piece*>>&);
    virtual void shadow_unmove(int, int, int, std::vector<std::vector<Piece*>>&);
    virtual std::pair<char, unsigned long long int> moves(std::vector<std::vector<Piece*>>&, char, char);
    virtual bool can_take(Piece*, std::vector<std::vector<Piece*>>&, char);
    virtual ~Pawn();
};

class Knight : public virtual Piece {
    public:
    Knight(int r, int c, int p) :
        Piece(r, c, p, knight) {}
    virtual std::pair<char, unsigned long long int> moves(std::vector<std::vector<Piece*>>&, char, char);
    virtual bool can_take(Piece*, std::vector<std::vector<Piece*>>&, char);
    virtual ~Knight();
};

class Bishop : public virtual Piece {
    public:
    Bishop(int r, int c, int p) :
        Piece(r, c, p, bishop) {}
    virtual std::pair<char, unsigned long long int> moves(std::vector<std::vector<Piece*>>&, char, char);
    virtual bool can_take(Piece*, std::vector<std::vector<Piece*>>&, char);
    virtual ~Bishop();
};

class Rook : public virtual Piece {
    public:
    Rook(int r, int c, int p) :
        Piece(r, c, p, rook) {}
    virtual std::pair<char, unsigned long long int> moves(std::vector<std::vector<Piece*>>&, char, char);
    virtual bool can_take(Piece*, std::vector<std::vector<Piece*>>&, char);
    virtual ~Rook();
};

class Queen : public virtual Piece {
    public:
    Queen(int r, int c, int p) :
        Piece(r, c, p, queen) {}
    virtual std::pair<char, unsigned long long int> moves(std::vector<std::vector<Piece*>>&, char, char);
    virtual bool can_take(Piece*, std::vector<std::vector<Piece*>>&, char);
    virtual ~Queen();
};

class King : public virtual Piece {
    public:
    King(int r, int c, int p) :
        Piece(r, c, p, king) {}
    virtual void move(int, int, std::vector<std::vector<Piece*>>&);
    virtual void shadow_move(int, int, int, std::vector<std::vector<Piece*>>&);
    virtual void shadow_unmove(int, int, int, std::vector<std::vector<Piece*>>&);
    virtual std::pair<char, unsigned long long int> moves(std::vector<std::vector<Piece*>>&, char, char);
    virtual bool can_take(Piece*, std::vector<std::vector<Piece*>>&, char);
    virtual ~King();
};

class SAkuna {
    uci &u;
    std::vector<std::vector<Piece*>> board;
    std::vector<std::vector<Piece*>> material;
    int player;
    char castling_rights;
    char en_passant;
    int halfmove_clock;
    int fullmove_number;
    int nb_states;
    public:
    SAkuna(uci&);
    void init();
    void set_position(const std::string&, const std::vector<std::string>&);
    bool check();
    bool valid(Move);
    double eval();
    std::pair<Move, double> alphabeta(int, double, double);
    void start_search();
    void display_board();
};

#endif
