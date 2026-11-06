#pragma once
#include <iostream>
#include <cstdint>
#include <vector>
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/Window/Keyboard.hpp>
#include <random>

#include "Engine.h"
#include "Piece.h"
#include "Board.h"
#include "Zobrist.h"
using namespace sf;

class GameManager {

private:
	RenderWindow window;
	Board& board;
	Piece* selectedPiece;
	U64 highlightedMoves;

	Texture PieceTextures;
	Font font;

	bool whiteToMove;
	bool promotionState;
	UndoMove promotionUndo;
	Move promotionMove;


	// Engine and game setup
	Engine engine;
	bool colorSelection = true;
	bool humanPlaysWhite = true;

	// vectors to store move and position histories of the game
	vector<Move> moveHistory;
	vector<U64> positionHistory;
	Zobrist zobrist;

public:
	GameManager(Board& board): window(VideoMode(1200, 1000), "Chess Engine"), board(board), selectedPiece(nullptr), highlightedMoves(0), whiteToMove(true), promotionState(false), colorSelection(true), humanPlaysWhite(true) {

		PieceTextures.loadFromFile("Sprites/Piece_Sprites.png");
		font.loadFromFile("Sprites/PressStart2P.ttf");
	}


	void handleMouseClick(int x, int y) {

		// COLOR SELECTION SCREEN
		if (colorSelection) {

			if (y >= 430 && y < 570) {

				if (x >= 100 && x < 400) {
					startGame(true);   // Play White
				}
				else if (x >= 450 && x < 750) {
					startGame(false);  // Play Black
				}
				else if (x >= 800 && x < 1100) {

					// Choose White or Black with equal probability.
					random_device rd;
					mt19937 generator(rd());
					uniform_int_distribution<int> distribution(0, 1);

					startGame(distribution(generator) == 0);
				}
			}

			return;
		}

		// Ignore input when the engine is to move or the game has ended.
		if (whiteToMove != humanPlaysWhite || isGameOver()) {
			return;
		}

		if (promotionState) {

			char promotionType = 0;

			if (whiteToMove) {

				// White promotion menu
				if (x >= 400 && x < 500 && y >= 900 && y <= 1000) {
					promotionType = 'Q';
				}
				else if (x >= 500 && x < 600 && y >= 900 && y <= 1000) {
					promotionType = 'B';
				}
				else if (x >= 600 && x < 700 && y >= 900 && y <= 1000) {
					promotionType = 'N';
				}
				else if (x >= 700 && x < 800 && y >= 900 && y <= 1000) {
					promotionType = 'R';
				}
			}
			else {

				// Black promotion menu
				if (x >= 400 && x < 500 && y >= 0 && y <= 100) {
					promotionType = 'q';
				}
				else if (x >= 500 && x < 600 && y >= 0 && y <= 100) {
					promotionType = 'b';
				}
				else if (x >= 600 && x < 700 && y >= 0 && y <= 100) {
					promotionType = 'n';
				}
				else if (x >= 700 && x < 800 && y >= 0 && y <= 100) {
					promotionType = 'r';
				}
			}

			// If a promotion piece was selected, complete the move
			if (promotionType != 0) {

				promotionMove.promotion = promotionType;
				board.makeMove(promotionMove);
				recordMove(promotionMove);
				whiteToMove = !whiteToMove;

				promotionState = false;
				selectedPiece = nullptr;
				highlightedMoves = 0;

				makeEngineMove();
			}
			else {
				promotionState = false;
				selectedPiece = nullptr;
				highlightedMoves = 0;
			}

			return;
		}

		// Ignore clicks outside the chessboard
		if (x < 200 || x >= 1000 || y < 100 || y >= 900) {
			selectedPiece = nullptr;
			highlightedMoves = 0;
			return;
		}

		int file = (x - 200) / 100;
		int tempRank = (y - 100) / 100;
		int rank = 7 - tempRank;

		int square = rank * 8 + (7 - file);

		if (selectedPiece != nullptr) {

			U64 bit = (U64)1 << square;

			// If the clicked square is a legal destination, move there
			if ((highlightedMoves & bit) != 0) {

				Move m;
				m.from = selectedPiece->getSquare();
				m.to = square;
				m.promotion = 0;

				Piece* piece = board.getSquare(m.from);

				bool promotion =
					(piece->getType() == 'P' && m.to >= 56) ||
					(piece->getType() == 'p' && m.to <= 7);

				if (!promotion) {

					board.makeMove(m);
					recordMove(m);
					whiteToMove = !whiteToMove;

					selectedPiece = nullptr;
					highlightedMoves = 0;

					// Respond immediately after the player's move
					display();
					makeEngineMove();
				}
				else {
					// Store the move and wait for the promotion choice
					promotionMove = m;

					selectedPiece = nullptr;
					highlightedMoves = 0;

					promotionState = true;
				}

				return;
			}
		}

		Piece* pieceOnSquare = board.getSquare(square);

		if (!pieceOnSquare) {
			// Clicking an empty square removes highlights
			selectedPiece = nullptr;
			highlightedMoves = 0;
			return;
		}

		if (pieceOnSquare->isBlack() == whiteToMove) {
			return;
		}

		selectedPiece = pieceOnSquare;
		highlightedMoves = board.getLegalMoves(selectedPiece);
	}

