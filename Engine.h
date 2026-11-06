#pragma once

#include <iostream>
#include <cstdint>
#include <vector>
#include "Piece.h"
#include "Board.h"
#include "Zobrist.h"

using namespace std;

typedef uint64_t U64;

// CONSTANTS
const int SEARCH_DEPTH = 4;

const int PAWN_VAL = 100;
const int KNIGHT_VAL = 300;
const int BISHOP_VAL = 300;
const int ROOK_VAL = 500;
const int QUEEN_VAL = 900;

const int BISHOP_PAIR_BONUS = 30;
const int DOUBLED_PAWN_PENALTY = 15;
const int ISOLATED_PAWN_PENALTY = 15;
const int PASSED_PAWN_BONUS[8] = {0, 5, 10, 20, 35, 60, 100, 0};

const int KNIGHT_MOBILITY_BONUS = 4;
const int BISHOP_MOBILITY_BONUS = 4;
const int ROOK_MOBILITY_BONUS = 2;
const int QUEEN_MOBILITY_BONUS = 1;

const int PAWN_SHELTER_BONUS = 10;





class Engine {
private:

    static const int PROMOTION_ORDER_BONUS = 8000;
    static const int CAPTURE_ORDER_BONUS = 1000;



    int evaluateMaterial(Board& board) {
        int score = 0;

        for (int i = 0; i < 64; i++) {
            Piece* piece = board.getSquare(i);

            if (piece == nullptr) continue;

            switch (piece->getType()) {
            case 'P': score += PAWN_VAL; break;
            case 'N': score += KNIGHT_VAL; break;
            case 'B': score += BISHOP_VAL; break;
            case 'R': score += ROOK_VAL; break;
            case 'Q': score += QUEEN_VAL; break;

            case 'p': score -= PAWN_VAL; break;
            case 'n': score -= KNIGHT_VAL; break;
            case 'b': score -= BISHOP_VAL; break;
            case 'r': score -= ROOK_VAL; break;
            case 'q': score -= QUEEN_VAL; break;

            default:
                break;
            }
        }

        return score;
    }

