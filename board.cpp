#include "board.hpp"

#include <cassert>
#include <cmath>

#include "magicmoves.hpp"

using namespace std;

const int MAX_MOVES = 256;

const int RANK_1 = 0;
const int RANK_2 = 1;
const int RANK_3 = 2;
const int RANK_4 = 3;
const int RANK_5 = 4;
const int RANK_6 = 5;
const int RANK_7 = 6;
const int RANK_8 = 7;

const int FILE_A = 0;
const int FILE_B = 1;
const int FILE_C = 2;
const int FILE_D = 3;
const int FILE_E = 4;
const int FILE_F = 5;
const int FILE_G = 6;
const int FILE_H = 7;

const Piece_Type PT_PROM[4] = {pt_queen, pt_rook, pt_bishop, pt_knight};
const double PT_VALUE[6] = {1, 3.05, 3.33, 5.63, 9.5, 0};

// Lookup Tables
const Bitboard AllSquares = ~Bitboard(0);
const Bitboard CLEAR_RANK[8] = {
    18446744073709551360ull,
    18446744073709486335ull,
    18446744073692839935ull,
    18446744069431361535ull,
    18446742978492891135ull,
    18446463698244468735ull,
    18374967954648334335ull,
       72057594037927935ull
};
const Bitboard MASK_RANK[8] = {
                     255ull,
                   65280ull,
                16711680ull,
              4278190080ull,
           1095216660480ull,
         280375465082880ull,
       71776119061217280ull,
    18374686479671623680ull
};
const Bitboard CLEAR_FILE[8] = {
    18374403900871474942ull,
    18302063728033398269ull,
    18157383382357244923ull,
    17868022691004938231ull,
    17289301308300324847ull,
    16131858542891098079ull,
    13816973012072644543ull,
     9187201950435737471ull
};
const Bitboard MASK_FILE[8] = {
       72340172838076673ull,
      144680345676153346ull,
      289360691352306692ull,
      578721382704613384ull,
     1157442765409226768ull,
     2314885530818453536ull,
     4629771061636907072ull,
     9259542123273814144ull
};
Bitboard DIAG[64][64];
Bitboard CASTLING_PATH[4] = {
    MASK_RANK[0] & (MASK_FILE[5] | MASK_FILE[6]),
    MASK_RANK[0] & (MASK_FILE[2] | MASK_FILE[3]),
    MASK_RANK[7] & (MASK_FILE[5] | MASK_FILE[6]),
    MASK_RANK[7] & (MASK_FILE[2] | MASK_FILE[3])
};

constexpr bool more_than_one(Bitboard b) {
  return b & (b - 1);
}

inline int count_bits(Bitboard b) {
    int cnt = 0;
    while(b) {
        b &= (b-1);
        ++cnt;
    }
    return cnt;
}

inline Square lsb(Bitboard b) {
  assert(b);
  return Square(__builtin_ctzll(b));
}

inline Square msb(Bitboard b) {
  assert(b);
  return Square(63 ^ __builtin_clzll(b));
}

inline Square pop_lsb(Bitboard* b) {
  assert(*b);
  const Square s = lsb(*b);
  *b &= *b - 1;
  return s;
}

void display_bitboard(Bitboard Bb);
inline Bitboard between_bb(Square s1, Square s2) {
  Bitboard b = DIAG[s1][s2] & ((AllSquares << s1) ^ (AllSquares << s2));
  return b & (b - 1); //exclude lsb
}

inline bool aligned(Square s1, Square s2, Square s3) {
    return DIAG[s1][s2] & (1LL << s3);
}

void lookup_table_init() {
    Board bd = Board();
    bd.player = 0;
    bd.allPieces[0] = bd.allPieces[1] = bd.allPieces[2] = 0;
    for(int i = 0; i < 64; ++i)
        for(int j = 0; j < 64; ++j) {
            Bitboard posi = MASK_RANK[i/8] & MASK_FILE[i%8];
            Bitboard posj = MASK_RANK[j/8] & MASK_FILE[j%8];
            for(Piece_Type pt : {pt_bishop, pt_rook}) {
                Bitboard atk = bd.attacks((Square)i, pt);
                if(atk & posj)
                    DIAG[i][j] |= atk & bd.attacks((Square)j, pt);
            }
            if(DIAG[i][j])
                DIAG[i][j] |= posi | posj;
        }
}

