#include <iostream>
#include <cstdio>
#include <vector>

#include "position.hpp"
#include "move.hpp"
#include "lookups.hpp"


const char castleTypes[] = {'K', 'Q', 'k', 'q'};

static const int DRAW_HALFMOVES = 100;

void Position::DebugPrint() const
{

    for(int rank = BOARD_SIZE-1; rank >= 0; rank--)
    {
        for(int file = 0; file < BOARD_SIZE; file++)
        {
            std::putchar(position_grid[file][rank].GetFENChar());

        }

        std::putchar('\n');
    }

    std::printf("Half move clock: %d | Fullmove number: %d\n", halfmoveClock, fullmoveNumber);

    printf("En passant: ");
    if(validCoord(en_passant))
        en_passant.PrintCoordAlgebraic();
    else
        std::putchar('-');

    std::printf("\nCastling rights: ");
    for(int i=0; i < 4; i++) if(castling_rights[i/2][i%2]) std::putchar(castleTypes[i]);

    std::printf("\n%s to move\n", color_playing == WHITE ? "White" : "Black");
}

Position::Position(const std::string& fen) : FEN(fen)
{
    int file = 0, rank = BOARD_SIZE -1;
    int fi;
    char FEN_CHAR;

    valid_metadata = false;
    kingPositions[0] = DEFAULT_INVALID_COORD; kingPositions[1] = DEFAULT_INVALID_COORD;
    en_passant = DEFAULT_INVALID_COORD;

    // Position data
    for(fi=0; fi < FEN.length() && !(rank == 0 && file == BOARD_SIZE); ++fi)
    {
        FEN_CHAR = FEN[fi];

        if(FEN_CHAR >= '1' && FEN_CHAR <= '8')
        {
            int empty = FEN_CHAR - '0';

            /* assert(file+empty <= BOARD_SIZE); */

            for(unsigned int i = 0; i < empty; i++, file++)
                position_grid[file][rank] = NO_PIECE_LITERAL;
        }
        else if(FEN_CHAR == '/')
        {
            /* assert(file == BOARD_SIZE); */

            rank--;
            file = 0;
        }
        else
        {
            position_grid[file][rank] = Piece::FromFENChar(FEN_CHAR);
            file++;
        }

    }

    // skip space
    /* assert(row == 0 && file == BOARD_SIZE); */
    /* assert(*FEN == ' '); */
    fi++;
    // turn

    switch(FEN[fi++])
    {
        case 'w': color_playing = WHITE; break;
        case 'b': color_playing = BLACK; break;

        default: exit(-1);
    }

    for(++fi; fi < FEN.length() && ((FEN_CHAR = FEN[fi]) != ' '); ++fi)
    {

        for(int i = 0; i < 4; i++)
        {
            if(FEN_CHAR == castleTypes[i])
            {
                castling_rights[i/2][i%2] = true;
                break;
            }
        }
    }

    /* Skip space */
    /* assert(*FEN == ' '); */
    fi++;

    if(FEN[fi] == '-')
    {
        en_passant = DEFAULT_INVALID_COORD;
        fi++;
    }
    else
    {
        en_passant = CoordFromAlgebraic(&FEN[fi]);
        //assert(validCoord(en_passant));
        fi+=2;
    }

    /* assert(*FEN == ' '); */
    fi++;
    std::sscanf(&FEN[fi], "%d %d", &halfmoveClock, &fullmoveNumber);

    CreateMetadata();
    findKings();
    createMoveStrings();
}
/*
void Position::CreateFENString()
{
    FEN = ""; 

    for(int row = BOARD_SIZE-1; row >= 0; row--)
    {
        int noPieceRunning = 0;
        for(int file = 0; file < BOARD_SIZE; file++)
        {
            Piece currentPiece = getPieceAtCoord((Coord){file, row});

            if(currentPiece.type != NO_PIECE)
            {
                if(noPieceRunning) printf("%d", noPieceRunning);

                putchar(currentPiece.GetFENChar());
                noPieceRunning = 0;
            }
            else
            {
                noPieceRunning++;
            }

        }

        if(noPieceRunning) printf("%d", noPieceRunning);

        if(row != 0) putchar('/');

    }

    printf(" %c ", color_playing == WHITE ? 'w' : 'b');

    bool anyCastlingRights = false;
    for(int i=0; i < 4; i++)
    {
        if(castling_rights[i/2][i%2])
        {
            putchar(castleTypes[i]);
            anyCastlingRights = true;
        }
    }

    if(!anyCastlingRights) putchar('-');

    putchar(' ');

    if(validCoord(pos->en_passant))
        PrintCoordAlgebraic(pos->en_passant);
    else
        putchar('-');

    printf(" %d %d", pos->halfmoveClock, pos->fullmoveNumber);

}
*/
Piece Position::getPieceAtCoord(Coord coord) const
{
    return validCoord(coord) ? position_grid[coord.file][coord.rank] : NO_PIECE_LITERAL;
}

