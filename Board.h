#pragma once
#include <iostream>
#include <vector>
#include <cstdint>
#include "Piece.h"
using namespace std;
typedef uint64_t U64;

// Bitboard convention:
// h1 = bit 0
// a1 = bit 7
// h8 = bit 56
// a8 = bit 63

struct Move {
	int from;
	int to;

	char promotion = 0;
};

struct UndoMove {

	Move move;

	Piece* movedPiece;
	Piece* capturedPiece;
	Piece* promotedPiece;

	int movedIndex;
	int capturedIndex;

	U64 oldWP;
	U64 oldWN;
	U64 oldWB;
	U64 oldWR;
	U64 oldWQ;
	U64 oldWK;

	U64 oldBP;
	U64 oldBN;
	U64 oldBB;
	U64 oldBR;
	U64 oldBQ;
	U64 oldBK;

	U64 oldAllWhite;
	U64 oldAllBlack;
	U64 oldOccupied;

	bool oldWhiteKingsideCastle;
	bool oldWhiteQueensideCastle;
	bool oldBlackKingsideCastle;
	bool oldBlackQueensideCastle;

	Piece* castledRook;
	int rookFrom;
	int rookTo;
	int oldEnPassantSquare;

};


class Board {
private:

	U64 allWhite;
	U64 allBlack;
	U64 occupied;
	U64 WP, WN, WB, WR, WQ, WK;
	U64 BP, BN, BB, BR, BQ, BK;

	Piece* pieces[32];
	Piece* squares[64]; // this is just for searches and lookups

	// flags for checking castling validity
	bool whiteKingsideCastle;
	bool whiteQueensideCastle;
	bool blackKingsideCastle;
	bool blackQueensideCastle; 
	
	int enPassantSquare;

public:
	Board() {
		WP = 0x000000000000FF00ULL;
		WN = 0x0000000000000042ULL;
		WB = 0x0000000000000024ULL;
		WR = 0x0000000000000081ULL;
		WQ = 0x0000000000000010ULL;
		WK = 0x0000000000000008ULL;
		allWhite = WP | WN | WB | WR | WQ | WK;

		BP = 0x00FF000000000000ULL;
		BN = 0x4200000000000000ULL;
		BB = 0x2400000000000000ULL;
		BR = 0x8100000000000000ULL;
		BQ = 0x1000000000000000ULL;
		BK = 0x0800000000000000ULL;
		allBlack = BP | BN | BB | BR | BQ | BK;

		occupied = allWhite | allBlack;



		// Create pieces
		pieces[0] = new Rook(false, 0, 0, 'R');
		pieces[1] = new Knight(false, 0, 1, 'N');
		pieces[2] = new Bishop(false, 0, 2, 'B');
		pieces[3] = new Queen(false, 0, 3, 'Q');
		pieces[4] = new King(false, 0, 4, 'K');
		pieces[5] = new Bishop(false, 0, 5, 'B');
		pieces[6] = new Knight(false, 0, 6, 'N');
		pieces[7] = new Rook(false, 0, 7, 'R');

		for (int i = 0; i < 8; i++) {
			pieces[8 + i] = new Pawn(false, 1, i, 'P');
		}

		pieces[16] = new Rook(true, 7, 0, 'r');
		pieces[17] = new Knight(true, 7, 1, 'n');
		pieces[18] = new Bishop(true, 7, 2, 'b');
		pieces[19] = new Queen(true, 7, 3, 'q');
		pieces[20] = new King(true, 7, 4, 'k');
		pieces[21] = new Bishop(true, 7, 5, 'b');
		pieces[22] = new Knight(true, 7, 6, 'n');
		pieces[23] = new Rook(true, 7, 7, 'r');

		for (int i = 0; i < 8; i++) {
			pieces[24 + i] = new Pawn(true, 6, i, 'p');
		}


		for (int i = 0; i < 64; i++) {
			squares[i] = nullptr;
		}

		for (int i = 0; i < 32; i++) {
			squares[pieces[i]->getSquare()] = pieces[i];
		}


		whiteKingsideCastle = true;
		whiteQueensideCastle = true;
		blackKingsideCastle = true;
		blackQueensideCastle = true;
		enPassantSquare = -1; // -1 means there is no en pessant on the board currently
	}