void display_bitboard(Bitboard Bb) {
    fprintf(stderr, "%llu\n+---------------+\n", Bb);
    for(int i = 0; i < 8; ++i) {
        fprintf(stderr, "|");
        for(int j = 0; j < 8; ++j)
            fprintf(stderr, "%c%c", (Bb & (1LL << (8*(7-i)+j))) ? 'X' : '.',
                    " |"[j==7]);
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "+---------------+\n");
}

Board::Board() {
    allPieces[0] = allPieces[1] = allPieces[2] = 0;
}

Board::Board(const string &fen, const vector<string> &moves) {
    pieces[0][pt_pawn]   = pieces[1][pt_pawn]   = 0;
    pieces[0][pt_knight] = pieces[1][pt_knight] = 0;
    pieces[0][pt_bishop] = pieces[1][pt_bishop] = 0;
    pieces[0][pt_rook]   = pieces[1][pt_rook]   = 0;
    pieces[0][pt_queen]  = pieces[1][pt_queen]  = 0;
    pieces[0][pt_king]   = pieces[1][pt_king]   = 0;

    int row = 7, c = 0, col = 0;
    while(fen[c] != ' ') {
        switch(fen[c]) {
            case '/':
                --row;
                col = -1;
                break;
            case 'p':
                pieces[1][pt_pawn] |=  1LL << (8*row+col);
                break;
            case 'n':
                pieces[1][pt_knight] |=  1LL << (8*row+col);
                break;
            case 'b':
                pieces[1][pt_bishop] |=  1LL << (8*row+col);
                break;
            case 'r':
                pieces[1][pt_rook] |=  1LL << (8*row+col);
                break;
            case 'q':
                pieces[1][pt_queen] |=  1LL << (8*row+col);
                break;
            case 'k':
                pieces[1][pt_king] |=  1LL << (8*row+col);
                break;
            case 'P':
                pieces[0][pt_pawn] |=  1LL << (8*row+col);
                break;
            case 'N':
                pieces[0][pt_knight] |=  1LL << (8*row+col);
                break;
            case 'B':
                pieces[0][pt_bishop] |=  1LL << (8*row+col);
                break;
            case 'R':
                pieces[0][pt_rook] |=  1LL << (8*row+col);
                break;
            case 'Q':
                pieces[0][pt_queen] |=  1LL << (8*row+col);
                break;
            case 'K':
                pieces[0][pt_king] |=  1LL << (8*row+col);
                break;
            default:
                col += fen[c] - '1';
        }
        ++col; ++c;
    }

    player = fen[c+1] == 'b';
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
    en_passant = Square(0);
    while(c < (int)fen.size() && fen[c] != ' ') {
        if(fen[c] >= '1' && fen[c] <= '8')
            en_passant = Square(en_passant + 8*(fen[c] - '1'));
        if(fen[c] >= 'a' && fen[c] <= 'h')
            en_passant = Square(en_passant + fen[c] - 'a');
        if(fen[c] == '-')
            en_passant = SQ_NONE;
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
        int c0 = move[0]-'a';
        int r0 = move[1]-'1';
        int c1 = move[2]-'a';
        int r1 = move[3]-'1';
        Square from = Square(8*r0+c0);
        Square to = Square(8*r1+c1);
        // handle castling rights
        if((c0 == 0 || c0 == 7) && (r0 == 0 || r0 == 7))
            castling_rights &= 15 ^ (1 << (1-c0/7 + 2*(r0/7)));
        if((c1 == 0 || c1 == 7) && (r1 == 0 || r1 == 7))
            castling_rights &= 15 ^ (1 << (1-c1/7 + 2*(r1/7)));
        if(c0 == 4 && r0 == 0)
            castling_rights &= 12;
        if(c0 == 4 && r0 == 7)
            castling_rights &= 3;
        Square new_en_passant = SQ_NONE;
        Piece_Type us_pt = piece_on(from);
        Piece_Type opp_pt = piece_on(to);
        if(opp_pt != pt_empty) {
            pieces[!player][opp_pt] ^= 1LL << to;
            halfmove_clock = -1;
        }
        pieces[player][us_pt] ^= (1LL << from) | (1LL << to);
        switch(us_pt) {
            case pt_pawn:
                halfmove_clock = -1;
                if(r0 == 1+5*player && r1 == 3+player)
                    new_en_passant = Square(8*(2+3*player) + c0);
                if(to == en_passant)
                    pieces[!player][pt_pawn] ^=
                        1LL << (player ? en_passant+8 : en_passant-8);
                if(r1 == (player ? 0 : 7)) {
                    assert(move.length() > 4);
                    pieces[player][us_pt] ^= (1LL << to);
                    switch(move[4]) {
                        case 'q':
                            pieces[player][pt_queen] ^= (1LL << to);
                            break;
                        case 'r':
                            pieces[player][pt_rook] ^= (1LL << to);
                            break;
                        case 'b':
                            pieces[player][pt_bishop] ^= (1LL << to);
                            break;
                        case 'n':
                            pieces[player][pt_knight] ^= (1LL << to);
                            break;
                        default:
                            assert(false);
                    }
                }
                break;
            case pt_knight:
            case pt_bishop:
            case pt_rook:
            case pt_queen:
                break;
            case pt_king:
                // castling
                if(c1 - c0 == 2) {
                    halfmove_clock = -1;
                    pieces[player][pt_rook] ^=
                        (1LL << (8*r0+7)) | (1LL << (8*r0+5));
                } else if(c0 - c1 == 2) {
                    halfmove_clock = -1;
                    pieces[player][pt_rook] ^=
                        (1LL << (8*r0)) | (1LL << (8*r0+3));
                }
                break;
            default:
                assert(false);
        }
        en_passant = new_en_passant;
        ++halfmove_clock;
        if(player) ++fullmove_number;
        player ^= 1;
    }

    init_done = false;
    //fprintf(stderr, "%s %lu\n", fen.c_str(), moves.size());
    //display_bitboard(allPieces[2]);
    //Move moveList[MAX_MOVES];
    //Move *last = this->moves(moveList);
    //fprintf(stderr, "# of moves: %lu\n", last-moveList);
    //for(Move *move = moveList; move < last; ++move)
    //    fprintf(stderr, "%s\n", move->toString().c_str());
}

