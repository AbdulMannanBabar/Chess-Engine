#pragma once
#include <cstdint>
#include <random>
#include "Board.h"

class Zobrist {
private:

    U64 pieceKeys[12][64];
    U64 sideKey;
    U64 castlingKeys[16];
    U64 enPassantKeys[8];

    int getPieceIndex(char type) {
        switch (type) {
        case 'P': return 0;
        case 'N': return 1;
        case 'B': return 2;
        case 'R': return 3;
        case 'Q': return 4;
        case 'K': return 5;
        case 'p': return 6;
        case 'n': return 7;
        case 'b': return 8;
        case 'r': return 9;
        case 'q': return 10;
        case 'k': return 11;
        }

        return -1;
    }

public:

    Zobrist() {

        mt19937_64 rng(676767676767676767);

        for (int i = 0; i < 12; i++) {
            for (int j = 0; j < 64; j++) {
                pieceKeys[i][j] = rng(); // assigning random keys for each piece-square combination
            }
        }

        sideKey = rng();

        for (int i = 0; i < 16; i++) {
            castlingKeys[i] = rng();
        }

        for (int i = 0; i < 8; i++) {
            enPassantKeys[i] = rng();
        }
    }

    U64 hashPosition(Board& board, bool blackToMove) {

        U64 hash = 0;

        for (int i = 0; i < 64; i++) {

            Piece* piece = board.getSquare(i);

            if (piece != nullptr) {

                int pieceIndex = getPieceIndex(piece->getType());

                if (pieceIndex != -1) {
                    hash ^= pieceKeys[pieceIndex][i];
                }
            }
        }


        if (blackToMove) {
            hash ^= sideKey;
        }



        int castlingRights = 0;
        if (board.getWhiteKingsideCastle()) castlingRights |= 1;
        if (board.getWhiteQueensideCastle()) castlingRights |= 2;
        if (board.getBlackKingsideCastle()) castlingRights |= 4;
        if (board.getBlackQueensideCastle()) castlingRights |= 8;

        hash ^= castlingKeys[castlingRights];




        int enPassantSquare = board.getEnPassantSquare();

        if (enPassantSquare != -1) {
            hash ^= enPassantKeys[enPassantSquare % 8];
        }

        return hash;
    }
};
