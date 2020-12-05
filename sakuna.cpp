#include "sakuna.hpp"

#include <cmath>
#include <cstdio>
#include <utility>

using namespace std;

void display_bitmove(unsigned long long int moves) {
    fprintf(stderr, "--------\n%llu\n", moves);
    for(int i = 0; i < 8; ++i) {
        for(int j = 0; j < 8; ++j)
            fprintf(stderr, "%c ", (moves & (1LL << (8*i+j))) ? 'X' : '.');
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "--------\n");
}

string Move::toString(vector<vector<Piece*>> &board) {
    string ans = "";
    ans += c0+'a';
    ans += r0+'1';
    ans += c1+'a';
    ans += r1+'1';
    if(board[r0][c0]->pt == pawn && r1 == 7-7*board[r0][c0]->player)
        ans += 'q';
    return ans;
}

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

Piece::~Piece() {}

Clear::~Clear() {}

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

Pawn::~Pawn() {}

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

Knight::~Knight() {}

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

Bishop::~Bishop() {}

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

Rook::~Rook() {}

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

Queen::~Queen() {}

void King::move(int r, int c, vector<vector<Piece*>> &board) {
    if(col - c == 2) { // castle queen side
        board[r][0]->move(r, 3, board);
    } else if(c - col == 2) { // castle kind side;
        board[r][7]->move(r, 5, board);
    }
    Piece::move(r, c, board);
}
/*
void Pawn::shadow_move(int r, int c, std::vector<std::vector<Piece*>> &board) {
    if(col != c && board[r][c]->pt == clear) {
        memory.push_back(board[4-player][c]);
        board[4-player][c] = new Clear(4-player, c);
    }
    Piece::shadow_move(r, c, board);
}

void Pawn::shadow_unmove(int r, int c, std::vector<std::vector<Piece*>> &board) {
    int r0 = row, c0 = col;
    Piece::shadow_unmove(r, c, board);
    if(c0 != c && board[r0][c0]->pt == clear) {
        delete board[4-player][c0];
        board[4-player][c0] = memory.back();
        memory.pop_back();
    }
}*/

// TODO: handle castling undo
void King::shadow_move(int r, int c, int, std::vector<std::vector<Piece*>> &board) {
    memory.push_back(board[r][c]);
    board[r][c] = this;
    board[row][col] = new Clear(row, col);
    row = r; col = c;
}