    int evaluatePieceSquare(Board& board) {


        // these are the PST Tables for white, for black these will get mirrored
        static const int pawnTable[64] = {
              0,   0,   0,   0,   0,   0,   0,   0,
             50,  50,  50,  50,  50,  50,  50,  50,
             10,  10,  20,  30,  30,  20,  10,  10,
              5,   5,  10,  25,  25,  10,   5,   5,
              0,   0,   0,  20,  20,   0,   0,   0,
              5,  -5, -10,   0,   0, -10,  -5,   5,
              5,  10,  10, -20, -20,  10,  10,   5,
              0,   0,   0,   0,   0,   0,   0,   0
        };

        static const int knightTable[64] = {
            -50, -40, -30, -30, -30, -30, -40, -50,
            -40, -20,   0,   5,   5,   0, -20, -40,
            -30,   5,  10,  15,  15,  10,   5, -30,
            -30,   0,  15,  20,  20,  15,   0, -30,
            -30,   5,  15,  20,  20,  15,   5, -30,
            -30,   0,  10,  15,  15,  10,   0, -30,
            -40, -20,   0,   0,   0,   0, -20, -40,
            -50, -40, -30, -30, -30, -30, -40, -50
        };

        static const int bishopTable[64] = {
            -20, -10, -10, -10, -10, -10, -10, -20,
            -10,   5,   0,   0,   0,   0,   5, -10,
            -10,  10,  10,  10,  10,  10,  10, -10,
            -10,   0,  10,  10,  10,  10,   0, -10,
            -10,   5,   5,  10,  10,   5,   5, -10,
            -10,   0,   5,  10,  10,   5,   0, -10,
            -10,   0,   0,   0,   0,   0,   0, -10,
            -20, -10, -10, -10, -10, -10, -10, -20
        };

        static const int rookTable[64] = {
              0,   0,   0,   5,   5,   0,   0,   0,
              5,  10,  10,  10,  10,  10,  10,   5,
             -5,   0,   0,   0,   0,   0,   0,  -5,
             -5,   0,   0,   0,   0,   0,   0,  -5,
             -5,   0,   0,   0,   0,   0,   0,  -5,
             -5,   0,   0,   0,   0,   0,   0,  -5,
             -5,   0,   0,   0,   0,   0,   0,  -5,
              0,   0,   0,   5,   5,   0,   0,   0
        };

        static const int queenTable[64] = {
            -20, -10, -10,  -5,  -5, -10, -10, -20,
            -10,   0,   5,   0,   0,   0,   0, -10,
            -10,   5,   5,   5,   5,   5,   0, -10,
              0,   0,   5,   5,   5,   5,   0,  -5,
             -5,   0,   5,   5,   5,   5,   0,  -5,
            -10,   0,   5,   5,   5,   5,   0, -10,
            -10,   0,   0,   0,   0,   0,   0, -10,
            -20, -10, -10,  -5,  -5, -10, -10, -20
        };

        static const int kingTable[64] = {
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -30, -40, -40, -50, -50, -40, -40, -30,
            -20, -30, -30, -40, -40, -30, -30, -20,
            -10, -20, -20, -20, -20, -20, -20, -10,
             20,  20,   0,   0,   0,   0,  20,  20,
             20,  30,  10,   0,   0,  10,  30,  20
        };

        int score = 0;

        for (int i = 0; i < 64; i++) {
            Piece* piece = board.getSquare(i);

            if (piece == nullptr)
                continue;

            int square = piece->getSquare();
            int rank = square / 8;
            int file = 7 - (square % 8);

            int relativeRank = piece->isBlack() ? 7 - rank : rank;
            int tableIndex = relativeRank * 8 + file;

            int positionalValue = 0;

            switch (piece->getType()) {
            case 'P':
            case 'p':
                positionalValue = pawnTable[tableIndex];
                break;

            case 'N':
            case 'n':
                positionalValue = knightTable[tableIndex];
                break;

            case 'B':
            case 'b':
                positionalValue = bishopTable[tableIndex];
                break;

            case 'R':
            case 'r':
                positionalValue = rookTable[tableIndex];
                break;

            case 'Q':
            case 'q':
                positionalValue = queenTable[tableIndex];
                break;

            case 'K':
            case 'k':
                positionalValue = kingTable[tableIndex];
                break;

            default:
                break;
            }

            if (piece->isBlack())
                score -= positionalValue;
            else
                score += positionalValue;
        }

        return score;
    }



    int evaluateBishopPair(Board& board) {
        int whiteBishops = 0;
        int blackBishops = 0;

        for (int i = 0; i < 64; i++) {
            Piece* piece = board.getSquare(i);

            if (piece == nullptr) continue;

            switch (piece->getType()) {
            case 'B':
                whiteBishops++;
                break;

            case 'b':
                blackBishops++;
                break;
            }
        }

        int score = 0;

        if (whiteBishops >= 2)
            score += BISHOP_PAIR_BONUS;

        if (blackBishops >= 2)
            score -= BISHOP_PAIR_BONUS;

        return score;
    }


    int evaluatePawnStructure(Board& board) {
        int score = 0;

        int whitePawns[8] = { 0 };
        int blackPawns[8] = { 0 };

        // Counting pawns on each file
        for (int i = 0; i < 64; i++) {
            Piece* piece = board.getSquare(i);

            if (piece == nullptr) continue;

            int file = 7 - (piece->getSquare() % 8);

            if (piece->getType() == 'P') whitePawns[file]++;
            else if (piece->getType() == 'p') blackPawns[file]++;
        }

        // Evaluating doubled and isolated pawns
        for (int file = 0; file < 8; file++) {

            // White doubled pawns
            if (whitePawns[file] > 1)
                score -= (whitePawns[file] - 1) * DOUBLED_PAWN_PENALTY;

            // Black doubled pawns
            if (blackPawns[file] > 1)
                score += (blackPawns[file] - 1) * DOUBLED_PAWN_PENALTY;

            // White isolated pawns
            if (whitePawns[file] > 0) {
                bool hasNeighbor = (file > 0 && whitePawns[file - 1] > 0) || (file < 7 && whitePawns[file + 1] > 0);

                if (!hasNeighbor)
                    score -= whitePawns[file] * ISOLATED_PAWN_PENALTY;
            }

            // Black isolated pawns
            if (blackPawns[file] > 0) {
                bool hasNeighbor = (file > 0 && blackPawns[file - 1] > 0) || (file < 7 && blackPawns[file + 1] > 0);

                if (!hasNeighbor)
                    score += blackPawns[file] * ISOLATED_PAWN_PENALTY;
            }
        }

        return score;
    }


