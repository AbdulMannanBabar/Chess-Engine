#include <iostream>
using namespace std;

#include "Piece.h"
#include "Board.h"
#include "GameManager.h"

int main() {

	Board board;
	GameManager game(board);
	game.run();

	return 0;
}