void King::shadow_unmove(int r, int c, int, std::vector<std::vector<Piece*>> &board) {
    delete board[r][c];
    board[r][c] = board[row][col];
    board[row][col] = memory.back();
    memory.pop_back();
    row = r; col = c;
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

bool King::can_take(Piece* p, std::vector<std::vector<Piece*>> &board, char) {
    if(pt == king && p->pt == king) {
        return max(abs(row-p->row), abs(col-p->col)) < 2;
    }
    return Piece::can_take(p, board, -1);
}

King::~King() {}

SAkuna::SAkuna(uci &_u) : u(_u) {
    material.resize(2);
}

void SAkuna::init() {
    srand(time(NULL));
    if(board.empty()) {
        board.resize(8);
        for(int i = 0; i < 8; ++i)
            for(int j = 0; j < 8; ++j)
                board[i].push_back(0);
    }
}

void SAkuna::set_position(const string& fen, const vector<string>& moves) {
    //fprintf(stderr, "%s\n", fen.c_str());
    material[0].clear(); material[1].clear();
    int row = 7, c = 0, col = 0;
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            delete board[i][j];
    while(fen[c] != ' ') {
        switch(fen[c]) {
            case '/':
                --row;
                col = -1;
                break;
            case 'p':
                board[row][col] = new Pawn(row, col, 1);
                break;
            case 'n':
                board[row][col] = new Knight(row, col, 1);
                break;
            case 'b':
                board[row][col] = new Bishop(row, col, 1);
                break;
            case 'r':
                board[row][col] = new Rook(row, col, 1);
                break;
            case 'q':
                board[row][col] = new Queen(row, col, 1);
                break;
            case 'k':
                board[row][col] = new King(row, col, 1);
                break;
            case 'P':
                board[row][col] = new Pawn(row, col, 0);
                break;
            case 'N':
                board[row][col] = new Knight(row, col, 0);
                break;
            case 'B':
                board[row][col] = new Bishop(row, col, 0);
                break;
            case 'R':
                board[row][col] = new Rook(row, col, 0);
                break;
            case 'Q':
                board[row][col] = new Queen(row, col, 0);
                break;
            case 'K':
                board[row][col] = new King(row, col, 0);
                break;
            default:
                for(int i = col; i < col + fen[c] - '0'; ++i)
                    board[row][i] = new Clear(row, i);
                col += fen[c] - '1';
        }
        ++col; ++c;
    }
    Piece *a = new Pawn(0, 2, 1);
    delete a;
    a = 0;
    //display_board();
    bool white_to_play = fen[c+1] == 'w';
    player = !white_to_play;
    castling_rights = 0;
    c += 3;
    while(c < (int)fen.size() && fen[c] != ' ') {
        switch(fen[c]) {
            case 'K':
                castling_rights |= 1;
                break;
            case 'Q':
                castling_rights |= 2;
                break;
            case 'k':
                castling_rights |= 4;
                break;
            case 'q':
                castling_rights |= 8;
                break;
        };
        ++c;
    }
    ++c;
    en_passant = 0;
    while(c < (int)fen.size() && fen[c] != ' ') {
        if(fen[c] >= '1' && fen[c] <= '8')
            en_passant += fen[c] - '1';
        if(fen[c] >= 'a' && fen[c] <= 'h')
            en_passant += 8*(fen[c] - 'a');
        if(fen[c] == '-')
            en_passant = -1;
        ++c;
    }
    ++c;
    halfmove_clock = 0;
    while(c < (int)fen.size() && fen[c] != ' ') {
        halfmove_clock = halfmove_clock*10+fen[c]-'0';
        ++c;
    }
    ++c;
    fullmove_number = 0;
    while(c < (int)fen.size() && fen[c] != ' ') {
        fullmove_number = fullmove_number*10+fen[c]-'0';
        ++c;
    }
    for(auto move : moves) {
        //display_board();
        int c0 = move[0]-'a';
        int r0 = move[1]-'1';
        int c1 = move[2]-'a';
        int r1 = move[3]-'1';
        // handle castling rights
        char new_castling_rights = castling_rights;
        if((c0 == 0 || c0 == 7) && (r0 == 0 || r0 == 7))
            new_castling_rights &= 15 ^ (1 << (c0/7 + 2*(r0/7)));
        if((c1 == 0 || c1 == 7) && (r1 == 0 || r1 == 7))
            new_castling_rights &= 15 ^ (1 << (c1/7 + 2*(r1/7)));
        if(c0 == 4 && r0 == 0)
            new_castling_rights &= 12;
        if(c0 == 4 && r0 == 7)
            new_castling_rights &= 3;
        // handle en passant
        char new_en_passant;
        if(board[r0][c0]->pt == pawn && r0 == 1+5*player && r1 == 3+player)
            new_en_passant = 2+3*player + 8*c0;
        else
            new_en_passant = -1;
        board[r0][c0]->move(r1, c1, board);
        castling_rights = new_castling_rights;
        en_passant = new_en_passant;
        player ^= 1;
        //fprintf(stderr, "%s\n", move.c_str());
    }
    halfmove_clock += moves.size();
    fullmove_number += (moves.size()+!white_to_play) / 2;
    //display_board();
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            if(board[i][j]->pt != clear)
                material[board[i][j]->player].push_back(board[i][j]);
}

bool SAkuna::check() {
    Piece* kg;
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            if(board[i][j]->player == player && board[i][j]->pt == king) {
                kg = board[i][j];
                goto found_king;
            }
found_king:
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j) if(board[i][j]->player == !kg->player)
            if(board[i][j]->can_take(kg, board, en_passant)) {
                return true;
            }
    return false;
}

// check if player is in check
bool SAkuna::valid(Move move) {
    // make shadow move
    board[move.r0][move.c0]->shadow_move(move.r1, move.c1, halfmove_clock, board);
    bool ans = check();
    // undo shadow move;
    board[move.r1][move.c1]->shadow_unmove(move.r0, move.c0, halfmove_clock, board);
    return !ans;
}