	U64 getOccupied() const {
		return occupied;
	}

	U64 getAllWhite() const {
		return allWhite;
	}

	U64 getAllBlack() const {
		return allBlack;
	}

	Piece* getSquare(int index) const {
		return squares[index];
	}

	int getEnPassantSquare() const {
		return enPassantSquare;
	}

	bool getWhiteKingsideCastle() const {
		return whiteKingsideCastle;
	}

	bool getWhiteQueensideCastle() const {
		return whiteQueensideCastle;
	}

	bool getBlackKingsideCastle() const {
		return blackKingsideCastle;
	}

	bool getBlackQueensideCastle() const {
		return blackQueensideCastle;
	}

	void updateBitboard(Move move, U64& bitboard) {

		U64 newSquare = (U64)1 << move.to;

		U64 oldSquare = (U64)1 << move.from;
		oldSquare = ~(oldSquare);

		bitboard &= oldSquare; // setting old square to 0
		bitboard |= newSquare; // setting new square to 1
	}

	UndoMove makeMove(Move move) {

		UndoMove u;

		u.move = move;

		u.movedPiece = squares[move.from];
		u.capturedPiece = nullptr;
		u.promotedPiece = nullptr;
		u.movedIndex = -1;
		u.capturedIndex = -1;

		// Find the moving piece's index
		for (int i = 0; i < 32; i++) {
			if (pieces[i] == u.movedPiece) {
				u.movedIndex = i;
				break;
			}
		}

		// store bitboards
		u.oldWP = WP;
		u.oldWN = WN;
		u.oldWB = WB;
		u.oldWR = WR;
		u.oldWQ = WQ;
		u.oldWK = WK;

		u.oldBP = BP;
		u.oldBN = BN;
		u.oldBB = BB;
		u.oldBR = BR;
		u.oldBQ = BQ;
		u.oldBK = BK;

		u.oldAllWhite = allWhite;
		u.oldAllBlack = allBlack;
		u.oldOccupied = occupied;

		u.oldWhiteKingsideCastle = whiteKingsideCastle;
		u.oldWhiteQueensideCastle = whiteQueensideCastle;
		u.oldBlackKingsideCastle = blackKingsideCastle;
		u.oldBlackQueensideCastle = blackQueensideCastle;

		u.castledRook = nullptr;
		u.rookFrom = -1;
		u.rookTo = -1;

		u.oldEnPassantSquare = enPassantSquare;

		int capturedSquare = move.to;

		bool enPassant = false;

		char movingType = u.movedPiece->getType();

		// If a rook is being moved normally, clear the relevant castling right
		if (movingType == 'R') {
			if (move.from == 0) whiteKingsideCastle = false;
			else if (move.from == 7) whiteQueensideCastle = false;
		}
		else if (movingType == 'r') {
			if (move.from == 56) blackKingsideCastle = false;
			else if (move.from == 63) blackQueensideCastle = false;
		}

		// En passant capture
		if ((movingType == 'P' || movingType == 'p') &&
			move.to == enPassantSquare &&
			squares[move.to] == nullptr) {

			enPassant = true;

			if (movingType == 'P')
				capturedSquare = move.to - 8;
			else
				capturedSquare = move.to + 8;
		}

		// Castling stuff
		if (u.movedPiece->getType() == 'K') {

			if (move.from == 3 && move.to == 1) {
				// White kingside

				Move rookMove;
				rookMove.from = 0;
				rookMove.to = 2;

				updateBitboard(rookMove, WR);

				Piece* rook = squares[0];
				// Validate rook before performing castling rook move
				if (rook == nullptr || rook->getType() != 'R') {
					// Invalid castling state: clear the castling right and skip rook move
					whiteKingsideCastle = false;
				}
				else {
					u.castledRook = rook;
					u.rookFrom = 0;
					u.rookTo = 2;

					squares[2] = rook;
					squares[0] = nullptr;

					rook->setSquare(2);
				}
			}

			else if (move.from == 3 && move.to == 5) {
				// White queenside

				Move rookMove;
				rookMove.from = 7;
				rookMove.to = 4;

				updateBitboard(rookMove, WR);

				Piece* rook = squares[7];
				if (rook == nullptr || rook->getType() != 'R') {
					whiteQueensideCastle = false;
				}
				else {
					u.castledRook = rook;
					u.rookFrom = 7;
					u.rookTo = 4;

					squares[4] = rook;
					squares[7] = nullptr;

					rook->setSquare(4);
				}
			}

			whiteKingsideCastle = false;
			whiteQueensideCastle = false;
		}

		if (u.movedPiece->getType() == 'k') {

			if (move.from == 59 && move.to == 57) {
				// Black kingside

				Move rookMove;
				rookMove.from = 56;
				rookMove.to = 58;

				updateBitboard(rookMove, BR);

				Piece* rook = squares[56];
				if (rook == nullptr || rook->getType() != 'r') {
					blackKingsideCastle = false;
				}
				else {
					u.castledRook = rook;
					u.rookFrom = 56;
					u.rookTo = 58;

					squares[58] = rook;
					squares[56] = nullptr;

					rook->setSquare(58);
				}
			}

			else if (move.from == 59 && move.to == 61) {
				// Black queenside

				Move rookMove;
				rookMove.from = 63;
				rookMove.to = 60;

				updateBitboard(rookMove, BR);

				Piece* rook = squares[63];
				if (rook == nullptr || rook->getType() != 'r') {
					blackQueensideCastle = false;
				}
				else {
					u.castledRook = rook;
					u.rookFrom = 63;
					u.rookTo = 60;

					squares[60] = rook;
					squares[63] = nullptr;

					rook->setSquare(60);
				}
			}

			blackKingsideCastle = false;
			blackQueensideCastle = false;
		}

		u.capturedPiece = squares[capturedSquare];

		// if capture exists, remove it from bitboards and detach from pieces[]
		if (u.capturedPiece != nullptr) {

			char type = u.capturedPiece->getType();

			// updating castling flags if captured piece is a rook
			if (type == 'R') {
				if (capturedSquare == 0) whiteKingsideCastle = false;
				else if (capturedSquare == 7) whiteQueensideCastle = false;
			}

			if (type == 'r') {
				if (capturedSquare == 56) blackKingsideCastle = false;
				else if (capturedSquare == 63) blackQueensideCastle = false;
			}

			if (type == 'P') WP &= ~((U64)1 << capturedSquare);
			else if (type == 'N') WN &= ~((U64)1 << capturedSquare);
			else if (type == 'B') WB &= ~((U64)1 << capturedSquare);
			else if (type == 'R') WR &= ~((U64)1 << capturedSquare);
			else if (type == 'Q') WQ &= ~((U64)1 << capturedSquare);
			else if (type == 'K') WK &= ~((U64)1 << capturedSquare);
			else if (type == 'p') BP &= ~((U64)1 << capturedSquare);
			else if (type == 'n') BN &= ~((U64)1 << capturedSquare);
			else if (type == 'b') BB &= ~((U64)1 << capturedSquare);
			else if (type == 'r') BR &= ~((U64)1 << capturedSquare);
			else if (type == 'q') BQ &= ~((U64)1 << capturedSquare);
			else if (type == 'k') BK &= ~((U64)1 << capturedSquare);

			for (int i = 0; i < 32; i++) {
				if (pieces[i] == u.capturedPiece) {
					u.capturedIndex = i;
					pieces[i] = nullptr;
					break;
				}
			}

			squares[capturedSquare] = nullptr;
		}

		// move piece in bitboards
		char type = u.movedPiece->getType();

		if (type == 'P') updateBitboard(move, WP);
		else if (type == 'N') updateBitboard(move, WN);
		else if (type == 'B') updateBitboard(move, WB);
		else if (type == 'R') updateBitboard(move, WR);
		else if (type == 'Q') updateBitboard(move, WQ);
		else if (type == 'K') updateBitboard(move, WK);
		else if (type == 'p') updateBitboard(move, BP);
		else if (type == 'n') updateBitboard(move, BN);
		else if (type == 'b') updateBitboard(move, BB);
		else if (type == 'r') updateBitboard(move, BR);
		else if (type == 'q') updateBitboard(move, BQ);
		else if (type == 'k') updateBitboard(move, BK);

		enPassantSquare = -1;

		if (type == 'P' && move.to - move.from == 16)
			enPassantSquare = move.from + 8;

		else if (type == 'p' && move.from - move.to == 16)
			enPassantSquare = move.from - 8;

		// update square table
		squares[move.to] = u.movedPiece;
		squares[move.from] = nullptr;

		u.movedPiece->setSquare(move.to);

		// Pawn promotion
		if ((type == 'P' && move.to >= 56) ||
			(type == 'p' && move.to <= 7)) {

			if (move.promotion != 0) {

				bool black = u.movedPiece->isBlack();
				char promotionType = move.promotion;

				// Make sure the promoted piece has the correct case
				if (black) {
					if (promotionType >= 'A' && promotionType <= 'Z')
						promotionType = promotionType + ('a' - 'A');
				}
				else {
					if (promotionType >= 'a' && promotionType <= 'z')
						promotionType = promotionType - ('a' - 'A');
				}

				// Remove the pawn from its bitboard
				if (type == 'P')
					WP &= ~((U64)1 << move.to);
				else
					BP &= ~((U64)1 << move.to);

				// Create the promoted piece
				switch (promotionType) {

				case 'Q':
				case 'q':
					u.promotedPiece = new Queen(black, 0, 0, promotionType);
					if (black) BQ |= ((U64)1 << move.to);
					else WQ |= ((U64)1 << move.to);
					break;

				case 'R':
				case 'r':
					u.promotedPiece = new Rook(black, 0, 0, promotionType);
					if (black) BR |= ((U64)1 << move.to);
					else WR |= ((U64)1 << move.to);
					break;

				case 'B':
				case 'b':
					u.promotedPiece = new Bishop(black, 0, 0, promotionType);
					if (black) BB |= ((U64)1 << move.to);
					else WB |= ((U64)1 << move.to);
					break;

				case 'N':
				case 'n':
					u.promotedPiece = new Knight(black, 0, 0, promotionType);
					if (black) BN |= ((U64)1 << move.to);
					else WN |= ((U64)1 << move.to);
					break;
				}

				// Replace the pawn in the piece list
				if (u.promotedPiece != nullptr) {
					u.promotedPiece->setSquare(move.to);
					pieces[u.movedIndex] = u.promotedPiece;
					squares[move.to] = u.promotedPiece;
				}
			}
		}

		allWhite = WP | WN | WB | WR | WQ | WK;
		allBlack = BP | BN | BB | BR | BQ | BK;
		occupied = allWhite | allBlack;

		return u;
	}

