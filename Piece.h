#pragma once
#include <iostream>
#include <cstdint>

using namespace std;
typedef uint64_t U64;



class Piece {
protected:
	bool black;
	int square;
	char type;
	
public:
	Piece(bool black, int row, int col, char type) : black(black), square((row * 8) + (7 - col)), type(type) {}

	bool isBlack() const {
		return black;
	}

	char getType() const {
		return type;
	}

	int getSquare() const {
		return square;
	}

	void setSquare(int newSquare) {
		square = newSquare;
	}

	virtual ~Piece() {}



	virtual U64 getPossibleMoves(U64 allWhite, U64 allBlack, U64 Occupied, int enPassantSquare) = 0; //returns a bitboard where all pseudo legal squares are marked as '1' whilst everything else is marked '0'
	virtual U64 getAttackMoves(U64 allWhite, U64 allBlack, U64 Occupied, int enPassantSquare) {
		return getPossibleMoves(allWhite, allBlack, Occupied, enPassantSquare);
	}


};





class Pawn : public Piece {

public:
	Pawn(bool black, int row, int col, char type) : Piece(black, row, col, type) {}


	U64 getPossibleMoves(U64 allWhite, U64 allBlack, U64 Occupied, int enPassantSquare) override {

		int currentBit = square; //gives us the position of the pawn's bit, starting from the Right most bit (LSB)
		int rank = currentBit / 8;
		int file = 7 - (currentBit % 8);
		U64 possibleMoves = 0x0000000000000000ULL;
		int nextBit = 0;
		int nextNextBit = 0;
		U64 enemies = 0;

		if (!black && rank == 7) return 0;
		if (black && rank == 0) return 0;
		
		if (black) {
			enemies = allWhite;
			nextBit = currentBit - 8;
			nextNextBit = nextBit - 8;
		}
		else {
			enemies = allBlack;
			nextBit = currentBit + 8;
			nextNextBit = nextBit + 8;
		}



		if ((((U64)1 << nextBit) & Occupied) == 0) { // the square above/below the pawn isn't occupied
			possibleMoves |= ((U64)1 << nextBit);

			// the pawn is on its starting rank and the square two spaces above/below isn't occupied
			if ( ((!black && rank == 1) || (black && rank == 6)) && ((((U64)1 << nextNextBit) & Occupied) == 0) ) {
				possibleMoves |= ((U64)1 << nextNextBit);
			}
		}

		if (file > 0) { // making sure the diagonal actually exists so that the pawn isnt actually checking a bit on the other side of the board

			if (black) {

				if ((((U64)1 << (currentBit - 7)) & enemies) != 0) { // diagonal capture
					possibleMoves |= ((U64)1 << (currentBit - 7));
				}

				if (currentBit - 7 == enPassantSquare)
					possibleMoves |= (U64)1 << (currentBit - 7);

				if (currentBit - 9 == enPassantSquare)
					possibleMoves |= (U64)1 << (currentBit - 9);

			}
			else {

				if ((((U64)1 << (currentBit + 9)) & enemies) != 0) { // diagonal capture
					possibleMoves |= ((U64)1 << (currentBit + 9));
				}

				if (currentBit + 7 == enPassantSquare)
					possibleMoves |= (U64)1 << (currentBit + 7);

				if (currentBit + 9 == enPassantSquare)
					possibleMoves |= (U64)1 << (currentBit + 9);

			}

		}

		if (file < 7) {

			if (black) {

				if ((((U64)1 << (currentBit - 9)) & enemies) != 0) { // diagonal capture
					possibleMoves |= ((U64)1 << (currentBit - 9));
				}

				if (currentBit - 7 == enPassantSquare)
					possibleMoves |= (U64)1 << (currentBit - 7);

				if (currentBit - 9 == enPassantSquare)
					possibleMoves |= (U64)1 << (currentBit - 9);

			}
			else {

				if ((((U64)1 << (currentBit + 7)) & enemies) != 0) { // diagonal capture
					possibleMoves |= ((U64)1 << (currentBit + 7));
				}

				if (currentBit + 7 == enPassantSquare)
					possibleMoves |= (U64)1 << (currentBit + 7);

				if (currentBit + 9 == enPassantSquare)
					possibleMoves |= (U64)1 << (currentBit + 9);

			}

		}

		return possibleMoves;
	}

