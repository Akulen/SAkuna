#include "piece.hpp"

#include <cassert>
#include <cmath>
#include <vector>

using namespace std;

void Piece::move(int r, int c, vector<vector<Piece*>> &board) {
    delete board[r][c];
    board[r][c] = this;
    board[row][col] = new Clear(row, col);
    row = r; col = c;
}

void Piece::shadow_move(int r, int c, int, std::vector<std::vector<Piece*>> &board) {
    memory.push_back(board[r][c]);
    board[r][c] = this;
    board[row][col] = new Clear(row, col);
    row = r; col = c;
}

void Piece::shadow_unmove(int r, int c, int halfmove_clock, std::vector<std::vector<Piece*>> &board) {
    if(halfmove_clock == promotion.first && promotion.second != NULL) {
        board[row][col] = promotion.second;
        board[row][col]->shadow_unmove(r, c, halfmove_clock, board);
        delete this;
        return;
    }
    delete board[r][c];
    board[r][c] = board[row][col];
    board[row][col] = memory.back();
    memory.pop_back();
    row = r; col = c;
}

void Piece::shadow_promote(int, int, int halfmove_clock, Piece* pawn, std::vector<std::vector<Piece*>>&) {
    promotion = {halfmove_clock, pawn};
}

pair<char, unsigned long long int> Piece::moves(vector<vector<Piece*>>&, char, char) {
    assert(false);
}

bool Piece::can_take(Piece* p, std::vector<std::vector<Piece*>> &board, char en_passant) {
    unsigned long long int mvs = moves(board, 0, en_passant).second;
    for(int i = 0; i < 64; ++i) if(mvs & (1LL << i))
        if(p->col == i%8 && p->row == i/8)
            return true;
    return false;
}

void Clear::shadow_move(int, int, int, std::vector<std::vector<Piece*>>&) {
    assert(false);
}

void Clear::shadow_unmove(int, int, int, std::vector<std::vector<Piece*>>&) {
    assert(false);
}

Piece::~Piece() {
    while(memory.size()) {
        delete memory.back();
        memory.pop_back();
    }
    if(promotion.second != NULL)
        delete promotion.second;
}

void Pawn::move(int r, int c, vector<vector<Piece*>> &board) {
    if(col != c && board[r][c]->pt == clear) {
        delete board[4-player][c];
        board[4-player][c] = new Clear(4-player, c);
    }
    Piece::move(r, c, board);
    // promotion : toujours dame pour l'instant TODO
    if(r == 7-7*player) {
        switch('q') {
            case 'n':
                board[row][col] = new Knight(row, col, player);
                break;
            case 'b':
                board[row][col] = new Bishop(row, col, player);
                break;
            case 'r':
                board[row][col] = new Rook(row, col, player);
                break;
            case 'q':
                board[row][col] = new Queen(row, col, player);
                break;
        };
        delete this;
    }
}

void Pawn::shadow_move(int r, int c, int halfmove_clock, std::vector<std::vector<Piece*>> &board) {
    if(col != c && board[r][c]->pt == clear) {
        memory.push_back(board[4-player][c]);
        board[4-player][c] = new Clear(4-player, c);
    }
    Piece::shadow_move(r, c, halfmove_clock, board);
    if(r == 7-7*player) {
        switch('q') {
            case 'n':
                board[row][col] = new Knight(row, col, player);
                break;
            case 'b':
                board[row][col] = new Bishop(row, col, player);
                break;
            case 'r':
                board[row][col] = new Rook(row, col, player);
                break;
            case 'q':
                board[row][col] = new Queen(row, col, player);
                break;
        };
        board[row][col]->shadow_promote(r, c, halfmove_clock, this, board);
    }
}

void Pawn::shadow_unmove(int r, int c, int halfmove_clock, std::vector<std::vector<Piece*>> &board) {
    int r0 = row, c0 = col;
    Piece::shadow_unmove(r, c, halfmove_clock, board);
    if(c0 != c && board[r0][c0]->pt == clear) {
        delete board[4-player][c0];
        board[4-player][c0] = memory.back();
        memory.pop_back();
    }
}

pair<char, unsigned long long int> Pawn::moves(vector<vector<Piece*>> &board, char, char en_passant) {
    pair<char, unsigned long long int> ans = {8*row+col, 0};
    int front = row + (player == 1 ? -1 : 1);
    if(col > 0 && (board[front][col-1]->player == !player || (en_passant != -1 && row == 4-player && en_passant % 8 == 5-3*player && en_passant / 8 == col-1)))
        ans.second |= (1LL << (8LL*front+col-1));
    if(col < 7 && (board[front][col+1]->player == !player || (en_passant != -1 && row == 4-player && en_passant % 8 == 5-3*player && en_passant / 8 == col+1)))
        ans.second |= (1LL << (8LL*front+col+1));
    if(front >= 0 && front < 8 && board[front][col]->pt == clear) {
        ans.second |= (1LL << (8LL*front+col));
        if((row == 1 && player == 0) || (row == 6 && player == 1)) {
            front += (player == 1 ? -1 : 1);
            if(board[front][col]->pt == clear)
                ans.second |= (1LL << (8LL*front+col));
        }
    }
    return ans;
}