void Board::init() {
    if(init_done) return;

    allPieces[0] = pieces[0][pt_pawn] | pieces[0][pt_knight] |
                 pieces[0][pt_bishop] | pieces[0][pt_rook]   |
                  pieces[0][pt_queen] | pieces[0][pt_king];
    allPieces[1] = pieces[1][pt_pawn] | pieces[1][pt_knight] |
                 pieces[1][pt_bishop] | pieces[1][pt_rook]   |
                  pieces[1][pt_queen] | pieces[1][pt_king];
    allPieces[2] = allPieces[0] | allPieces[1];

    // detect check
    Square kg = lsb(pieces[player][pt_king]);
    checkers = attacks_on(kg);

    // compute king blockers
    Bitboard snipers =
        (attacks_empty(kg, pt_rook) &
            (pieces[!player][pt_rook] | pieces[!player][pt_queen]))
        | (attacks_empty(kg, pt_bishop) &
            (pieces[!player][pt_bishop] | pieces[!player][pt_queen]));
    Bitboard occupancy = allPieces[2] ^ snipers;
    blockers = 0;
    while(snipers) {
        Square sniper = pop_lsb(&snipers);
        Bitboard b = between_bb(kg, sniper) & occupancy;

        if(b && !more_than_one(b)) {
            blockers |= b;
        }
    }

    init_done = true;
}

