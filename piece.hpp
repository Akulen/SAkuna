#ifndef PIECE_HPP_
#define PIECE_HPP_

#include <vector>

enum PieceType { clear, pawn, knight, bishop, rook, queen, king };

class Piece;

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
    virtual void shadow_move(int, int, int, std::vector<std::vector<Piece*>>&);
    virtual void shadow_unmove(int, int, int, std::vector<std::vector<Piece*>>&);
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
};

class Knight : public virtual Piece {
    public:
    Knight(int r, int c, int p) :
        Piece(r, c, p, knight) {}
    virtual std::pair<char, unsigned long long int> moves(std::vector<std::vector<Piece*>>&, char, char);
    virtual bool can_take(Piece*, std::vector<std::vector<Piece*>>&, char);
};

class Bishop : public virtual Piece {
    public:
    Bishop(int r, int c, int p) :
        Piece(r, c, p, bishop) {}
    virtual std::pair<char, unsigned long long int> moves(std::vector<std::vector<Piece*>>&, char, char);
    virtual bool can_take(Piece*, std::vector<std::vector<Piece*>>&, char);
};

class Rook : public virtual Piece {
    public:
    Rook(int r, int c, int p) :
        Piece(r, c, p, rook) {}
    virtual std::pair<char, unsigned long long int> moves(std::vector<std::vector<Piece*>>&, char, char);
    virtual bool can_take(Piece*, std::vector<std::vector<Piece*>>&, char);
};

class Queen : public virtual Piece {
    public:
    Queen(int r, int c, int p) :
        Piece(r, c, p, queen) {}
    virtual std::pair<char, unsigned long long int> moves(std::vector<std::vector<Piece*>>&, char, char);
    virtual bool can_take(Piece*, std::vector<std::vector<Piece*>>&, char);
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
};

#endif