	void undoMove(UndoMove u) {

		Move move = u.move;

		// restore bitboards
		WP = u.oldWP;
		WN = u.oldWN;
		WB = u.oldWB;
		WR = u.oldWR;
		WQ = u.oldWQ;
		WK = u.oldWK;

		BP = u.oldBP;
		BN = u.oldBN;
		BB = u.oldBB;
		BR = u.oldBR;
		BQ = u.oldBQ;
		BK = u.oldBK;

		allWhite = u.oldAllWhite;
		allBlack = u.oldAllBlack;
		occupied = u.oldOccupied;

		whiteKingsideCastle = u.oldWhiteKingsideCastle;
		whiteQueensideCastle = u.oldWhiteQueensideCastle;
		blackKingsideCastle = u.oldBlackKingsideCastle;
		blackQueensideCastle = u.oldBlackQueensideCastle;
		enPassantSquare = u.oldEnPassantSquare;

		// Restore the original pawn or moving piece
		squares[move.from] = u.movedPiece;
		squares[move.to] = nullptr;

		if (u.movedPiece != nullptr) {
			u.movedPiece->setSquare(move.from);
		}

		// Restore the original piece in the piece list after promotion
		if (u.promotedPiece != nullptr) {

			delete u.promotedPiece;

			pieces[u.movedIndex] = u.movedPiece;
		}

		// Restore captured piece
		if (u.capturedPiece != nullptr) {

			int capturedSquare = move.to;

			if ((u.movedPiece->getType() == 'P' ||
				u.movedPiece->getType() == 'p') &&
				move.to == u.oldEnPassantSquare &&
				u.oldEnPassantSquare != -1) {

				if (u.movedPiece->getType() == 'P')
					capturedSquare = move.to - 8;
				else
					capturedSquare = move.to + 8;
			}

			squares[capturedSquare] = u.capturedPiece;
		}

		// restore captured piece into list if needed (use original index)
		if (u.capturedPiece != nullptr && u.capturedIndex != -1) {
			pieces[u.capturedIndex] = u.capturedPiece;
		}

		if (u.castledRook != nullptr) {

			squares[u.rookFrom] = u.castledRook;
			squares[u.rookTo] = nullptr;

			u.castledRook->setSquare(u.rookFrom);
		}
	}