Bitboard Board::compute_king_incomplete(Bitboard king_loc, Bitboard own_side) const {
    /* we can ignore the rank clipping since the overflow/underflow with
        respect to rank simply vanishes. We only care about the file
        overflow/underflow. */ 

    Bitboard king_clip_file_h = king_loc & CLEAR_FILE[FILE_H]; 
    Bitboard king_clip_file_a = king_loc & CLEAR_FILE[FILE_A]; 

    /* remember the representation of the board in relation to the bitindex 
         when looking at these shifts.... */
    Bitboard spot_1 = king_clip_file_a << 7; 
    Bitboard spot_2 = king_loc << 8; 
    Bitboard spot_3 = king_clip_file_h << 9; 
    Bitboard spot_4 = king_clip_file_h << 1; 

    Bitboard spot_5 = king_clip_file_h >> 7; 
    Bitboard spot_6 = king_loc >> 8; 
    Bitboard spot_7 = king_clip_file_a >> 9; 
    Bitboard spot_8 = king_clip_file_a >> 1; 

    Bitboard king_moves = spot_1 | spot_2 | spot_3 | spot_4 |
                          spot_5 | spot_6 | spot_7 | spot_8; 

    Bitboard KingValid = king_moves & ~own_side; 

    /* compute only the places where the king can move and attack. The caller
        will interpret this as a white or black king. */
    return KingValid;
}

Bitboard Board::compute_knight(Bitboard knight_loc, Bitboard own_side) const {
    /* we can ignore the rank clipping since the overflow/underflow with
        respect to rank simply vanishes. We only care about the file
        overflow/underflow which is much more work for a knight. */ 
    
    Bitboard spot_1_clip = CLEAR_FILE[FILE_A] & CLEAR_FILE[FILE_B];
    Bitboard spot_2_clip = CLEAR_FILE[FILE_A];
    Bitboard spot_3_clip = CLEAR_FILE[FILE_H];
    Bitboard spot_4_clip = CLEAR_FILE[FILE_H] & CLEAR_FILE[FILE_G];

    Bitboard spot_5_clip = CLEAR_FILE[FILE_H] & CLEAR_FILE[FILE_G];
    Bitboard spot_6_clip = CLEAR_FILE[FILE_H];
    Bitboard spot_7_clip = CLEAR_FILE[FILE_A];
    Bitboard spot_8_clip = CLEAR_FILE[FILE_A] & CLEAR_FILE[FILE_B];

    /* The clipping masks we just created will be used to ensure that no
        under or overflow positions are computed when calculating the
        possible moves of the knight in certain files. */

    Bitboard spot_1 = (knight_loc & spot_1_clip) << 6;
    Bitboard spot_2 = (knight_loc & spot_2_clip) << 15;
    Bitboard spot_3 = (knight_loc & spot_3_clip) << 17;
    Bitboard spot_4 = (knight_loc & spot_4_clip) << 10;

    Bitboard spot_5 = (knight_loc & spot_5_clip) >> 6;
    Bitboard spot_6 = (knight_loc & spot_6_clip) >> 15;
    Bitboard spot_7 = (knight_loc & spot_7_clip) >> 17;
    Bitboard spot_8 = (knight_loc & spot_8_clip) >> 10;

    Bitboard KnightValid = spot_1 | spot_2 | spot_3 | spot_4 |
                           spot_5 | spot_6 | spot_7 | spot_8;

    /* compute only the places where the knight can move and attack. The
        caller will determine if this is a white or black night. */
    return KnightValid & ~own_side;
}