	U64 getAttackMoves(U64 allWhite, U64 allBlack, U64 occupied,int enPassantSquare) override {

		U64 attacks = 0;

		int rank = square / 8;
		int file = 7 - (square % 8);

		if (!black && rank == 7) return 0;
		if (black && rank == 0) return 0;

		if (black) {

			if (file > 0)
				attacks |= ((U64)1 << (square - 7));

			if (file < 7)
				attacks |= ((U64)1 << (square - 9));
		}
		else {

			if (file > 0)
				attacks |= ((U64)1 << (square + 9));

			if (file < 7)
				attacks |= ((U64)1 << (square + 7));
		}

		return attacks;
	}




	~Pawn() override {}
};




class Knight : public Piece {
public:
	Knight(bool black, int row, int col, char type) : Piece(black, row, col, type) {}

	U64 getPossibleMoves(U64 allWhite, U64 allBlack, U64 Occupied, int enPassantSquare) override {

		int currentBit = square;
		int rank = currentBit / 8;
		int file = 7 - (currentBit % 8);
		U64 possibleMoves = 0x0000000000000000ULL;
		U64 friends = 0;

		if (!black) {
			friends = allWhite;
		}
		else {
			friends = allBlack;
		}

		U64 checkBit = 0;


		if (rank <= 5 && file <= 6) { // the knight has space to move up-right

			checkBit = (U64)1 << (currentBit + 15);

			if ((checkBit & friends) == 0) {
				possibleMoves |= checkBit;
			}
		}

		if (rank <= 5 && file >= 1) { // up-left

			checkBit = (U64)1 << (currentBit + 17);

			if ((checkBit & friends) == 0) {
				possibleMoves |= checkBit;
			}
		}

		if (rank >= 2 && file <= 6) { // down-right

			checkBit = (U64)1 << (currentBit - 17);

			if ((checkBit & friends) == 0) {
				possibleMoves |= checkBit;
			}
		}

		if (rank >= 2 && file >= 1) { // down-left

			checkBit = (U64)1 << (currentBit - 15);

			if ((checkBit & friends) == 0) {
				possibleMoves |= checkBit;
			}
		}

		if (file <= 5 && rank <= 6) { // right-up

			checkBit = (U64)1 << (currentBit + 6);

			if ((checkBit & friends) == 0) {
				possibleMoves |= checkBit;
			}
		}

		if (file <= 5 && rank >= 1) { // right-down

			checkBit = (U64)1 << (currentBit - 10);

			if ((checkBit & friends) == 0) {
				possibleMoves |= checkBit;
			}
		}

		if (file >= 2 && rank <= 6) { // left-up

			checkBit = (U64)1 << (currentBit + 10);

			if ((checkBit & friends) == 0) {
				possibleMoves |= checkBit;
			}
		}

		if (file >= 2 && rank >= 1) { // left-down

			checkBit = (U64)1 << (currentBit - 6);

			if ((checkBit & friends) == 0) {
				possibleMoves |= checkBit;
			}
		}


		return possibleMoves;
	}


	~Knight() override {}
};



class Rook : public Piece {
private:
	bool moved; //stores wether the rook has moved atleast once (will use this for castling)

public:
	Rook(bool black, int row, int col, char type) : Piece(black, row, col, type), moved(false) {}