	U64 getAttackedSquares(bool byBlack) {

		U64 attackedSquares = 0x0000000000000000ULL;

		for (int i = 0; i < 32; i++) {

			if (pieces[i] == nullptr) continue;
			
			if (pieces[i]->isBlack() == byBlack) {
				attackedSquares |= pieces[i]->getAttackMoves(allWhite, allBlack, occupied, enPassantSquare);
			}
		}

		return attackedSquares;
	}

	int findKingSquare(bool black) {

		for (int i = 0; i < 32; i++) {

			if (pieces[i] == nullptr) continue;

			if (pieces[i]->isBlack() == black && pieces[i]->getType() == (black ? 'k' : 'K')) {

				return pieces[i]->getSquare();
			}
		}

		return -1;
	}

	bool isKingInCheck(bool black) {

		int kingSquare = findKingSquare(black);

		U64 enemyAttacks = getAttackedSquares(!black);

		return (enemyAttacks & ((U64)1 << kingSquare)) != 0;
	}

	U64 getLegalMoves(Piece* piece) {

		U64 pseudo = piece->getPossibleMoves(allWhite, allBlack, occupied, enPassantSquare);
		U64 legal = 0;

		while (pseudo != 0) {

			int to = -1;

			// manually find the LSB set bit
			for (int i = 0; i < 64; i++) {

				if (pseudo & ((U64)1 << i)) {
					to = i;
					break;
				}
			}

			U64 bit = (U64)1 << to;

			Move m;
			m.from = piece->getSquare();
			m.to = to;

			UndoMove u = makeMove(m);

			if (!isKingInCheck(piece->isBlack())) {
				legal |= bit;
			}

			undoMove(u);

			// remove that bit
			pseudo &= ~bit;
		}

		// CASTLING
		if (piece->getType() == 'K') {

			U64 enemyAttacks = getAttackedSquares(true);

			// White kingside
			if (whiteKingsideCastle && squares[2] == nullptr && squares[1] == nullptr && !isKingInCheck(false) && !(enemyAttacks & ((U64)1 << 2)) && !(enemyAttacks & ((U64)1 << 1))){
				legal |= ((U64)1 << 1);
			}

			// White queenside
			if (whiteQueensideCastle && squares[4] == nullptr && squares[5] == nullptr && squares[6] == nullptr && !isKingInCheck(false) && !(enemyAttacks & ((U64)1 << 4)) && !(enemyAttacks & ((U64)1 << 5))){
				legal |= ((U64)1 << 5);
			}
		}

		if (piece->getType() == 'k') {

			U64 enemyAttacks = getAttackedSquares(false);

			// Black kingside
			if (blackKingsideCastle && squares[58] == nullptr && squares[57] == nullptr && !isKingInCheck(true) && !(enemyAttacks & ((U64)1 << 58)) && !(enemyAttacks & ((U64)1 << 57))) {
				legal |= ((U64)1 << 57);
			}

			// Black queenside
			if (blackQueensideCastle && squares[60] == nullptr && squares[61] == nullptr && squares[62] == nullptr && !isKingInCheck(true) && !(enemyAttacks & ((U64)1 << 60)) && !(enemyAttacks & ((U64)1 << 61))) {
				legal |= ((U64)1 << 61);
			}
		}

		return legal;
	}