Bitboard Board::compute_pawn(Bitboard pawn_loc, int player) const {
    /* check the single space infront of the pawn */
    Bitboard pawn_one_step =
        (player ? pawn_loc >> 8 : pawn_loc << 8) & ~allPieces[2]; 

    /* for all moves that came from rank 2 (home row) and passed the above 
        filter, thereby being on rank 3, check and see if I can move forward 
        one more */
    Bitboard pawn_one_step_double =
        pawn_one_step & MASK_RANK[player ? RANK_6 : RANK_3];
    Bitboard pawn_two_steps = 
        (player ? pawn_one_step_double >> 8 : pawn_one_step_double << 8) &
        ~allPieces[2]; 

    /* the union of the movements dictate the possible moves forward 
        available */
    Bitboard pawn_valid_moves =
        pawn_one_step | pawn_two_steps;

    /* next we calculate the pawn attacks */

    /* check the left side of the pawn, minding the underflow File A */
    Bitboard pawn_left_attack =
        player ? (pawn_loc & CLEAR_FILE[FILE_A]) >> 9 :
                 (pawn_loc & CLEAR_FILE[FILE_A]) << 7;

    /* then check the right side of the pawn, minding the overflow File H */
    Bitboard pawn_right_attack =
        player ? (pawn_loc & CLEAR_FILE[FILE_H]) >> 7 :
                 (pawn_loc & CLEAR_FILE[FILE_H]) << 9;

    /* the union of the left and right attacks together make up all the 
        possible attacks */
    Bitboard pawn_attacks =
        pawn_left_attack | pawn_right_attack;

    /* Calculate where I can _actually_ attack something */
    Bitboard pawn_valid_attacks = pawn_attacks & allPieces[!player];

    /* then we combine the two situations in which a pawn can legally 
        attack/move. */
    Bitboard PawnValid =
        pawn_valid_moves | pawn_valid_attacks;

    /* Check if en passant is possible */
    if(en_passant != SQ_NONE) {
        if(en_passant / 8 != (player ? 2 : 5)) return PawnValid;
        Bitboard pawn_ep =
            MASK_RANK[en_passant / 8] & MASK_FILE[en_passant % 8];
        PawnValid |= (pawn_ep & pawn_attacks);
    }

    return PawnValid;
}

Move* Board::moves(Move* moveList) {
    init();
    Move* cur = moveList;

    moveList = make_moves(moveList, pt_pawn);
    moveList = make_moves(moveList, pt_knight);
    moveList = make_moves(moveList, pt_bishop);
    moveList = make_moves(moveList, pt_rook);
    moveList = make_moves(moveList, pt_queen);
    moveList = make_moves(moveList, pt_king);
    // filter illegal moves
    Bitboard pinned = blockers & allPieces[player];
    Square kg = lsb(pieces[player][pt_king]);
    while(cur != moveList) {
        if((pinned || kg == 8*cur->r0 + cur->c0
                    || ((pieces[player][pt_pawn] & MASK_RANK[cur->r0]
                            & MASK_FILE[cur->c0])
                        && en_passant == 8*cur->r1 + cur->c1))
                && !legal(*cur))
            *cur = *(--moveList);
        else
            ++cur;
    }
    return moveList;
}

Bitboard Board::attacks_empty(Square from, Piece_Type pt, int player) {
    if(player == -1) player = this->player;
    Bitboard b;//, from_bb = MASK_RANK[from/8] & MASK_FILE[from%8];
    switch(pt) {
        case pt_bishop:
            b = Bmagic(from, 0) & ~allPieces[player];
            break;
        case pt_rook:
            b = Rmagic(from, 0) & ~allPieces[player];
            break;
        default:
            assert(false);
            break;
    }
    return b;
}

Bitboard Board::attacks(Square from, Piece_Type pt, int player) const {
    if(player == -1) player = this->player;
    Bitboard b, from_bb = MASK_RANK[from/8] & MASK_FILE[from%8];
    switch(pt) {
        case pt_pawn:
            b = compute_pawn(from_bb, player);
            break;
        case pt_knight:
            b = compute_knight(from_bb, allPieces[player]);
            break;
        case pt_bishop:
            b = Bmagic(from, allPieces[2]) & ~allPieces[player];
            break;
        case pt_rook:
            b = Rmagic(from, allPieces[2]) & ~allPieces[player];
            break;
        case pt_queen:
            b = Rmagic(from, allPieces[2]) & ~allPieces[player];
            b |= Bmagic(from, allPieces[2]) & ~allPieces[player];
            break;
        case pt_king:
            b = compute_king_incomplete(from_bb, allPieces[player]);
            break;
        default:
            b = 0;
            break;
    }
    return b;
}