	U64 getPossibleMoves(U64 allWhite, U64 allBlack, U64 Occupied, int enPassantSquare) override {

		int currentBit = square;
		int rank = currentBit / 8;
		int file = 7 - (currentBit % 8);
		U64 possibleMoves = 0x0000000000000000ULL;
		U64 friends = 0;
		U64 enemies = 0;
		U64 checkBit = 0;
		U64 occupied = Occupied;

		if (black) {
			enemies = allWhite;
		}
		else {
			enemies = allBlack;
		}

		if (rank < 7) { // Rook has space to move UP

			checkBit = (U64)1 << (currentBit + 8);
			int tempPos = rank + 1; //tempPos will be used to ensure the Bit we are checking doesn't go out of the bounds of the board

			while ( (checkBit & occupied) == 0  && tempPos <= 7 ) {
				possibleMoves |= checkBit;
				checkBit <<= 8;
				tempPos++;
			}
			if ( (checkBit & enemies) != 0  && tempPos < 8) {
				possibleMoves |= checkBit;
			}
		}

		if (rank > 0) { // DOWN

			checkBit = (U64)1 << (currentBit - 8);
			int tempPos = rank - 1; 

			while ((checkBit & occupied) == 0 && tempPos >= 0) {
				possibleMoves |= checkBit;
				checkBit >>= 8;
				tempPos--;
			}
			if ((checkBit & enemies) != 0 && tempPos >= 0) {
				possibleMoves |= checkBit;
			}
		}

		if (file < 7) { // RIGHT

			checkBit = (U64)1 << (currentBit - 1);
			int tempPos = file + 1;

			while ((checkBit & occupied) == 0 && tempPos <= 7) {
				possibleMoves |= checkBit;
				checkBit >>= 1;
				tempPos++;
			}
			if ((checkBit & enemies) != 0 && tempPos < 8) {
				possibleMoves |= checkBit;
			}
		}

		if (file > 0) { // LEFT

			checkBit = (U64)1 << (currentBit + 1);
			int tempPos = file - 1;

			while ((checkBit & occupied) == 0 && tempPos >= 0) {
				possibleMoves |= checkBit;
				checkBit <<= 1;
				tempPos--;
			}
			if ((checkBit & enemies) != 0 && tempPos >= 0) {
				possibleMoves |= checkBit;
			}
		}

		return possibleMoves;
	}


	~Rook() override {}
};



class Bishop : public Piece {
public:
	Bishop(bool black, int row, int col, char type) : Piece(black, row, col, type) {}


	U64 getPossibleMoves(U64 allWhite, U64 allBlack, U64 Occupied, int enPassantSquare) override {

		int currentBit = square;
		int rank = currentBit / 8;
		int file = 7 - (currentBit % 8);
		U64 possibleMoves = 0x0000000000000000ULL;
		U64 friends = 0;
		U64 enemies = 0;
		U64 checkBit = 0;
		U64 occupied = Occupied;

		if (black) {
			enemies = allWhite;
		}
		else {
			enemies = allBlack;
		}


		if (file < 7 && rank < 7) { // UP-RIGHT Diagonal

			checkBit = (U64)1 << (currentBit + 7);
			int tempPosY = rank + 1;
			int tempPosX = file + 1;

			while ((checkBit & occupied) == 0 && tempPosX <= 7 && tempPosY <= 7) {
				possibleMoves |= checkBit;
				checkBit <<= 7;
				tempPosX++;
				tempPosY++;
			}
			if ((checkBit & enemies) != 0 && tempPosX < 8 && tempPosY < 8) {
				possibleMoves |= checkBit;
			}

		}

		if (file > 0 && rank < 7) { // UP-LEFT Diagonal

			checkBit = (U64)1 << (currentBit + 9);
			int tempPosY = rank + 1;
			int tempPosX = file - 1;

			while ((checkBit & occupied) == 0 && tempPosX >= 0 && tempPosY <= 7) {
				possibleMoves |= checkBit;
				checkBit <<= 9;
				tempPosX--;
				tempPosY++;
			}
			if ((checkBit & enemies) != 0 && tempPosX >= 0 && tempPosY < 8) {
				possibleMoves |= checkBit;
			}

		}

		if (file < 7 && rank > 0) { // DOWN-RIGHT Diagonal

			checkBit = (U64)1 << (currentBit - 9);
			int tempPosY = rank - 1;
			int tempPosX = file + 1;

			while ((checkBit & occupied) == 0 && tempPosX <= 7 && tempPosY >= 0) {
				possibleMoves |= checkBit;
				checkBit >>= 9;
				tempPosX++;
				tempPosY--;
			}
			if ((checkBit & enemies) != 0 && tempPosX < 8 && tempPosY >= 0) {
				possibleMoves |= checkBit;
			}

		}

		if (file > 0 && rank > 0) { // DOWN-LEFT Diagonal

			checkBit = (U64)1 << (currentBit - 7);
			int tempPosY = rank - 1;
			int tempPosX = file - 1;

			while ((checkBit & occupied) == 0 && tempPosX >= 0 && tempPosY >= 0) {
				possibleMoves |= checkBit;
				checkBit >>= 7;
				tempPosX--;
				tempPosY--;
			}
			if ((checkBit & enemies) != 0 && tempPosX >= 0 && tempPosY >= 0) {
				possibleMoves |= checkBit;
			}

		}

		return possibleMoves;
	}