	bool hasLegalMove(bool black) {

		for (int i = 0; i < 64; i++) {

			if (squares[i] == nullptr)
				continue;

			if (squares[i]->isBlack() == black && getLegalMoves(squares[i]) != 0)
				return true;
		}

		return false;
	}


	bool isCheckmate(bool black) {
		return isKingInCheck(black) && !hasLegalMove(black);
	}

	bool isStalemate(bool black) {
		return !isKingInCheck(black) && !hasLegalMove(black);
	}

	bool isInsufficientMaterial() {

		int whiteBishops = 0;
		int whiteKnights = 0;

		int blackBishops = 0;
		int blackKnights = 0;

		for (int i = 0; i < 32; i++) {

			if (pieces[i] == nullptr)
				continue;

			switch (pieces[i]->getType()) {

			// Any pawn, rook or queen means mate is possible
			case 'P': case 'R': case 'Q': case 'p': case 'r': case 'q':
				return false;

			case 'B':
				whiteBishops++;
				break;

			case 'N':
				whiteKnights++;
				break;

			case 'b':
				blackBishops++;
				break;

			case 'n':
				blackKnights++;
				break;
			}
		}

		int whiteMinors = whiteBishops + whiteKnights;
		int blackMinors = blackBishops + blackKnights;

		// Kings only
		if (whiteMinors == 0 && blackMinors == 0)
			return true;

		// King + one minor vs king
		if (whiteMinors == 1 && blackMinors == 0)
			return true;

		if (whiteMinors == 0 && blackMinors == 1)
			return true;

		// King + one minor vs king + one minor
		if (whiteMinors == 1 && blackMinors == 1)
			return true;

		return false;
	}