Bitboard Board::attacks_on(Square target) const {
    return (attacks(target, pt_pawn, player) & pieces[!player][pt_pawn])   |
           (attacks(target, pt_knight)       & pieces[!player][pt_knight]) |
           (attacks(target, pt_bishop)       & pieces[!player][pt_bishop]) |
           (attacks(target, pt_rook)         & pieces[!player][pt_rook])   |
           (attacks(target, pt_queen)        & pieces[!player][pt_queen])  |
           (attacks(target, pt_king)         & pieces[!player][pt_king]);
}

Move* Board::make_moves(Move* moveList, Piece_Type pt) {
    Bitboard target = ~allPieces[player];
    if(checkers) {
        Square kg = lsb(pieces[player][pt_king]);
        if(pt == pt_king) {
            Bitboard sliderAttacks = 0;
            Bitboard sliders = checkers &
                ~(pieces[!player][pt_knight] | pieces[!player][pt_pawn]);
            while(sliders)
                sliderAttacks |= DIAG[kg][pop_lsb(&sliders)] & ~checkers;

            // Evasions of the king
            Bitboard b = attacks(kg, pt_king) & ~allPieces[player] & ~sliderAttacks;
            while(b)
                *moveList++ = Move((int)kg, (int)pop_lsb(&b));
            return moveList;
        }

        if(more_than_one(checkers))
            return moveList;

        target = between_bb(kg, lsb(checkers)) | checkers;
        if(pt == pt_pawn && en_passant != SQ_NONE) {
            Bitboard pawn_ep = 1LL << en_passant;
            if((player ? pawn_ep << 8 : pawn_ep >> 8) & checkers)
                target |= pawn_ep;
        }
    }
    Bitboard bb = pieces[player][pt], b;
    Square from;
    while(bb) {
        from = pop_lsb(&bb);
        b = attacks(from, pt) & target;
        while(b) {
            if(pt == pt_pawn && from / 8 == (player ? 1 : 6)) {
                /* Handle promotions */
                Square to = pop_lsb(&b);
                for(int p = 1; p < 5; ++p)
                    *moveList++ = Move((int)from, (int)to, p);
            } else
                *moveList++ = Move((int)from, (int)pop_lsb(&b));
        }
    }

    // Castling
    if(pt == pt_king) {
        Square kg = lsb(pieces[player][pt_king]);
        for(int cr = 0; cr < 2; ++cr)
            if((castling_rights >> (2*player)) & (1+cr) &&
                    (CASTLING_PATH[2*player+cr] & allPieces[2]) == 0) {
                bb = CASTLING_PATH[2*player+cr];
                bool valid = true;
                while(bb) {
                    if(attacks_on(pop_lsb(&bb))) {
                        valid = false;
                        break;
                    }
                }
                if(!valid) continue;
                *moveList++ = Move((int)kg, (int)kg+2-4*cr);
            }
    }
    return moveList;
}

bool Board::legal(Move m) const {
    Square from = Square(8*m.r0 + m.c0);
    Square to = Square(8*m.r1 + m.c1);

    // en passant, check if king is in check
    if(piece_on(from) == pt_pawn && en_passant == to) {
        Square kg = lsb(pieces[player][pt_king]);
        Square capsq = Square(player ? to + 8 : to - 8);
        Bitboard occupied =
            (allPieces[2] ^ (1LL << from) ^ (1LL << capsq)) | (1LL << to);
        return !(Rmagic(kg, occupied) &
                (pieces[!player][pt_rook] | pieces[!player][pt_queen]))
            && !(Bmagic(kg, occupied) &
                (pieces[!player][pt_bishop] | pieces[!player][pt_queen]));
    }

    // check if king moved in check
    if(pieces[player][pt_king] & (1LL << from)) {
        return !attacks_on(to);
    }

    // A non-king move is legal if and only if it is not pinned or it
    // is moving along the ray towards or away from the king.
    return !(blockers & (1LL << from))
        || aligned(from, to, lsb(pieces[player][pt_king]));
}