// Values taken from https://www.chessprogramming.org/Simplified_Evaluation_Function
const double VALUES[7] = {0, 100, 320, 330, 500, 900, 20000};
const double POSITION_TABLE[9][8][8] = {
    {{0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0},
     {0, 0, 0, 0, 0, 0, 0, 0}},
    // pawn middle game
    {{ 0,  0,   0,   0,   0,   0,  0,  0},
     {50, 50,  50,  50,  50,  50, 50, 50},
     {10, 10,  20,  30,  30,  20, 10, 10},
     { 5,  5,  10,  25,  25,  10,  5,  5},
     { 0,  0,   0,  20,  20,   0,  0,  0},
     { 5, -5, -10,   0,   0, -10, -5,  5},
     { 5, 10,  10, -20, -20,  10, 10,  5},
     { 0,  0,   0,   0,   0,   0,  0,  0}},
    // knight
    {{-50, -40, -30, -30, -30, -30, -40, -50},
     {-40, -20,   0,   0,   0,   0, -20, -40},
     {-30,   0,  10,  15,  15,  10,   0, -30},
     {-30,   5,  15,  20,  20,  15,   5, -30},
     {-30,   0,  15,  20,  20,  15,   0, -30},
     {-30,   5,  10,  15,  15,  10,   5, -30},
     {-40, -20,   0,   5,   5,   0, -20, -40},
     {-50, -40, -30, -30, -30, -30, -40, -50}},
    // bishop
    {{-20, -10, -10, -10, -10, -10, -10, -20},
     {-10,   0,   0,   0,   0,   0,   0, -10},
     {-10,   0,   5,  10,  10,   5,   0, -10},
     {-10,   5,   5,  10,  10,   5,   5, -10},
     {-10,   0,  10,  10,  10,  10,   0, -10},
     {-10,  10,  10,  10,  10,  10,  10, -10},
     {-10,   5,   0,   0,   0,   0,   5, -10},
     {-20, -10, -10, -10, -10, -10, -10, -20}},
    // rook
    {{ 0,  0,  0,  0,  0,  0,  0,  0},
     { 5, 10, 10, 10, 10, 10, 10,  5},
     {-5,  0,  0,  0,  0,  0,  0, -5},
     {-5,  0,  0,  0,  0,  0,  0, -5},
     {-5,  0,  0,  0,  0,  0,  0, -5},
     {-5,  0,  0,  0,  0,  0,  0, -5},
     {-5,  0,  0,  0,  0,  0,  0, -5},
     { 0,  0,  0,  5,  5,  0,  0,  0}},
    // queen
    {{-20, -10, -10, -5, -5, -10, -10, -20},
     {-10,   0,   0,  0,  0,   0,   0, -10},
     {-10,   0,   5,  5,  5,   5,   0, -10},
     { -5,   0,   5,  5,  5,   5,   0,  -5},
     {  0,   0,   5,  5,  5,   5,   0,  -5},
     {-10,   5,   5,  5,  5,   5,   0, -10},
     {-10,   0,   5,  0,  0,   0,   0, -10},
     {-20, -10, -10, -5, -5, -10, -10, -20}},
    // king middle game
    {{-30, -40, -40, -50, -50, -40, -40, -30},
     {-30, -40, -40, -50, -50, -40, -40, -30},
     {-30, -40, -40, -50, -50, -40, -40, -30},
     {-30, -40, -40, -50, -50, -40, -40, -30},
     {-20, -30, -30, -40, -40, -30, -30, -20},
     {-10, -20, -20, -20, -20, -20, -20, -10},
     { 20,  20,   0,   0,   0,   0,  20,  20},
     { 20,  30,  10,   0,   0,  10,  30,  20}},
    // pawn end game
    {{ 0,  0,   0,   0,   0,   0,  0,  0},
     {50, 50,  50,  50,  50,  50, 50, 50},
     {10, 10,  20,  30,  30,  20, 10, 10},
     { 5,  5,  10,  25,  25,  10,  5,  5},
     { 0,  0,   0,  20,  20,   0,  0,  0},
     {-5, -5, -10,   0,   0, -10, -5, -5},
     {-5, -5, -10, -20, -20, -10, -5, -5},
     { 0,  0,   0,   0,   0,   0,  0,  0}},
    // king end game
    {{-50, -40, -30, -20, -20, -30, -40, -50},
     {-30, -20, -10,   0,   0, -10, -20, -30},
     {-30, -10,  20,  30,  30,  20, -10, -30},
     {-30, -10,  30,  40,  40,  30, -10, -30},
     {-30, -10,  30,  40,  40,  30, -10, -30},
     {-30, -10,  20,  30,  30,  20, -10, -30},
     {-30, -30,   0,   0,   0,   0, -30, -30},
     {-50, -30, -30, -30, -30, -30, -30, -50}}
};

