#ifndef CHEESENG_POSITION_H
#define CHEESENG_POSITION_H
#include <iostream>
#include <vector>

#include "coord.hpp"
#include "piecetypes.hpp"
#include "move.hpp"

#define BOARD_SIZE 8
#define MAX_PIECES 32
#define PLAYER_COUNT 2

enum PositionState{NORMAL, CHECK, CHECKMATE, DRAW, INVALID};

struct PositionMetadata
{
    std::vector<Move> legalMoves;
    PositionState state;

};

class Position
{
public:
    Piece position_grid[BOARD_SIZE][BOARD_SIZE];
    PieceColor color_playing;
    bool castling_rights[PLAYER_COUNT][2];
    Coord en_passant;
    int halfmoveClock, fullmoveNumber;
    
    Coord kingPositions[2];

    PositionMetadata metadata;
    bool valid_metadata;

    std::string FEN; 

    // Constructors
    Position(const std::string& fen);

    Piece getPieceAtCoord(Coord coord) const;
    void setPieceAtCoord(Coord coord, Piece piece);

    bool isLegal();
    bool isPlayable();
    bool isInCheck(PieceColor color);
    PositionState getPositionState();
    

    std::string CreateFENString();
    void CreateMetadata();
    void CreateState();
    void ClearMetadata();
    
    void findKings();

    void DebugPrint() const;

    bool doesMoveExist(Move& move);
    void createMoveStrings();
    void playMove(const Move& move, Position& newPosition);

    void printLegalMoves();

    std::vector<Move> createLegalMoves();
    int AttackersTargetingCoord(Coord target, PieceColor color, MoveTypes castingTypes, std::vector<Coord> *out_moves=nullptr) const;
    std::vector<Move> MovesFromSquare(Coord square);
};


#endif //CHEESENG_POSITION_H