Piece_Type Board::piece_on(Square sq) const {
    for(auto pt : {pt_pawn, pt_knight, pt_bishop, pt_rook, pt_queen, pt_king})
        if((pieces[0][pt] | pieces[1][pt]) & (1LL << sq))
            return pt;
    return pt_empty;
}

void Board::do_move(Move m, Board* newBd) {
    for(int p = 0; p < 2; ++p)
        for(int pt = 0; pt < 6; ++pt)
            newBd->pieces[p][pt] = pieces[p][pt];
    newBd->player = player;
    newBd->castling_rights = castling_rights;
    newBd->en_passant = en_passant;
    newBd->halfmove_clock = halfmove_clock;
    newBd->fullmove_number = fullmove_number;
    newBd->init_done = false;

    int c0 = m.c0;
    int r0 = m.r0;
    int c1 = m.c1;
    int r1 = m.r1;
    Square from = Square(8*r0+c0);
    Square to = Square(8*r1+c1);
    // handle castling rights
    if((c0 == 0 || c0 == 7) && (r0 == 0 || r0 == 7))
        newBd->castling_rights &= 15 ^ (1 << (1-c0/7 + 2*(r0/7)));
    if((c1 == 0 || c1 == 7) && (r1 == 0 || r1 == 7))
        newBd->castling_rights &= 15 ^ (1 << (1-c1/7 + 2*(r1/7)));
    if(c0 == 4 && r0 == 0)
        newBd->castling_rights &= 12;
    if(c0 == 4 && r0 == 7)
        newBd->castling_rights &= 3;
    Square new_en_passant = SQ_NONE;
    Piece_Type us_pt = piece_on(from);
    Piece_Type opp_pt = piece_on(to);
    if(opp_pt != pt_empty) {
        newBd->pieces[!player][opp_pt] ^= 1LL << to;
        newBd->halfmove_clock = -1;
    }
    newBd->pieces[player][us_pt] ^= (1LL << from) | (1LL << to);
    switch(us_pt) {
        case pt_pawn:
            newBd->halfmove_clock = -1;
            if(r0 == 1+5*player && r1 == 3+player)
                new_en_passant = Square(8*(2+3*player) + c0);
            if(to == en_passant)
                newBd->pieces[!player][pt_pawn] ^=
                    1LL << (player ? en_passant+8 : en_passant-8);
            if(r1 == (player ? 0 : 7)) {
                newBd->pieces[player][us_pt] ^= (1LL << to);
                assert(m.p != 0);
                newBd->pieces[player][PT_PROM[m.p-1]] ^= (1LL << to);
            }
            break;
        case pt_knight:
        case pt_bishop:
        case pt_rook:
        case pt_queen:
            break;
        case pt_king:
            // castling
            if(c1 - c0 == 2) {
                newBd->halfmove_clock = -1;
                newBd->pieces[player][pt_rook] ^=
                    (1LL << (8*r0+7)) | (1LL << (8*r0+5));
            } else if(c0 - c1 == 2) {
                newBd->halfmove_clock = -1;
                newBd->pieces[player][pt_rook] ^=
                    (1LL << (8*r0)) | (1LL << (8*r0+3));
            }
            break;
        default:
            assert(false);
    }
    newBd->en_passant = new_en_passant;
    ++newBd->halfmove_clock;
    if(player) ++newBd->fullmove_number;
    newBd->player ^= 1;
}

bool Board::in_check(int player) const {
    return attacks_on(lsb(pieces[player][pt_king]));
}