double SAkuna::eval() {
    double val = 0;
    bool endgame = true;
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            if(board[i][j]->player >= 0) {
                val += (1-2*(board[i][j]->player ^ player)) * (
                        VALUES[board[i][j]->pt]);
                if(board[i][j]->pt == queen)
                    endgame = false;
            }
    val = (val > 0 ? 1 : -1) * pow(abs(val), 1.1); // trade down if up material
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j)
            if(board[i][j]->player >= 0) {
                val += (1-2*(board[i][j]->player ^ player)) * (
                        POSITION_TABLE[board[i][j]->pt + (endgame ? (board[i][j]->pt == king ? 2 : board[i][j]->pt == pawn ? 6 : 0) : 0)][board[i][j]->player ? i : 7-i][board[i][j]->player ? 7-j : j]);
            }
    return val;

}

const int MAX_DEPTH = 6;

pair<Move, double> SAkuna::alphabeta(int depth=0, double alpha=-numeric_limits<double>::infinity(), double beta=numeric_limits<double>::infinity()) {
    if(depth == MAX_DEPTH) {
        //++nb_states;
        return {Move(-1,-1,-1,-1), eval()};
    }
    vector<pair<double, Move>> moves;
    for(int i = 0; i < 8; ++i)
        for(int j = 0; j < 8; ++j) if(board[i][j]->player == player) {
            pair<char, unsigned long long int> cands = board[i][j]->moves(board, castling_rights, en_passant);
            for(int k = 0; k < 64; ++k) if(cands.second & (1LL << k)) {
                Move cand = Move(cands.first/8, cands.first%8, k/8, k%8);
                if(valid(cand)) {
                    board[cand.r0][cand.c0]->shadow_move(cand.r1, cand.c1, halfmove_clock, board);
                    moves.push_back({eval(), cand});
                    board[cand.r1][cand.c1]->shadow_unmove(cand.r0, cand.c0, halfmove_clock, board);
                }
            }
        }
    if(moves.size() == 0) {
        return {Move(-1, -1, -1, -1), check() ? -1000000 : 0};
    }
    random_shuffle(moves.begin(), moves.end());
    sort(moves.begin(), moves.end(), [] (const pair<double, Move> &a, const pair<double, Move> &b) -> bool {
            return a.first > b.first;
    });
    Move bestMove = moves[0].second; double bestScore = -numeric_limits<double>::infinity();
    player ^= 1;
    ++halfmove_clock;
    for(auto m : moves) {
        board[m.second.r0][m.second.c0]->shadow_move(m.second.r1, m.second.c1, halfmove_clock-1, board);
        double score = -alphabeta(depth+1, -beta, -alpha).second;
        board[m.second.r1][m.second.c1]->shadow_unmove(m.second.r0, m.second.c0, halfmove_clock-1, board);
        if(score > bestScore) {
            bestScore = score;
            bestMove = m.second;
        }
        alpha = max(alpha, bestScore);
        if(alpha >= beta)
            break;
    }
    --halfmove_clock;
    player ^= 1;
    return {bestMove, bestScore};
}

void SAkuna::start_search() {
    //nb_states = 0;
    Move bestMove = alphabeta().first;
    //fprintf(stderr, "%d\n", nb_states);
    printf("bestmove %s\n", bestMove.toString(board).c_str());
}

void SAkuna::display_board() {
    fprintf(stderr, "========\n");
    for(int i = 7; i >= 0; --i)
        for(int j = 0; j < 8; ++j)
            fprintf(stderr, "%c%c", ".pnbrqkPNBRQK"[board[i][j]->pt+(board[i][j]->player==0?6:0)], " \n"[j==7]);
    fprintf(stderr, "========\n");
}





