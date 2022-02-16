#include <iostream>
#include <string.h>
#include <ctype.h>

#include "lookups.hpp"
#include "move.hpp"
#include "coord.hpp"

#include "position.hpp"

Move::Move() : from(DEFAULT_INVALID_COORD), 
               to(DEFAULT_INVALID_COORD), 
               catpureTarget(DEFAULT_INVALID_COORD),
               algebraicNotation(""), castlingType(NO_CASTLE), promotionType(NO_PIECE) {};

Move::Move(const std::string& uci) : Move()
{
    const size_t len = uci.length();

    from = CoordFromAlgebraic(uci.c_str());
    to   = CoordFromAlgebraic(uci.c_str() + 2);


    if(len > 4) promotionType = Piece::FromFENChar(uci[4]).type;
}


Move::Move(Coord from, Coord to, PieceType promotion) : 
    from(from), to(to), algebraicNotation(""), catpureTarget(DEFAULT_INVALID_COORD), castlingType(NO_CASTLE), promotionType(promotion)
{
}

void Move::DebugPrintMove() const
{
    from.PrintCoordAlgebraic();
    to.PrintCoordAlgebraic();

    if(promotionType != NO_PIECE)
    {
        printf("%c", std::tolower(PIECE_DATA[promotionType].symbol));
    }

    printf(" (%s)", algebraicNotation.c_str());

    printf("\n");
}

void Move::createMoveString(Position& pos)
{
    if(algebraicNotation != "") return;

    if(castlingType != NO_CASTLE)
    {
    
        algebraicNotation = castlingType == SHORT_CASTLE ? "0-0" : "0-0-0";

        return;
    }

    Piece pieceMoving   = pos.getPieceAtCoord(from);
    Piece pieceOnTarget = pos.getPieceAtCoord(to);

    bool isCapture = (pieceOnTarget.type != NO_PIECE) || (pieceMoving.type == PAWN && CoordEquals(to, pos.en_passant));

    bool specifyFile = false, specifyRank = false;


    if(pieceMoving.type != PAWN)
    {
        std::vector<Coord> extraPiecesMovingThere;
        pos.AttackersTargetingCoord(to, pieceMoving.color, PIECE_DATA[pieceMoving.type].move_types, &extraPiecesMovingThere);

        int specC = 0;
        for(Coord c : extraPiecesMovingThere)
        {
            if(!CoordEquals(from, c) && (pos.getPieceAtCoord(c).type == pieceMoving.type))
            {
                specC++;
                if(from.rank == c.rank)   specifyFile = true;
                if(from.file == c.file)   specifyRank = true;

            }

        }
        if(specC && !specifyFile && !specifyRank) specifyFile = true;


        algebraicNotation += PIECE_DATA[pieceMoving.type].symbol;
    }
    else if(isCapture)
    {
        specifyFile = true;
    }

    if(specifyFile) algebraicNotation += from.FileChar();
    if(specifyRank) algebraicNotation += from.RankChar();

    if(isCapture) algebraicNotation += 'x';

    algebraicNotation += to.FileChar();
    algebraicNotation += to.RankChar();

    if(promotionType != NO_PIECE)
    {
        algebraicNotation += '=';
        algebraicNotation += PIECE_DATA[promotionType].symbol;
    }

    Position played(pos);

    pos.playMove(*this, played);
    played.CreateMetadata();

    switch(played.metadata.state)
    {
        case CHECK:
            algebraicNotation += '+';
            break;

        case CHECKMATE:
            algebraicNotation += '#';
            break;
    }
}