void Board::display() const {
    fprintf(stderr, "%d %d %d %d\n+---------------+\n", castling_rights, en_passant, halfmove_clock, fullmove_number);
    for(int i = 0; i < 8; ++i) {
        fprintf(stderr, "|");
        for(int j = 0; j < 8; ++j) {
            Piece_Type pt = piece_on(Square(8*(7-i)+j));
            fprintf(stderr, "%c%c", "PNBRQK."[pt] + (pt != pt_empty) * ((pieces[0][pt] & (1LL << (8*(7-i)+j))) == 0) * ('a' - 'A'), " |"[j==7]);
        }
        fprintf(stderr, "\n");
    }
    fprintf(stderr, "+---------------+\n");
}

// Values taken from https://www.chessprogramming.org/Simplified_Evaluation_Function
const double VALUES[6] = {100, 320, 330, 500, 900, 20000};
const double POSITION_TABLE[8][8][8] = {
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

double Board::eval() const {
    double sc[3] = {0, 0, 0};
    for(int p = 0; p < 2; ++p)
        for(int pt = 0; pt < 6; ++pt)
            sc[p] += VALUES[pt] * count_bits(pieces[p][pt]);
    //double sc = sc = (sc > 0 ? 1 : -1) * pow(abs(sc), 1.1); // trade down if up material
    sc[2] = pow(sc[1], 1.1) - pow(sc[0], 1.1);
    Bitboard b;
    for(int p = 0; p < 2; ++p)
        for(int pt = 0; pt < 6; ++pt) {
            b = pieces[p][pt];
            while(b) {
                Square cur = pop_lsb(&b);
                sc[2] += (2*p-1) *
                  POSITION_TABLE[pt][p ? cur/8 : 7-cur/8][p ? cur%8 : 7-cur%8];
            }
        }
    return (player ? 1 : -1) * sc[2];
    //double sc = 0;
    //for(int p = 0; p < 2; ++p)
    //    for(int pt = 0; pt < 6; ++pt)
    //        sc += (2*p-1) * PT_VALUE[pt] * count_bits(pieces[p][pt]);
    //return (player ? 1 : -1) * sc;
    //double val = 0;
    //bool endgame = false;
    //int queen_cnt[2] = {0, 0};
    //int minor_cnt[2] = {0, 0};
    //int major_cnt[2] = {0, 0};
    //for(int i = 0; i < 8; ++i)
    //    for(int j = 0; j < 8; ++j)
    //        if(board[i][j]->player >= 0) {
    //            val += (1-2*(board[i][j]->player ^ player)) * (
    //                    VALUES[board[i][j]->pt]);
    //            switch(board[i][j]->pt) {
    //                case queen:
    //                    ++queen_cnt[board[i][j]->player];
    //                    break;
    //                case knight:
    //                case bishop:
    //                    ++minor_cnt[board[i][j]->player];
    //                    break;
    //                case rook:
    //                    ++major_cnt[board[i][j]->player];
    //                    break;
    //                default:
    //                    break;
    //            };
    //        }
    //bool endgame = (queen_cnt[0] == 0 || (queen_cnt[0] == 1 && minor_cnt[0] < 2 && major_cnt[0] == 0))
    //    && (queen_cnt[1] == 0 || (queen_cnt[1] == 1 && minor_cnt[1] < 2 && major_cnt[1] == 0));
    //val = (val > 0 ? 1 : -1) * pow(abs(val), 1.1); // trade down if up material
    //for(int i = 0; i < 8; ++i)
    //    for(int j = 0; j < 8; ++j)
    //        if(board[i][j]->player >= 0) {
    //            val += (1-2*(board[i][j]->player ^ player)) * (
    //                    POSITION_TABLE[board[i][j]->pt + (endgame ? (board[i][j]->pt == king ? 2 : board[i][j]->pt == pawn ? 6 : 0) : 0)][board[i][j]->player ? i : 7-i][board[i][j]->player ? 7-j : j]);
    //        }
    //return val;

}