    bool isPassedPawn(Board& board, int square, bool isBlack) {
        int rank = square / 8;
        int file = 7 - (square % 8);

        for (int i = 0; i < 64; i++) {
            Piece* piece = board.getSquare(i);

            if (piece == nullptr) continue;

            // Only opposing pawns can stop a passed pawn
            if (isBlack && piece->getType() != 'P')
                continue;

            if (!isBlack && piece->getType() != 'p')
                continue;

            int enemyRank = piece->getSquare() / 8;
            int enemyFile = 7 - (piece->getSquare() % 8);

            // The enemy pawn must be on the same or an adjacent file
            if (abs(enemyFile - file) > 1)
                continue;

            // White advances toward rank 7, Black advances toward rank 0
            if (!isBlack && enemyRank > rank)
                return false;

            if (isBlack && enemyRank < rank)
                return false;
        }

        return true;
    }


    int evaluatePassedPawns(Board& board) {

        int score = 0;

        for (int i = 0; i < 64; i++) {
            Piece* piece = board.getSquare(i);

            if (piece == nullptr)
                continue;

            char type = piece->getType();

            if (type != 'P' && type != 'p')
                continue;

            bool isBlack = piece->isBlack();

            if (!isPassedPawn(board, piece->getSquare(), isBlack))
                continue;

            int rank = piece->getSquare() / 8;
            int relativeRank = isBlack ? 7 - rank : rank;

            // Keep the bonus within the valid table range
            if (relativeRank < 0 || relativeRank > 7)
                continue;

            int bonus = PASSED_PAWN_BONUS[relativeRank];

            if (isBlack)
                score -= bonus;
            else
                score += bonus;
        }

        return score;
    }


    int evaluateMobility(Board& board) {

        int score = 0;

        // Evaluate White's legal moves
        vector<Move> whiteMoves = board.getAllLegalMoves(false);

        for (const Move& move : whiteMoves) {
            Piece* piece = board.getSquare(move.from);

            if (piece == nullptr) continue;

            switch (piece->getType()) {
            case 'N':
                score += KNIGHT_MOBILITY_BONUS;
                break;

            case 'B':
                score += BISHOP_MOBILITY_BONUS;
                break;

            case 'R':
                score += ROOK_MOBILITY_BONUS;
                break;

            case 'Q':
                score += QUEEN_MOBILITY_BONUS;
                break;

            default:
                break;
            }
        }

        // Evaluate Black's legal moves
        vector<Move> blackMoves = board.getAllLegalMoves(true);

        for (const Move& move : blackMoves) {
            Piece* piece = board.getSquare(move.from);

            if (piece == nullptr)  continue;


            switch (piece->getType()) {
            case 'n':
                score -= KNIGHT_MOBILITY_BONUS;
                break;

            case 'b':
                score -= BISHOP_MOBILITY_BONUS;
                break;

            case 'r':
                score -= ROOK_MOBILITY_BONUS;
                break;

            case 'q':
                score -= QUEEN_MOBILITY_BONUS;
                break;

            default:
                break;
            }
        }

        return score;
    }


    int evaluateKingShelter(Board& board, int kingSquare, bool isBlack) {

        if (kingSquare < 0 || kingSquare >= 64) return 0;


        int kingRank = kingSquare / 8;
        int kingFile = 7 - (kingSquare % 8);
        int shelterRank = isBlack ? kingRank - 1 : kingRank + 1;

        if (shelterRank < 0 || shelterRank > 7) return 0;

        int score = 0;

        // Check the three squares in "front" of the king
        for (int fileOffset = -1; fileOffset <= 1; fileOffset++) {
            int file = kingFile + fileOffset;

            if (file < 0 || file > 7)
                continue;

            int square = shelterRank * 8 + (7 - file);

            Piece* piece = board.getSquare(square);

            if (piece == nullptr) continue;

            if (!isBlack && piece->getType() == 'P')
                score += PAWN_SHELTER_BONUS;
            else if (isBlack && piece->getType() == 'p')
                score += PAWN_SHELTER_BONUS;
        }

        return score;
    }


