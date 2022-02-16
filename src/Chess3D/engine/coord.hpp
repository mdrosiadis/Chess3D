#ifndef CHEESENG_COORD_H
#define CHEESENG_COORD_H

enum CoordFile {FILE_A=0, FILE_B, FILE_C, FILE_D, FILE_E, FILE_F, FILE_G, FILE_H, INVALID_FILE=-1};
enum CoordRank {RANK_1=0, RANK_2, RANK_3, RANK_4, RANK_5, RANK_6, RANK_7, RANK_8, INVALID_RANK=-1};


struct Coord
{
    int file;
    int rank;
    
    Coord() = default;
    Coord(int f, int r);
    
    void addDirection(int dir[2]);
    char FileChar() const;
    char RankChar() const;

    void PrintCoordAlgebraic() const;
    void DebugPrintCoordFull() const;
};

#define IS_DEFAULT_INVALID_COORD(coord) (CoordEquals(coord, DEFAULT_INVALID_COORD))

bool validCoord(Coord coord);
bool CoordEquals(Coord a, Coord b);


Coord CoordFromAlgebraic(const char *alg);

#endif //CHEESENG_COORD_H
