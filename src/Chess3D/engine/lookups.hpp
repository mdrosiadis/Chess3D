#include "coord.hpp"
#include "move.hpp"
#include "position.hpp"
#include "piecetypes.hpp"
#include "piece.hpp"

static const PieceData PIECE_DATA[] =
{
    {'P', 1, {0, 0, 0, 1, 0}},
    {'N', 3, {0, 0, 1, 0, 0}},
    {'B', 1, {1, 0, 0, 0, 0}},
    {'R', 5, {0, 1, 0, 0, 0}},
    {'Q', 9, {1, 1, 0, 0, 0}},
    {'K', 0, {0, 0, 0, 0, 1}},
    {'.', 0, {0, 0, 0, 0, 0}}
};

/* static const Coord DEFAULT_INVALID_COORD = {-1, -1}; */
#define DEFAULT_INVALID_COORD Coord(INVALID_FILE, INVALID_RANK)

static const int DIAGONAL_DIRECTIONS[][2] = {{-1, -1}, {1, 1}, {-1, 1}, {1, -1}};
static const int CROSS_DIRECTIONS[][2]    = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};

static const int KNIGHT_MOVES[][2] = {{1, 2}, {2, 1}, {2, -1}, {1, -2}, {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}};

static const Coord CASTLING_KING_START_COORD[2] = {{FILE_E, RANK_1}, {FILE_E, RANK_8}};
static const Coord CASTLING_ROOK_START_COORD[2][2] = {{{FILE_H, RANK_1}, {FILE_A, RANK_1}}, {{FILE_H, RANK_8}, {FILE_A, RANK_8}}};

static const Coord CASTLING_KING_TARGET_COORD[2][2] = {{{FILE_G, RANK_1}, {FILE_C, RANK_1}}, {{FILE_G, RANK_8}, {FILE_C, RANK_8}}};
static const Coord CASTLING_ROOK_TARGET_COORD[2][2] = {{{FILE_F, RANK_1}, {FILE_D, RANK_1}}, {{FILE_F, RANK_8}, {FILE_D, RANK_8}}};

static const CoordRank PAWN_STARTING_RANK[2]  = {RANK_2, RANK_7};
static const CoordRank PAWN_PROMOTION_RANK[2] = {RANK_8, RANK_1};

static const int PAWN_MOVING_DIRECTION[2] = {1, -1};

std::vector<Move> (* const MOVE_TYPE_FUNCTION_LOOKUP[])(const Position&, Coord, PieceColor) =
{DiagonalMove, CrossMove, KnightMove, PawnMove, KingMove};

static const MoveTypes ALL_TYPES = {1,1,1,1,1};