	~Bishop() override {}
};



class Queen : public Piece {
public:
	Queen(bool black, int row, int col, char type) : Piece(black, row, col, type) {}


	U64 getPossibleMoves(U64 allWhite, U64 allBlack, U64 Occupied, int enPassantSquare) override {

		int currentBit = square;
		int rank = currentBit / 8;
		int file = 7 - (currentBit % 8);
		U64 possibleMoves = 0x0000000000000000ULL;
		U64 enemies = 0;
		U64 checkBit = 0;
		U64 occupied = Occupied;

		if (black) {
			enemies = allWhite;
		}
		else {
			enemies = allBlack;
		}

		//Rook Movements----------------------------------------------------------
		if (rank < 7) { // UP

			checkBit = (U64)1 << (currentBit + 8);
			int tempPos = rank + 1;

			while ((checkBit & occupied) == 0 && tempPos <= 7) {
				possibleMoves |= checkBit;
				checkBit <<= 8;
				tempPos++;
			}
			if ((checkBit & enemies) != 0 && tempPos < 8) {
				possibleMoves |= checkBit;
			}
		}

		if (rank > 0) { // DOWN

			checkBit = (U64)1 << (currentBit - 8);
			int tempPos = rank - 1;

			while ((checkBit & occupied) == 0 && tempPos >= 0) {
				possibleMoves |= checkBit;
				checkBit >>= 8;
				tempPos--;
			}
			if ((checkBit & enemies) != 0 && tempPos >= 0) {
				possibleMoves |= checkBit;
			}
		}

		if (file < 7) { // RIGHT

			checkBit = (U64)1 << (currentBit - 1);
			int tempPos = file + 1;

			while ((checkBit & occupied) == 0 && tempPos <= 7) {
				possibleMoves |= checkBit;
				checkBit >>= 1;
				tempPos++;
			}
			if ((checkBit & enemies) != 0 && tempPos < 8) {
				possibleMoves |= checkBit;
			}
		}

		if (file > 0) { // LEFT

			checkBit = (U64)1 << (currentBit + 1);
			int tempPos = file - 1;

			while ((checkBit & occupied) == 0 && tempPos >= 0) {
				possibleMoves |= checkBit;
				checkBit <<= 1;
				tempPos--;
			}
			if ((checkBit & enemies) != 0 && tempPos >= 0) {
				possibleMoves |= checkBit;
			}
		}


		//Bishop Movements----------------------------------------------------------
		if (file < 7 && rank < 7) { // UP-RIGHT Diagonal

			checkBit = (U64)1 << (currentBit + 7);
			int tempPosY = rank + 1;
			int tempPosX = file + 1;

			while ((checkBit & occupied) == 0 && tempPosX <= 7 && tempPosY <= 7) {
				possibleMoves |= checkBit;
				checkBit <<= 7;
				tempPosX++;
				tempPosY++;
			}
			if ((checkBit & enemies) != 0 && tempPosX < 8 && tempPosY < 8) {
				possibleMoves |= checkBit;
			}

		}

		if (file > 0 && rank < 7) { // UP-LEFT Diagonal

			checkBit = (U64)1 << (currentBit + 9);
			int tempPosY = rank + 1;
			int tempPosX = file - 1;

			while ((checkBit & occupied) == 0 && tempPosX >= 0 && tempPosY <= 7) {
				possibleMoves |= checkBit;
				checkBit <<= 9;
				tempPosX--;
				tempPosY++;
			}
			if ((checkBit & enemies) != 0 && tempPosX >= 0 && tempPosY < 8) {
				possibleMoves |= checkBit;
			}

		}

		if (file < 7 && rank > 0) { // DOWN-RIGHT Diagonal

			checkBit = (U64)1 << (currentBit - 9);
			int tempPosY = rank - 1;
			int tempPosX = file + 1;

			while ((checkBit & occupied) == 0 && tempPosX <= 7 && tempPosY >= 0) {
				possibleMoves |= checkBit;
				checkBit >>= 9;
				tempPosX++;
				tempPosY--;
			}
			if ((checkBit & enemies) != 0 && tempPosX < 8 && tempPosY >= 0) {
				possibleMoves |= checkBit;
			}

		}

		if (file > 0 && rank > 0) { // DOWN-LEFT Diagonal

			checkBit = (U64)1 << (currentBit - 7);
			int tempPosY = rank - 1;
			int tempPosX = file - 1;

			while ((checkBit & occupied) == 0 && tempPosX >= 0 && tempPosY >= 0) {
				possibleMoves |= checkBit;
				checkBit >>= 7;
				tempPosX--;
				tempPosY--;
			}
			if ((checkBit & enemies) != 0 && tempPosX >= 0 && tempPosY >= 0) {
				possibleMoves |= checkBit;
			}

		}


		return possibleMoves;
	}