void Position::setPieceAtCoord(Coord coord, Piece piece)
{
    if(validCoord(coord)) position_grid[coord.file][coord.rank] = piece;
}



// Utility Function: Get the number (and locations) of squares attacking the target square
// param data: NULL for no list creation, pointer to Darray(Coord) to return the data
// return: number of squares / pieces found attacking the target square
// PURPOSE OF EXISTENCE OF THIS FUNCTION: dont create a list of potential attacks if we only need the number of them
int Position::AttackersTargetingCoord(Coord target, PieceColor color, MoveTypes castingTypes, std::vector<Coord> *out_moves) const
{
    int numberOfAttacks = 0;
    PieceColor castingAs = OTHER_COLOR(color);

    std::vector<Move> result;

    for(int moveType=0; moveType < 5; moveType++)
    {
        if(!castingTypes.move_types[moveType]) continue;

        std::vector<Move> moves = MOVE_TYPE_FUNCTION_LOOKUP[moveType](*this, target, castingAs);

        for(Move& move : moves)
        {
            Piece pieceAtTarget = getPieceAtCoord(move.to);
            if(pieceAtTarget.type != NO_PIECE && PIECE_DATA[pieceAtTarget.type].move_types.move_types[moveType])
            {
                if(out_moves) out_moves->push_back(move.to);
                numberOfAttacks++;

            }
        }

    }

    return numberOfAttacks;
}

PositionState Position::getPositionState()
{
    PieceColor colorNotPlaying = OTHER_COLOR(color_playing);

    if(!valid_metadata) CreateMetadata();

    return metadata.state;
}

bool Position::isInCheck(PieceColor color)
{
    if(CoordEquals(kingPositions[color], DEFAULT_INVALID_COORD))
            findKings();
    
    return AttackersTargetingCoord(kingPositions[color], OTHER_COLOR(color), ALL_TYPES) != 0;
}


bool Position::isLegal()
{

    return !isInCheck(OTHER_COLOR(color_playing));
}

bool Position::isPlayable()
{
    PositionState state = getPositionState();

    return state == NORMAL || state == CHECK;
}

void Position::findKings()
{
    Coord current;
    int found = 0;

    for(current.file = FILE_A; found < 2 && current.file <= FILE_H; current.file++)
        for(current.rank = RANK_1; current.rank <= RANK_8; current.rank++)
        {
            Piece pieceAtCurrent = getPieceAtCoord(current);

            if(pieceAtCurrent.type == KING)
            {
                kingPositions[pieceAtCurrent.color] = current;
                found++;
            }
        }

}


void Position::CreateMetadata()
{
    if(valid_metadata) return;

    valid_metadata = true;

    metadata.legalMoves = createLegalMoves();

    PieceColor colorNotPlaying = OTHER_COLOR(color_playing);

    //printf("meta-check\n");
    // If the color not playing is in check, the position is invalid
    if(isInCheck(colorNotPlaying))
        metadata.state =  INVALID;

    else if(halfmoveClock >= DRAW_HALFMOVES)
        metadata.state = DRAW;

    else
    {
        /* Check for checks and legal moves.
     * If in check and no legal moves -> checkmate, if legal moves -> check.
     * If not in check and no legal moves -> draw*/

        bool inCheck = isInCheck(color_playing);

        int legalMoveCount = metadata.legalMoves.size();


        if(inCheck)
            metadata.state = legalMoveCount > 0 ? CHECK : CHECKMATE;
        else
            metadata.state = legalMoveCount > 0 ? NORMAL : DRAW;
    }

}