	vector<Move> getAllLegalMoves(bool black) {

		vector<Move> moves;

		for (int i = 0; i < 32; i++) {

			if (pieces[i] == nullptr) continue;
			if (pieces[i]->isBlack() != black) continue;

			U64 legal = getLegalMoves(pieces[i]);

			while (legal != 0) {

				int to = -1;

				for (int j = 0; j < 64; j++) {

					if (legal & ((U64)1 << j)) {
						to = j;
						break;
					}
				}

				Move m;
				m.from = pieces[i]->getSquare();
				m.to = to;
				m.promotion = 0;

				char type = pieces[i]->getType();

				// Pawn promotion
				if ((type == 'P' && to >= 56) || (type == 'p' && to <= 7)) {

					if (type == 'P') {
						m.promotion = 'Q';
						moves.push_back(m);

						m.promotion = 'R';
						moves.push_back(m);

						m.promotion = 'B';
						moves.push_back(m);

						m.promotion = 'N';
						moves.push_back(m);
					}
					else {
						m.promotion = 'q';
						moves.push_back(m);

						m.promotion = 'r';
						moves.push_back(m);

						m.promotion = 'b';
						moves.push_back(m);

						m.promotion = 'n';
						moves.push_back(m);
					}
				}
				else {
					moves.push_back(m);
				}

				legal &= ~((U64)1 << to);
			}
		}

		return moves;
	}

};