	~Queen() override {}
};




class King : public Piece {
private:
	bool moved; //for castling

public:
	King(bool black, int row, int col, char type) : Piece(black, row, col, type), moved(false) {}


	U64 getPossibleMoves(U64 allWhite, U64 allBlack, U64 Occupied, int enPassantSquare) override {

		int currentBit = square;
		int rank = currentBit / 8;
		int file = 7 - (currentBit % 8);
		U64 possibleMoves = 0x0000000000000000ULL;
		U64 friends = 0;
		U64 checkBit = 0;
		U64 occupied = Occupied;

		if (!black) {
			friends = allWhite;
		}
		else {
			friends = allBlack;
		}


		if (rank < 7) { // UP
			checkBit = (U64)1 << (currentBit + 8);
			if ((checkBit & friends) == 0) {
				possibleMoves |= checkBit;
			}
		}

		if (rank > 0) { // DOWN
			checkBit = (U64)1 << (currentBit - 8);
			if ((checkBit & friends) == 0) {
				possibleMoves |= checkBit;
			}
		}

		if (file > 0) { // RIGHT
			checkBit = (U64)1 << (currentBit + 1);
			if ((checkBit & friends) == 0) {
				possibleMoves |= checkBit;
			}
		}

		if (file < 7) { // LEFT
			checkBit = (U64)1 << (currentBit - 1);
			if ((checkBit & friends) == 0) {
				possibleMoves |= checkBit;
			}
		}

		if (file < 7 && rank < 7) { // UP-RIGHT
			checkBit = (U64)1 << (currentBit + 7);
			if ((checkBit & friends) == 0) {
				possibleMoves |= checkBit;
			}
		}

		if (file > 0 && rank < 7) { // UP-LEFT
			checkBit = (U64)1 << (currentBit + 9);
			if ((checkBit & friends) == 0) {
				possibleMoves |= checkBit;
			}
		}

		if (file < 7 && rank > 0) { // DOWN-RIGHT
			checkBit = (U64)1 << (currentBit - 9);
			if ((checkBit & friends) == 0) {
				possibleMoves |= checkBit;
			}
		}

		if (file > 0 && rank > 0) { // DOWN-LEFT
			checkBit = (U64)1 << (currentBit - 7);
			if ((checkBit & friends) == 0) {
				possibleMoves |= checkBit;
			}
		}

		return possibleMoves;
	}


	~King() override {}
};
