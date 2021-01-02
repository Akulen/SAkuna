#ifndef MOVE_HPP_
#define MOVE_HPP_

#include <string>
#include <vector>

#include "piece.hpp"

struct Move {
    int r0, c0, r1, c1, p;
    Move() {}
    Move(int from, int to, int p) : r0(from/8), c0(from%8), r1(to/8), c1(to%8),
        p(p) {}
    Move(int from, int to) : r0(from/8), c0(from%8), r1(to/8), c1(to%8), p(0)
        {}
    Move(int r0, int c0, int r1, int c1) : r0(r0), c0(c0), r1(r1), c1(c1), p(0)
        {}
    std::string toString();
    bool operator == (const Move&) const;
};

#endif
