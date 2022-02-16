#ifndef PIECETYPES_H
#define PIECETYPES_H
enum PieceType{PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING, NO_PIECE};
enum PieceColor{WHITE, BLACK, NO_COLOR};


#define OTHER_COLOR(color) ((color == WHITE) ? BLACK : WHITE)

#define N_MOVE_TYPES 5
struct MoveTypes
{
    bool move_types[N_MOVE_TYPES];
};

struct PieceData
{
    char symbol;
    int value;
    MoveTypes move_types;
};


struct Piece
{
    PieceType type;
    PieceColor color;

    char GetFENChar() const;
    static Piece FromFENChar(char fenChar);
};

#define NO_PIECE_LITERAL (Piece){NO_PIECE, NO_COLOR}

#endif
