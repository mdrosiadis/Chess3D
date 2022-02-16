#include "piecetypes.hpp"
#include "piece.hpp"
#include "coord.hpp"
#include "position.hpp"
#include "lookups.hpp"

#include <cctype>

const char PIECE_SYMBOLS[] = {'P', 'N', 'B', 'R', 'Q', 'K', '.'};

bool PieceEquals(Piece a, Piece b)
{
    return a.color == b.color && a.type == b.type;
}

char Piece::GetFENChar() const
{
    return color == WHITE ? PIECE_SYMBOLS[type] : std::tolower(PIECE_SYMBOLS[type]);
}

Piece Piece::FromFENChar(char FENChar)
{
    char p = toupper(FENChar);
    
    for(int i = 0; i < sizeof PIECE_SYMBOLS / sizeof (char); i++)
    {
        if(p == PIECE_SYMBOLS[i])
            return {static_cast<PieceType>(i),  std::isupper(FENChar) ? WHITE : BLACK};
    }

    return NO_PIECE_LITERAL;
}

#define COORD_ADD_DIRECTION(a, b) (a).file+=b[0], (a).rank+=b[1]


std::vector<Move> DirectionalMove(const Position& pos, Coord from, PieceColor color, const int DIRSET[4][2])
{
    std::vector<Move> moves;
    
    for(int dir = 0; dir < 4; dir++) 
    { 
        Coord current = from;
        COORD_ADD_DIRECTION(current, DIRSET[dir]); 
        for(; validCoord(current); COORD_ADD_DIRECTION(current, DIRSET[dir]))
        { 
            Piece pieceAtCurrent = pos.getPieceAtCoord(current); 

            /* Piece hit! If enemy, it can be a capture unless its the enemy king */ 
            Move newMove(from, current);

            if(pieceAtCurrent.type != NO_PIECE) 
            { 

                newMove.catpureTarget = current;
                if(pieceAtCurrent.color != color) 
                {
                    moves.push_back(newMove);
                } 


                break; 
            } 

            /* Otherwise, we can make a regular move on square current */ 
            moves.push_back(newMove); 
        } 
    } 
    return moves;

}

std::vector<Move> DiagonalMove(const Position& pos, Coord from, PieceColor color)
{
    return DirectionalMove(pos, from, color, DIAGONAL_DIRECTIONS);
}

std::vector<Move> CrossMove(const Position& pos, Coord from, PieceColor color)
{
    return DirectionalMove(pos, from, color, CROSS_DIRECTIONS);
}


std::vector<Move> KnightMove(const Position& pos, Coord from, PieceColor color)
{
    std::vector<Move> moves;

    for(int move=0; move < 8; move++)
    {
        Coord current = from;

        COORD_ADD_DIRECTION(current, KNIGHT_MOVES[move]);

        if(!validCoord(current)) continue;

        Piece pieceAtCurrent = pos.getPieceAtCoord(current);

        if(pieceAtCurrent.type == NO_PIECE || pieceAtCurrent.color != color)
        {
            Move newMove(from, current);

            if(pieceAtCurrent.type != NO_PIECE) newMove.catpureTarget = current;
            moves.push_back(newMove);
        }
    }

    return moves;
}

std::vector<Move> PawnMove(const Position& pos, Coord from, PieceColor color)
{
    std::vector<Move> moves;

    int captures[][2] = {{-1, PAWN_MOVING_DIRECTION[color]}, {1, PAWN_MOVING_DIRECTION[color]}};

    Coord current = from;

    for(int i=0; i < 1 + (from.rank == PAWN_STARTING_RANK[color]); i++)
    {
        current.rank += PAWN_MOVING_DIRECTION[color];

        if(!validCoord(current)) break;

        Piece pieceAtCurrent = pos.getPieceAtCoord(current);

        if(pieceAtCurrent.type != NO_PIECE) break;

        Move newMove(from, current);

        if(current.rank == PAWN_PROMOTION_RANK[color])
        {
            for(int type = PieceType::KNIGHT; type <= PieceType::QUEEN; type++)
            {
                newMove.promotionType = static_cast<PieceType>(type);
                moves.push_back(newMove);
            }
        }
        else
        {
            moves.push_back(newMove);
        }

    }


    for(int move=0; move < 2; move++)
    {
        current = from;
        COORD_ADD_DIRECTION(current, captures[move]); 

        if(!validCoord(current)) continue;

        Piece pieceAtCurrent = pos.getPieceAtCoord(current);

        if(pieceAtCurrent.type != NO_PIECE && pieceAtCurrent.color != color)
        {
            Move newMove(from, current);
            newMove.catpureTarget = current;

            if(current.rank == PAWN_PROMOTION_RANK[color])
            {
                for(int type = PieceType::KNIGHT; type <= PieceType::QUEEN; type++)
                {
                    newMove.promotionType = static_cast<PieceType>(type);
                    moves.push_back(newMove);
                }
            }
            else
            {
                moves.push_back(newMove);
            }

        }
        else if(CoordEquals(current, pos.en_passant) && pieceAtCurrent.type == NO_PIECE && color == pos.color_playing) //en passant
        {

            Move newMove(from , current);
            newMove.catpureTarget = Coord(current.file, current.rank - PAWN_MOVING_DIRECTION[color]);
            moves.push_back(newMove);
        }
    }

    return moves;
}


std::vector<Move> KingMove(const Position& pos, Coord from, PieceColor color)
{
    std::vector<Move> moves;
    
    for(int file=-1; file <= 1; file++)
        for(int rank=-1; rank <= 1; rank++)
        {
            if(file == 0 && rank == 0) continue;

            Coord current(from.file + file, from.rank + rank);

            if(!validCoord(current)) continue;

            Piece pieceAtCurrent = pos.getPieceAtCoord(current);

            if(pieceAtCurrent.type == NO_PIECE || pieceAtCurrent.color != color)
            {
                Move newMove(from, current);

                if(pieceAtCurrent.type != NO_PIECE) newMove.catpureTarget = current;

                moves.push_back(newMove);
            }

        }

    return moves;
}

