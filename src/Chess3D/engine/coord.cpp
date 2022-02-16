#include <iostream>
#include <cstdio>

#include "coord.hpp"

const Coord DEFAULT_INVALID_COORD = {INVALID_FILE, INVALID_RANK};

Coord::Coord(int f, int r) : file(f), rank(r) {}; 

char Coord::FileChar() const
{
    return file + 'a';
}

char Coord::RankChar() const
{
    return rank + '1';
}

void Coord::addDirection(int dir[2])
{
    file += dir[0];
    rank += dir[1];
}

inline bool validCoord(Coord coord)
{
    return coord.file >= CoordFile::FILE_A && coord.file <= CoordFile::FILE_H && 
           coord.rank >= CoordRank::RANK_1 && coord.rank <= CoordRank::RANK_8;
}

void Coord::DebugPrintCoordFull() const
{
    PrintCoordAlgebraic();
    std::printf(" (file: %d row: %d)\n", file + 'a', rank + '1');
}

bool CoordEquals(Coord a, Coord b)
{
    return a.file == b.file && a.rank == b.rank;
}


void Coord::PrintCoordAlgebraic() const
{
    std::printf("%c%c", FileChar(), RankChar());
}

Coord CoordFromAlgebraic(const char *alg)
{
    Coord c(alg[0] - 'a', alg[1] - '1');

    return validCoord(c) ? c : DEFAULT_INVALID_COORD;
}
