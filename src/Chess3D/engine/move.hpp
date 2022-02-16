#ifndef CHEESENG_MOVE_H
#define CHEESENG_MOVE_H

#include <vector>
#include <string>
#include <iostream>

#include "coord.hpp"
#include "piecetypes.hpp"

enum CastlingMove{SHORT_CASTLE, LONG_CASTLE, NO_CASTLE};

class Position;

struct Move
{
    Coord from;
    Coord to;
    Coord catpureTarget;

    CastlingMove castlingType;
    PieceType promotionType;

    std::string algebraicNotation;

    Move();
    Move(const std::string& uci);
    Move(Coord from, Coord to, PieceType promotion);

    void DebugPrintMove() const;
    void createMoveString(Position& pos);
};

#endif //CHEESENG_MOVE_H