std::vector<Move> Position::MovesFromSquare(Coord square)
{
    std::vector<Move> moves;

    Piece pieceAtCurrent = getPieceAtCoord(square);

    if (pieceAtCurrent.color != color_playing) return moves;

    for (int type = 0; type < 5; type++) {
        if (!PIECE_DATA[pieceAtCurrent.type].move_types.move_types[type]) continue;

        std::vector<Move> extra = MOVE_TYPE_FUNCTION_LOOKUP[type](*this, square, color_playing);
    
        for(Move& move : extra)
        {
            Position check = *this;
            playMove(move, check);
            
            if(check.isLegal()) moves.push_back(move);
        }
    }

    return moves;
}

void Position::playMove(const Move& move, Position& newPosition)
{
    Coord enPassantCache = en_passant;
    newPosition = *this;


    newPosition.en_passant = DEFAULT_INVALID_COORD;
    newPosition.valid_metadata = false;

    newPosition.halfmoveClock++;
    if(color_playing == BLACK) newPosition.fullmoveNumber++;

    if(move.castlingType != NO_CASTLE)
    {

        newPosition.setPieceAtCoord(CASTLING_KING_START_COORD[color_playing], NO_PIECE_LITERAL);
        newPosition.setPieceAtCoord(CASTLING_ROOK_START_COORD[color_playing][move.castlingType], NO_PIECE_LITERAL);

        newPosition.setPieceAtCoord(CASTLING_KING_TARGET_COORD[color_playing][move.castlingType], Piece{KING, newPosition.color_playing});
        newPosition.setPieceAtCoord(CASTLING_ROOK_TARGET_COORD[color_playing][move.castlingType], Piece{ROOK, newPosition.color_playing});

        newPosition.castling_rights[newPosition.color_playing][SHORT_CASTLE] = false;
        newPosition.castling_rights[newPosition.color_playing][LONG_CASTLE]  = false;

        newPosition.kingPositions[color_playing] = CASTLING_KING_TARGET_COORD[color_playing][move.castlingType];
    }
    else
    {
        Piece pieceMoving = getPieceAtCoord(move.from);

        if(pieceMoving.type == KING)
        {
            newPosition.castling_rights[color_playing][SHORT_CASTLE] = false;
            newPosition.castling_rights[color_playing][LONG_CASTLE] = false;

            newPosition.kingPositions[color_playing] = move.to;
        }
        else if(pieceMoving.type == ROOK)
        {
            for(int castleType = SHORT_CASTLE; castleType <= LONG_CASTLE; castleType++)
            {
                if(CoordEquals(move.from, CASTLING_ROOK_START_COORD[color_playing][castleType]))
                {
                    newPosition.castling_rights[color_playing][castleType] = false;
                }
            }
        }
        else if(pieceMoving.type == PAWN)
        {
            newPosition.halfmoveClock = 0;

            if(move.from.rank == PAWN_STARTING_RANK[color_playing] &&
                    move.to.rank == (PAWN_STARTING_RANK[color_playing] + 2 * PAWN_MOVING_DIRECTION[color_playing]) &&
                    (PieceEquals(getPieceAtCoord(Coord(move.to.file-1, move.to.rank)), Piece{PAWN, OTHER_COLOR(color_playing)}) ||
                     PieceEquals(getPieceAtCoord(Coord(move.to.file+1, move.to.rank)), Piece{PAWN, OTHER_COLOR(color_playing)}) ))
            {
                newPosition.en_passant = Coord(move.from.file, move.from.rank + PAWN_MOVING_DIRECTION[color_playing]);
            }
        }

        if(getPieceAtCoord(move.to).type != NO_PIECE) newPosition.halfmoveClock = 0;

        // order is important here
        if(!CoordEquals(move.catpureTarget, DEFAULT_INVALID_COORD))
                newPosition.setPieceAtCoord(move.catpureTarget, NO_PIECE_LITERAL);

        newPosition.setPieceAtCoord(move.to, pieceMoving);
        newPosition.setPieceAtCoord(move.from, NO_PIECE_LITERAL);


        if(move.promotionType != NO_PIECE) newPosition.setPieceAtCoord(move.to, Piece{move.promotionType, color_playing});
    }

    newPosition.color_playing = OTHER_COLOR(newPosition.color_playing);
    
    CreateMetadata();
    findKings();
    if(this == &newPosition) createMoveStrings();
}