    int evaluateKingSafety(Board& board) {
        int whiteKingSquare = board.findKingSquare(false);
        int blackKingSquare = board.findKingSquare(true);

        int whiteShelter = evaluateKingShelter(board, whiteKingSquare, false);
        int blackShelter = evaluateKingShelter(board, blackKingSquare, true);

        return whiteShelter - blackShelter;
    }

    bool isDrawByRepetition(vector<U64>& positionHistory) {
        if (positionHistory.empty())
            return false;

        U64 currentHash = positionHistory.back();
        int repetitions = 0;

        for (int i = 0; i < positionHistory.size(); i++) {
            if (positionHistory[i] == currentHash)
                repetitions++;
        }

        return repetitions >= 3;
    }

public:

    Engine() {}


    int evaluate(Board& board) {
        int materialScore = evaluateMaterial(board);
        int positionalScore = evaluatePieceSquare(board);
        int bishopPairScore = evaluateBishopPair(board);
        int pawnStructureScore = evaluatePawnStructure(board);
        int passedPawnScore = evaluatePassedPawns(board);
        int mobilityScore = evaluateMobility(board);
        int kingSafetyScore = evaluateKingSafety(board);


        int finalScore = materialScore + positionalScore + bishopPairScore + pawnStructureScore + passedPawnScore + mobilityScore + kingSafetyScore;
        return finalScore;
    }


    int search(Board& board, int depth, int alpha, int beta, bool blackToMove,
        vector<U64>& positionHistory, Zobrist& zobrist) {

        const int MATE_SCORE = 100000;
        const int INF = 1000000;

        if (isDrawByRepetition(positionHistory))
            return 0;

        vector<Move> legalMoves = board.getAllLegalMoves(blackToMove);

        // Handle checkmate and stalemate
        if (legalMoves.empty()) {
            if (board.isKingInCheck(blackToMove)) {

                if (blackToMove) {
                    return MATE_SCORE + depth;
                }
                else {
                    return -MATE_SCORE - depth;
                };
            }

            // No legal moves and not in check means stalemate
            return 0;
        }

        if (board.isInsufficientMaterial()) {
            return 0;
        }

        if (depth <= 0) {
            return evaluate(board);
        }

        // Using Move ordering to optimize the Alpha-Beta pruning process
        moveOrdering(board, legalMoves);

        if (!blackToMove) { // for white
            int bestScore = -INF;

            for (int i = 0; i < legalMoves.size(); i++) {

                const Move& move = legalMoves[i];
                UndoMove undo = board.makeMove(move);

                U64 hash = zobrist.hashPosition(board, true);
                positionHistory.push_back(hash);

                int score = search(board, depth - 1, alpha, beta, true,
                    positionHistory, zobrist);

                positionHistory.pop_back();
                board.undoMove(undo);

                if (score > bestScore)  bestScore = score;
                if (bestScore > alpha)  alpha = bestScore;

                // Alpha-beta pruning
                if (alpha >= beta) {
                    break;
                }
            }

            return bestScore;
        }
        else { // for black
            int bestScore = INF;

            for (int i = 0; i < legalMoves.size(); i++) {

                const Move& move = legalMoves[i];
                UndoMove undo = board.makeMove(move);

                U64 hash = zobrist.hashPosition(board, false);
                positionHistory.push_back(hash);

                int score = search(board, depth - 1, alpha, beta, false,
                    positionHistory, zobrist);

                positionHistory.pop_back();
                board.undoMove(undo);

                if (score < bestScore)  bestScore = score;
                if (bestScore < beta)  beta = bestScore;

                // Alpha-beta pruning
                if (alpha >= beta) {
                    break;
                }
            }

            return bestScore;
        }
    }


