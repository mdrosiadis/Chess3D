#ifndef CHEESENG_PIECE_H
#define CHEESENG_PIECE_H

#include "piecetypes.hpp"
#include "move.hpp"
#include "position.hpp"

bool PieceEquals(Piece a, Piece b);

std::vector<Move> DiagonalMove(const Position& pos, Coord from, PieceColor color);
std::vector<Move> CrossMove(const Position& pos, Coord from, PieceColor color);
std::vector<Move> KnightMove(const Position& pos, Coord from, PieceColor color);
std::vector<Move> PawnMove(const Position& pos, Coord from, PieceColor color);
std::vector<Move> KingMove(const Position& pos, Coord from, PieceColor color);

#endif //CHEESENG_PIECE_H