	void run() {

		while (window.isOpen()) {

			Event event;

			while (window.pollEvent(event)) {

				if (event.type == Event::Closed) {
					window.close();
				}

				if (event.type == Event::MouseButtonPressed && event.mouseButton.button == Mouse::Left) {

					handleMouseClick(event.mouseButton.x, event.mouseButton.y);
				}
			}

			display();
		}
	}

	void display() {

		window.clear();

		RectangleShape background(Vector2f(1200,1000));
		background.setFillColor(Color(40,40,40));
		window.draw(background);

		// COLOR SELECTION SCREEN
		if (colorSelection) {

			Text title;
			title.setFont(font);
			title.setString("CHOOSE YOUR COLOR");
			title.setCharacterSize(30);
			title.setFillColor(Color::White);

			FloatRect titleBounds = title.getLocalBounds();
			title.setOrigin(
				titleBounds.left + titleBounds.width / 2.0f,
				titleBounds.top + titleBounds.height / 2.0f
			);
			title.setPosition(600, 300);

			window.draw(title);

			// Draw the three selection buttons
			auto drawButton = [this](float x, const string& label, Color fillColor) {

				RectangleShape button(Vector2f(300, 140));
				button.setPosition(x, 430);
				button.setFillColor(fillColor);

				window.draw(button);

				Text buttonText;
				buttonText.setFont(font);
				buttonText.setString(label);
				buttonText.setCharacterSize(18);
				buttonText.setFillColor(Color::White);

				FloatRect bounds = buttonText.getLocalBounds();
				buttonText.setOrigin(
					bounds.left + bounds.width / 2.0f,
					bounds.top + bounds.height / 2.0f
				);
				buttonText.setPosition(x + 150, 500);

				window.draw(buttonText);
				};

			drawButton(100, "PLAY WHITE", Color(90, 120, 90));
			drawButton(450, "PLAY BLACK", Color(90, 90, 140));
			drawButton(800, "RANDOM", Color(120, 100, 80));

			window.display();
			return;
		}

		const int tileSize = 100;
		const int boardX = 200;
		const int boardY = 100;
		float scaleX = 100.0f / 314.0f    -0.025;
		float scaleY = 100.0f / 302.0f    -0.025;


		// DRAWING THE CHESS BOARD
		for (int y = 1; y < 9; y++) {
			for (int x = 2; x < 10; x++) {

				RectangleShape square(sf::Vector2f(tileSize, tileSize));
				square.setPosition(x * tileSize, y * tileSize);

				if ((x + y) % 2 == 0)
					square.setFillColor(Color(240, 217, 181)); // light
				else
					square.setFillColor(Color(181, 136, 99));  // dark

				window.draw(square);
			}
		}


		// DRAWING HIGHLIGHTED MOVES (IF ANY)
		for (int square = 0; square < 64; square++) {

			U64 bit = (U64)1 << square;

			if ((highlightedMoves & bit) == 0)
				continue;

			int rank = square / 8;
			int file = 7 - (square % 8);

			float x = boardX + file * tileSize;
			float y = boardY + (7 - rank) * tileSize;

			RectangleShape highlight(Vector2f(tileSize, tileSize));

			highlight.setPosition(x, y);
			highlight.setFillColor(Color(0, 255, 0, 100));

			window.draw(highlight);
		}


		// DRAWING CHESS PIECES
		for (int i = 0; i < 64; i++) {
			if (board.getSquare(i) != nullptr) {

				Sprite PieceSprite;
				PieceSprite.setTexture(PieceTextures);
				PieceSprite.setScale(scaleX, scaleY);
				int square = i;
				int rank = square / 8;
				int file = 7 - (square % 8);
				float x = boardX + (tileSize * file);
				float y = boardY + ((7 - rank) * tileSize);
				PieceSprite.setPosition(x-15, y+5);
				

				switch (board.getSquare(i)->getType()) {

				case 'K':
					PieceSprite.setTextureRect(IntRect(0, 0, 314, 302));
					PieceSprite.setPosition(x + 10, y + 7.5);
					PieceSprite.setScale(scaleX, scaleY);
					break;

				case 'Q':
					PieceSprite.setTextureRect(IntRect(315, 0, 314, 302));
					PieceSprite.setPosition(x + 5, y + 10);
					PieceSprite.setScale(scaleX, scaleY);
					break;

				case 'B':
					PieceSprite.setTextureRect(IntRect(629, 0, 314, 302));
					PieceSprite.setPosition(x, y + 7.5);
					PieceSprite.setScale(scaleX, scaleY);
					break;

				case 'N':
					PieceSprite.setTextureRect(IntRect(943, 0, 314, 302));
					PieceSprite.setPosition(x - 5, y + 7.5);
					PieceSprite.setScale(scaleX, scaleY);
					break;

				case 'R':
					PieceSprite.setTextureRect(IntRect(1257, 0, 314, 302));
					PieceSprite.setPosition(x - 10, y + 7.5);
					PieceSprite.setScale(scaleX, scaleY);
					break;

				case 'P':
					PieceSprite.setTextureRect(IntRect(1571, 0, 314, 302));
					PieceSprite.setPosition(x - 15, y + 7.5);
					PieceSprite.setScale(scaleX, scaleY);
					break;

				case 'k':
					PieceSprite.setTextureRect(IntRect(0, 303, 314, 302));
					PieceSprite.setPosition(x + 10, y);
					PieceSprite.setScale(scaleX, scaleY);
					break;

				case 'q':
					PieceSprite.setTextureRect(IntRect(315, 303, 314, 302));
					PieceSprite.setPosition(x + 5, y);
					PieceSprite.setScale(scaleX, scaleY);
					break;

				case 'b':
					PieceSprite.setTextureRect(IntRect(629, 303, 314, 302));
					PieceSprite.setPosition(x, y);
					PieceSprite.setScale(scaleX, scaleY);
					break;

				case 'n':
					PieceSprite.setTextureRect(IntRect(943, 303, 314, 302));
					PieceSprite.setPosition(x - 7.5, y);
					PieceSprite.setScale(scaleX, scaleY);
					break;

				case 'r':
					PieceSprite.setTextureRect(IntRect(1257, 303, 314, 302));
					PieceSprite.setPosition(x - 10, y);
					PieceSprite.setScale(scaleX, scaleY);
					break;

				case 'p':
					PieceSprite.setTextureRect(IntRect(1571, 303, 314, 302));
					PieceSprite.setPosition(x - 15, y);
					PieceSprite.setScale(scaleX, scaleY);
					break;
				
				default:
					break;


				}

				window.draw(PieceSprite);
			}
		}

		if (promotionState) {
			Sprite promotionSprite;
			promotionSprite.setTexture(PieceTextures);
			promotionSprite.setScale(scaleX, scaleY);

			if (whiteToMove) {
				promotionSprite.setTextureRect(IntRect(315,0,1256,302));
				promotionSprite.setPosition(400, 900);
			}
			else {
				promotionSprite.setTextureRect(IntRect(315, 303, 1256, 302));
				promotionSprite.setPosition(400, 0);
			}
			window.draw(promotionSprite);
		}

		if (board.isCheckmate(!whiteToMove)) {
			Text text;
			text.setFont(font);
			text.setCharacterSize(64);
			text.setFillColor(Color::Red);
			text.setPosition(260, 450);

			if (whiteToMove) text.setString("Black Wins!");
			else text.setString("White Wins!");
			window.draw(text);	
		}
		if (board.isStalemate(!whiteToMove)) {
			Text text;
			text.setFont(font);
			text.setCharacterSize(64);
			text.setFillColor(Color::Red);
			text.setPosition(250, 450);

			text.setString("Stalemate!");
			window.draw(text);
		}
		if (board.isInsufficientMaterial()) {
			Text text;
			text.setFont(font);
			text.setCharacterSize(48);
			text.setFillColor(Color::Red);
			text.setPosition(420, 400);

			text.setString("Draw by ");
			window.draw(text);

			text.setPosition(80, 500);
			text.setString("Insufficient Material");
			window.draw(text);
		}
		if (isDrawByRepetition()) {
			Text text;
			text.setFont(font);
			text.setCharacterSize(48);
			text.setFillColor(Color::Red);
			text.setPosition(420, 400);

			text.setString("Draw by ");
			window.draw(text);

			text.setPosition(350, 500);
			text.setString("Repetition");
			window.draw(text);
		}

		window.display();
	}