    Move findBestMove(Board& board, bool blackToMove, int depth,
        vector<U64>& gameHistory, Zobrist& zobrist) {

        const int INF = 1000000;

        if (depth < 1) depth = 1;

        vector<Move> legalMoves = board.getAllLegalMoves(blackToMove);

        if (legalMoves.empty() || board.isInsufficientMaterial()) {
            Move noMove;
            noMove.from = -1;
            noMove.to = -1;
            noMove.promotion = 0;
            return noMove;
        }

        // Create a separate history for the engine's search
        vector<U64> searchHistory = gameHistory;

        Move bestMove = legalMoves[0];

        int bestScore;
        if (blackToMove)  bestScore = INF;
        else bestScore = -INF;

        int alpha = -INF;
        int beta = INF;

        // Testing each move
        for (int i = 0; i < legalMoves.size(); i++) {

            Move move = legalMoves[i];
            UndoMove undo = board.makeMove(move);

            U64 hash = zobrist.hashPosition(board, !blackToMove);
            searchHistory.push_back(hash);

            int score = search(board, depth - 1, alpha, beta, !blackToMove,
                searchHistory, zobrist);

            searchHistory.pop_back();
            board.undoMove(undo);

            if (!blackToMove) {
                if (score > bestScore) {
                    bestScore = score;
                    bestMove = move;
                }

                if (bestScore > alpha) {
                    alpha = bestScore;
                }
            }
            else {
                if (score < bestScore) {
                    bestScore = score;
                    bestMove = move;
                }

                if (bestScore < beta) {
                    beta = bestScore;
                }
            }

            // Alpha-beta pruning
            if (alpha >= beta) {
                break;
            }
        }

        return bestMove;
    }





    int scoreMove(Board& board, const Move& move) {
        int score = 0;

        Piece* movingPiece = board.getSquare(move.from);
        Piece* targetPiece = board.getSquare(move.to);

        // Prioritize promotions
        if (move.promotion != 0) {
            score += PROMOTION_ORDER_BONUS;

            char promotionType = move.promotion;

            if (promotionType == 'Q' || promotionType == 'q') {
                score += 900;
            }
            else if (promotionType == 'R' || promotionType == 'r') {
                score += 400;
            }
            else if (promotionType == 'B' || promotionType == 'b') {
                score += 200;
            }
            else if (promotionType == 'N' || promotionType == 'n') {
                score += 300;
            }
        }


        if (targetPiece != nullptr && movingPiece != nullptr) {
            int capturedValue = 0;
            int movingValue = 0;

            switch (targetPiece->getType()) {
            case 'P':
            case 'p':
                capturedValue = PAWN_VAL;
                break;

            case 'N':
            case 'n':
                capturedValue = KNIGHT_VAL;
                break;

            case 'B':
            case 'b':
                capturedValue = BISHOP_VAL;
                break;

            case 'R':
            case 'r':
                capturedValue = ROOK_VAL;
                break;

            case 'Q':
            case 'q':
                capturedValue = QUEEN_VAL;
                break;
            }

            switch (movingPiece->getType()) {
            case 'P':
            case 'p':
                movingValue = PAWN_VAL;
                break;

            case 'N':
            case 'n':
                movingValue = KNIGHT_VAL;
                break;

            case 'B':
            case 'b':
                movingValue = BISHOP_VAL;
                break;

            case 'R':
            case 'r':
                movingValue = ROOK_VAL;
                break;

            case 'Q':
            case 'q':
                movingValue = QUEEN_VAL;
                break;

            case 'K':
            case 'k':
                movingValue = 20000;
                break;
            }

            score += CAPTURE_ORDER_BONUS + (capturedValue * 10) - movingValue;
        }

        return score;
    }


    void moveOrdering(Board& board, vector<Move>& moves) {

        // Insertion sorting moves from highest to lowest priority
        for (int i = 1; i < moves.size(); i++) {
            Move currentMove = moves[i];
            int currentScore = scoreMove(board, currentMove);
            int j = i - 1;

            while (j >= 0 && scoreMove(board, moves[j]) < currentScore) { 

                moves[j + 1] = moves[j];
                j--;
            }

            moves[j + 1] = currentMove;
        }
    }

};