bool Pawn::can_take(Piece* p, std::vector<std::vector<Piece*>>&, char en_passant) {
    int front = row + (player == 1 ? -1 : 1);
    if(abs(p->col - col) != 1) return false;
    if(front == p->row) return true;
    if(p->pt == pawn && en_passant/8 == p->col && p->row + (player == 1 ? -1 : 1) == en_passant%8)
        return true;
    return false;
}

const pair<int, int> KNIGHT_MOVES[8] = {{1, 2}, {2, 1}, {-1, 2}, {2, -1}, {1, -2}, {-2, 1}, {-1, -2}, {-2, -1}};

pair<char, unsigned long long int> Knight::moves(vector<vector<Piece*>> &board, char, char) {
    pair<char, unsigned long long int> ans = {8*row+col, 0};
    int r, c;
    for(int dir = 0; dir < 8; ++dir) {
        r = row + KNIGHT_MOVES[dir].first;
        c = col + KNIGHT_MOVES[dir].second;
        if(r < 0 || c < 0 || r > 7 || c > 7) continue;
        if(board[r][c]->player != player)
            ans.second |= (1LL << (8LL*r+c));
    }
    return ans;
}

bool Knight::can_take(Piece* p, std::vector<std::vector<Piece*>>&, char) {
    int r, c;
    for(int dir = 0; dir < 8; ++dir) {
        r = row + KNIGHT_MOVES[dir].first;
        if(r != p->row) continue;
        c = col + KNIGHT_MOVES[dir].second;
        if(c == p->col) return true;
    }
    return false;
}

const pair<int, int> LINEAR_MOVES[8] = {{1, 1}, {1, -1}, {-1, -1}, {-1, 1}, {1, 0}, {0, 1}, {-1, 0}, {0, -1}};

pair<char, unsigned long long int> Bishop::moves(vector<vector<Piece*>> &board, char, char) {
    pair<char, unsigned long long int> ans = {8*row+col, 0};
    int r, c;
    for(int dir = 0; dir < 4; ++dir) {
        r = row + LINEAR_MOVES[dir].first;
        c = col + LINEAR_MOVES[dir].second;
        while(r >= 0 && c >= 0 && r < 8 && c < 8) {
            if(board[r][c]->player == player) break;
            ans.second |= (1LL << (8LL*r+c));
            if(board[r][c]->player == !player) break;
            r += LINEAR_MOVES[dir].first;
            c += LINEAR_MOVES[dir].second;
        }
    }
    return ans;
}

bool Bishop::can_take(Piece* p, std::vector<std::vector<Piece*>> &board, char) {
    int r, c;
    for(int dir = 0; dir < 4; ++dir) {
        if(LINEAR_MOVES[dir].second * (row-p->row) != LINEAR_MOVES[dir].first * (col-p->col))
            continue;
        r = row + LINEAR_MOVES[dir].first;
        c = col + LINEAR_MOVES[dir].second;
        while(r >= 0 && c >= 0 && r < 8 && c < 8) {
            if(r == p->row && c == p->col) return true;
            if(board[r][c]->player != -1) break;
            r += LINEAR_MOVES[dir].first;
            c += LINEAR_MOVES[dir].second;
        }
    }
    return false;
}

pair<char, unsigned long long int> Rook::moves(vector<vector<Piece*>> &board, char, char) {
    pair<char, unsigned long long int> ans = {8*row+col, 0};
    int r, c;
    for(int dir = 4; dir < 8; ++dir) {
        r = row + LINEAR_MOVES[dir].first;
        c = col + LINEAR_MOVES[dir].second;
        while(r >= 0 && c >= 0 && r < 8 && c < 8) {
            if(board[r][c]->player == player) break;
            ans.second |= (1LL << (8LL*r+c));
            if(board[r][c]->player == !player) break;
            r += LINEAR_MOVES[dir].first;
            c += LINEAR_MOVES[dir].second;
        }
    }
    return ans;
}

bool Rook::can_take(Piece* p, std::vector<std::vector<Piece*>> &board, char) {
    int r, c;
    for(int dir = 4; dir < 8; ++dir) {
        if(LINEAR_MOVES[dir].second * (row-p->row) != LINEAR_MOVES[dir].first * (col-p->col))
            continue;
        r = row + LINEAR_MOVES[dir].first;
        c = col + LINEAR_MOVES[dir].second;
        while(r >= 0 && c >= 0 && r < 8 && c < 8) {
            if(r == p->row && c == p->col) return true;
            if(board[r][c]->player != -1) break;
            r += LINEAR_MOVES[dir].first;
            c += LINEAR_MOVES[dir].second;
        }
    }
    return false;
}

