#include "move.hpp"

using namespace std;

string Move::toString() {
    string ans = "";
    ans += c0+'a';
    ans += r0+'1';
    ans += c1+'a';
    ans += r1+'1';
    switch(p) {
        case 1:
            ans += 'q';
            break;
        case 2:
            ans += 'r';
            break;
        case 3:
            ans += 'b';
            break;
        case 4:
            ans += 'n';
            break;
        default:
            break;
    }
    return ans;
}

bool Move::operator == (const Move& other) const {
    return r0 == other.r0 && c0 == other.c0 && r1 == other.r1 && c1 == other.c1 && p == other.p;
}