std::vector<Move> Position::createLegalMoves()
{
    std::vector<Move> allMoves;
    Coord current;

    for(current.file = FILE_A; current.file <= FILE_H; current.file++) {
        for (current.rank = RANK_1; current.rank <= RANK_8; current.rank++)
        {
            Piece pieceAtCurrent = getPieceAtCoord(current);

            if (pieceAtCurrent.color != color_playing) continue;

            std::vector<Move> movesToAdd = MovesFromSquare(current);

            allMoves.insert(allMoves.end(), movesToAdd.begin(), movesToAdd.end());
        }
    }

    // short castle
    if(castling_rights[color_playing][SHORT_CASTLE] &&
       PieceEquals(getPieceAtCoord(CASTLING_KING_START_COORD[color_playing]), {KING, color_playing}) &&
       PieceEquals(getPieceAtCoord(CASTLING_ROOK_START_COORD[color_playing][SHORT_CASTLE]), {ROOK, color_playing}))
    {
        Coord current = CASTLING_KING_START_COORD[color_playing];
        current.file++;
        bool valid = true;

        for(; current.file < CASTLING_ROOK_START_COORD[color_playing][SHORT_CASTLE].file; current.file++)
        {
            if(getPieceAtCoord(current).type != NO_PIECE)
            {
                valid = false;
                break;
            }

            if(current.file <= CASTLING_KING_TARGET_COORD[color_playing][SHORT_CASTLE].file &&
               AttackersTargetingCoord(current, OTHER_COLOR(color_playing), ALL_TYPES) > 0)
            {
                valid = false;
                break;
            }
        }

        if(valid)
        {
            Move castling;
            castling.from = CASTLING_KING_START_COORD[color_playing];
            castling.to   = CASTLING_KING_TARGET_COORD[color_playing][SHORT_CASTLE];
            castling.castlingType = SHORT_CASTLE;

            allMoves.push_back(castling);
        }
    }
    

    // long castle
    if(castling_rights[color_playing][LONG_CASTLE] &&
       PieceEquals(getPieceAtCoord(CASTLING_KING_START_COORD[color_playing]), {KING, color_playing}) &&
       PieceEquals(getPieceAtCoord(CASTLING_ROOK_START_COORD[color_playing][LONG_CASTLE]), {ROOK, color_playing}))
    {
        Coord current = CASTLING_KING_START_COORD[color_playing];
        current.file--;
        bool valid = true;

        for(; current.file > CASTLING_ROOK_START_COORD[color_playing][LONG_CASTLE].file; current.file--)
        {
            if(getPieceAtCoord(current).type != NO_PIECE)
            {
                valid = false;
                break;
            }

            if(current.file >= CASTLING_KING_TARGET_COORD[color_playing][LONG_CASTLE].file &&
               AttackersTargetingCoord(current, OTHER_COLOR(color_playing), ALL_TYPES) > 0)
            {
                valid = false;
                break;
            }
        }

        if(valid)
        {
            Move castling;
            castling.from = CASTLING_KING_START_COORD[color_playing];
            castling.to   = CASTLING_KING_TARGET_COORD[color_playing][LONG_CASTLE];
            castling.castlingType = LONG_CASTLE; 

            allMoves.push_back(castling);
        }
    }

    return allMoves;
}

void Position::createMoveStrings()
{
    for(Move& move : metadata.legalMoves) move.createMoveString(*this);
}

bool Position::doesMoveExist(Move& move)
{
    CreateMetadata();

    for(Move& validMove : metadata.legalMoves)
    {
        if(CoordEquals(validMove.from, move.from) && 
           CoordEquals(validMove.to, move.to)     &&
           validMove.promotionType == move.promotionType)
        {
            move = validMove;
            return true;
        }
    }

    return false;
}

void Position::printLegalMoves()
{
    int i = 1;
    for(Move& move : metadata.legalMoves)
    {
        std::cout << i++ << " ";
        move.DebugPrintMove();
        move.catpureTarget.DebugPrintCoordFull();
    } 
}