pair<char, unsigned long long int> Queen::moves(vector<vector<Piece*>> &board, char, char) {
    pair<char, unsigned long long int> ans = {8*row+col, 0};
    int r, c;
    for(int dir = 0; dir < 8; ++dir) {
        r = row + LINEAR_MOVES[dir].first;
        c = col + LINEAR_MOVES[dir].second;
        while(r >= 0 && c >= 0 && r < 8 && c < 8) {
            if(board[r][c]->player == player) break;
            ans.second |= (1LL << (8LL*r+c));
            if(board[r][c]->player == !player) break;
            r += LINEAR_MOVES[dir].first;
            c += LINEAR_MOVES[dir].second;
        }
    }
    return ans;
}

bool Queen::can_take(Piece* p, std::vector<std::vector<Piece*>> &board, char) {
    int r, c;
    for(int dir = 0; dir < 8; ++dir) {
        if(LINEAR_MOVES[dir].second * (row-p->row) != LINEAR_MOVES[dir].first * (col-p->col))
            continue;
        r = row + LINEAR_MOVES[dir].first;
        c = col + LINEAR_MOVES[dir].second;
        while(r >= 0 && c >= 0 && r < 8 && c < 8) {
            if(r == p->row && c == p->col) return true;
            if(board[r][c]->player != -1) break;
            r += LINEAR_MOVES[dir].first;
            c += LINEAR_MOVES[dir].second;
        }
    }
    return false;
}

void King::move(int r, int c, vector<vector<Piece*>> &board) {
    if(col - c == 2) { // castle queen side
        board[r][0]->move(r, 3, board);
    } else if(c - col == 2) { // castle kind side;
        board[r][7]->move(r, 5, board);
    }
    Piece::move(r, c, board);
}

void King::shadow_move(int r, int c, int halfmove_clock, std::vector<std::vector<Piece*>> &board) {
    if(col - c == 2) { // castle queen side
        board[r][0]->shadow_move(r, 3, halfmove_clock, board);
    } else if(c - col == 2) { // castle king side
        board[r][7]->shadow_move(r, 5, halfmove_clock, board);
    }
    Piece::shadow_move(r, c, halfmove_clock, board);
}

void King::shadow_unmove(int r, int c, int halfmove_clock, std::vector<std::vector<Piece*>> &board) {
    if(col - c == -2) { // castle queen side
        board[r][3]->shadow_move(r, 0, halfmove_clock, board);
    } else if(c - col == -2) { // castle king side
        board[r][5]->shadow_move(r, 7, halfmove_clock, board);
    }
    Piece::shadow_unmove(r, c, halfmove_clock, board);
}

pair<char, unsigned long long int> King::moves(vector<vector<Piece*>> &board, char castling_rights, char) {
    pair<char, unsigned long long int> ans = {8*row+col, 0};
    int r, c;
    for(int dir = 0; dir < 8; ++dir) {
        r = row + LINEAR_MOVES[dir].first;
        c = col + LINEAR_MOVES[dir].second;
        if(r < 0 || c < 0 || r > 7 || c > 7) continue;
        if(board[r][c]->player != player)
            ans.second |= (1LL << (8LL*r+c));
    }
    bool king_attacked = false;
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            if(board[i][j]->player == !player && board[i][j]->can_take(this, board, -1)) {
                king_attacked = true;
                goto outer;
            }
outer:
    if(!king_attacked) {
        if((castling_rights >> (2*player)) & 1 && board[row][5]->pt == clear && board[row][6]->pt == clear) {
            bool can_castle = true;
            for(int i = 0; i < 8; ++i)
                for(int j = 0; j < 8; ++j)
                    if(board[i][j]->player == !player)
                        for(int spot = 5; spot < 7; ++spot)
                            if(board[i][j]->can_take(board[row][spot], board, -1)) {
                            can_castle = false;
                            goto outer_castle_king;
                        }
outer_castle_king:
            if(can_castle)
                ans.second |= (1LL << (8LL*row+6));
        }
        if((castling_rights >> (2*player)) & 2 && board[row][3]->pt == clear && board[row][2]->pt == clear && board[row][1]->pt == clear) {
            bool can_castle = true;
            for(int i = 0; i < 8; ++i)
                for(int j = 0; j < 8; ++j)
                    if(board[i][j]->player == !player)
                        for(int spot = 2; spot < 4; ++spot)
                            if(board[i][j]->can_take(board[row][spot], board, -1)) {
                            can_castle = false;
                            goto outer_castle_queen;
                        }
outer_castle_queen:
            if(can_castle)
                ans.second |= (1LL << (8LL*row+2));
        }
    }
    return ans;
}

bool King::can_take(Piece* p, std::vector<std::vector<Piece*>>&, char) {
    return max(abs(row-p->row), abs(col-p->col)) < 2;
}