	bool isGameOver() {
		return board.isCheckmate(!whiteToMove) || board.isStalemate(!whiteToMove) || board.isInsufficientMaterial() || isDrawByRepetition();
	}


	void startGame(bool playWhite) {

		moveHistory.clear();
		positionHistory.clear();

		humanPlaysWhite = playWhite;
		whiteToMove = true;

		positionHistory.push_back(zobrist.hashPosition(board, false));

		colorSelection = false;
		selectedPiece = nullptr;
		highlightedMoves = 0;
		promotionState = false;

		
		if (!humanPlaysWhite) {
			display();
			makeEngineMove();
		}
	}


	void makeEngineMove() {

		// Do not move if the game has ended or it is the human's turn.
		if (colorSelection || isGameOver()) {
			return;
		}

		if (whiteToMove == humanPlaysWhite) {
			return;
		}

		// whiteToMove is true for White and false for Black.
		bool blackToMove = !whiteToMove;

		Move bestMove = engine.findBestMove(board, blackToMove, SEARCH_DEPTH, positionHistory, zobrist);

		// Check that the engine returned a valid move.
		if (bestMove.from < 0 || bestMove.from >= 64 || bestMove.to < 0 || bestMove.to >= 64) {
			return;
		}

		board.makeMove(bestMove);
		recordMove(bestMove);

		whiteToMove = !whiteToMove;
		selectedPiece = nullptr;
		highlightedMoves = 0;
	}

	void recordMove(const Move& move) {

		moveHistory.push_back(move);
		U64 hash = zobrist.hashPosition(board, !whiteToMove);
		positionHistory.push_back(hash);
	}

	bool isDrawByRepetition() {

		if (positionHistory.empty()) return false;

		U64 currentPosition = positionHistory.back();
		int repetitions = 0;

		for (int i = 0; i < positionHistory.size(); i++) {

			if (positionHistory[i] == currentPosition) {
				repetitions++;
			}
		}

		return repetitions >= 3;
	}
};