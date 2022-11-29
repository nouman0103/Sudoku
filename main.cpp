#include <iostream>
#include <string>
#include <time.h>
#include <fstream>
#include "SDL.h"
#include "SDL_image.h"

SDL_Texture *boardNumbers[9];
SDL_Texture *boardNumbersDefault[9];
SDL_Texture *board;
SDL_Texture *selectedRowTex;
SDL_Texture *selectedColTex;
SDL_Texture *cellHoverTex;

SDL_Texture *buttonsDefault[9];
SDL_Texture *buttonsHover[9];
SDL_Texture *buttonsPress[9];

SDL_Texture *menuButtonsDefault[3];
SDL_Texture *menuButtonsHover[3];
SDL_Texture *menuButtonsPress[3];

const int boardCoords[9] = {10, 84, 158, 243, 317, 391, 476, 550, 624};
const int buttonsCoords[9][2] = {{743, 207}, {843, 207}, {943, 207}, {743, 307}, {843, 307}, {943, 307}, {743, 407}, {843, 407}, {943, 407}};

SDL_Texture *slotButtonsDefault[3];
SDL_Texture *slotButtonHover[3];
SDL_Texture *slotButtonPress[3];
SDL_Texture *mistakes[4];
SDL_Texture *mistakeOverlay;

SDL_Texture *quitButton;
SDL_Texture *quitButtonHover;
SDL_Texture *quitButtonPress;

SDL_Texture *levelMenu;
SDL_Texture *levelButtonsDefault[3];
SDL_Texture *levelButtonsPress[3];
SDL_Texture *levelButtonsHover[3];

SDL_Texture *winPopup;
SDL_Texture *lostPopup;
SDL_Texture *backToMenuButton;
SDL_Texture *backToMenuButtonHover;
SDL_Texture *backToMenuButtonPress;

SDL_Texture *backButtonBasic;

SDL_Texture *emptySlotDefault;
SDL_Texture *emptySlotHover;
SDL_Texture *emptySlotPress;

// load audio from ./assets/sounds/cellSelect.mp3
SDL_AudioSpec cellSelectSpec;
Uint32 cellSelectLength;
Uint8 *cellSelectBuffer;
SDL_AudioDeviceID cellSelectDeviceId;

SDL_AudioSpec buttonDownSpec;
Uint32 buttonDownLength;
Uint8 *buttonDownBuffer;
SDL_AudioDeviceID buttonDownDeviceId;

SDL_AudioSpec buttonUpSpec;
Uint32 buttonUpLength;
Uint8 *buttonUpBuffer;
SDL_AudioDeviceID buttonUpDeviceId;

void drawBoard(
	SDL_Renderer *renderer,
	bool LeftClick, int &selectedRow,
	int &selectedCol, int gameMoves[][9], int mistakesCount, int gameInitialMoves[][9],
	bool mouseReleased, bool popupActive);
void loadSlot(int gameMoves[][9], int &slot, int &mistakes, int gameInitialMoves[][9]);
void saveSlot(int gameMoves[][9], int &slot, int &mistakes, int gameInitialMoves[][9]);
void resetSlot(int &slot);
void scanSlots();
void generateBoard(int gameMoves[][9], int gameInitialMoves[][9], int level);
bool validateCell(int gameMoves[][9], int row, int col, int num);
bool solveSudoku(int board[][9]);
bool findEmptyCell(int board[][9], int &row, int &col);
bool badMove(int board[][9], int row, int col, int num);
bool checkWin(int gameMoves[][9]);
void getMousePos(SDL_Renderer *renderer, int &x, int &y);
void buttonDownSound(){
	SDL_QueueAudio(buttonDownDeviceId, buttonDownBuffer, buttonDownLength);
	SDL_PauseAudioDevice(buttonDownDeviceId, 0);
}
void buttonUpSound() {
	SDL_QueueAudio(buttonUpDeviceId, buttonUpBuffer, buttonUpLength);
	SDL_PauseAudioDevice(buttonUpDeviceId, 0);
}


bool windowed = false;
int existingSaves[3] = { 0, 0, 0 };

int main(int argc, char *argv[])
{
	srand(time(NULL));
	//Initializing the window and the renderer
	SDL_Init(SDL_INIT_EVERYTHING);
		
	SDL_Init(SDL_INIT_AUDIO);
	//Initialize SDL_mixer
	
	SDL_Window *window = SDL_CreateWindow("Sudoku", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1055, 700, SDL_WINDOW_FULLSCREEN_DESKTOP);
	SDL_Rect boardRect;
	SDL_GetWindowSize(window, &boardRect.w, &boardRect.h);
	
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_RenderSetLogicalSize(renderer, 1055, 700);
	//enable anti aliasing
	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");

	// set window logo
	SDL_Surface *icon = IMG_Load("./assets/images/logo.png");
	SDL_SetWindowIcon(window, icon);
	SDL_FreeSurface(icon);

	SDL_LoadWAV("./assets/sounds/cellSelect.wav", &cellSelectSpec, &cellSelectBuffer, &cellSelectLength);
	cellSelectDeviceId =  SDL_OpenAudioDevice(NULL, 0, &cellSelectSpec, NULL, 0);

	SDL_LoadWAV("./assets/sounds/buttonDown.wav", &buttonDownSpec, &buttonDownBuffer, &buttonDownLength);
	buttonDownDeviceId =  SDL_OpenAudioDevice(NULL, 0, &buttonDownSpec, NULL, 0);

	SDL_LoadWAV("./assets/sounds/buttonUp.wav", &buttonUpSpec, &buttonUpBuffer, &buttonUpLength);
	buttonUpDeviceId =  SDL_OpenAudioDevice(NULL, 0, &buttonUpSpec, NULL, 0);

	// loading textures and assets
	board = IMG_LoadTexture(renderer, "./assets/images/board/board.png");
	selectedRowTex = IMG_LoadTexture(renderer, "./assets/images/board/selectedRow.png");
	selectedColTex = IMG_LoadTexture(renderer, "./assets/images/board/selectedCol.png");
	cellHoverTex = IMG_LoadTexture(renderer, "./assets/images/board/cellHover.png");
	
	//Board Buttons
	for(int i = 1 ; i <= 9; i++){
		std::string path = "./assets/images/buttons/board/default/b" + std::to_string(i) + ".png";
		buttonsDefault[i-1] = IMG_LoadTexture(renderer, path.c_str());
		path = "./assets/images/buttons/board/hover/b" + std::to_string(i) + ".png";
		buttonsHover[i-1] = IMG_LoadTexture(renderer, path.c_str());
		path = "./assets/images/buttons/board/press/b" + std::to_string(i) + ".png";
		buttonsPress[i-1] = IMG_LoadTexture(renderer, path.c_str());
	}

	//board numbers
	
	for(int i = 1 ; i <= 9; i++){
		std::string path = "./assets/images/board/numbers/user/" + std::to_string(i) + ".png";
		boardNumbers[i-1] = IMG_LoadTexture(renderer, path.c_str());
		path = "./assets/images/board/numbers/system/" + std::to_string(i) + ".png";
		boardNumbersDefault[i-1] = IMG_LoadTexture(renderer, path.c_str());

	}


	//Main Menu Assets
	SDL_Texture *mainMenu = IMG_LoadTexture(renderer, "./assets/images/menu/mainMenu.png");
	
	int menuButtonsCoords[3][2] = {{301, 247}, {301, 357}, {301, 467}};
	for(int i = 1 ; i <= 3; i++){
		std::string path = "./assets/images/buttons/mainMenu/default/b" + std::to_string(i) + ".png";
		menuButtonsDefault[i-1] = IMG_LoadTexture(renderer, path.c_str());
		path = "./assets/images/buttons/mainMenu/hover/b" + std::to_string(i) + ".png";
		menuButtonsHover[i-1] = IMG_LoadTexture(renderer, path.c_str());
		path = "./assets/images/buttons/mainMenu/press/b" + std::to_string(i) + ".png";
		menuButtonsPress[i-1] = IMG_LoadTexture(renderer, path.c_str());
	}

	// save menu assets
	SDL_Texture *saveMenu = IMG_LoadTexture(renderer, "./assets/images/menu/saveMenu.png");
	
	for(int i = 1 ; i <= 3; i++){
		std::string path = "./assets/images/buttons/saveMenu/default/b" + std::to_string(i) + ".png";
		slotButtonsDefault[i-1] = IMG_LoadTexture(renderer, path.c_str());
		path = "./assets/images/buttons/saveMenu/hover/b" + std::to_string(i) + ".png";
		slotButtonHover[i-1] = IMG_LoadTexture(renderer, path.c_str());
		path = "./assets/images/buttons/saveMenu/press/b" + std::to_string(i) + ".png";
		slotButtonPress[i-1] = IMG_LoadTexture(renderer, path.c_str());
	}

	for(int i = 0; i < 4; i++){
		std::string path = "./assets/images/board/mistakes/mistakes_" + std::to_string(i) + ".png";
		mistakes[i] = IMG_LoadTexture(renderer, path.c_str());
	}
	mistakeOverlay = IMG_LoadTexture(renderer, "./assets/images/board/mistakes/overlay.png");

	quitButton = IMG_LoadTexture(renderer, "./assets/images/buttons/board/default/quit.png");
	quitButtonHover = IMG_LoadTexture(renderer, "./assets/images/buttons/board/hover/quit.png");
	quitButtonPress = IMG_LoadTexture(renderer, "./assets/images/buttons/board/press/quit.png");

	levelMenu = IMG_LoadTexture(renderer, "./assets/images/menu/levelMenu.png");
	for(int i = 1 ; i <= 3; i++){
		std::string path = "./assets/images/buttons/mainMenu/default/lb" + std::to_string(i) + ".png";
		levelButtonsDefault[i-1] = IMG_LoadTexture(renderer, path.c_str());
		path = "./assets/images/buttons/mainMenu/hover/lb" + std::to_string(i) + ".png";
		levelButtonsHover[i-1] = IMG_LoadTexture(renderer, path.c_str());
		path = "./assets/images/buttons/mainMenu/press/lb" + std::to_string(i) + ".png";
		levelButtonsPress[i-1] = IMG_LoadTexture(renderer, path.c_str());
	}

	winPopup = IMG_LoadTexture(renderer, "./assets/images/menu/resultPopupWin.png");
	lostPopup = IMG_LoadTexture(renderer, "./assets/images/menu/resultPopupLost.png");
	backToMenuButton = IMG_LoadTexture(renderer, "./assets/images/buttons/board/default/backToMenu.png");
	backToMenuButtonHover = IMG_LoadTexture(renderer, "./assets/images/buttons/board/hover/backToMenu.png");
	backToMenuButtonPress = IMG_LoadTexture(renderer, "./assets/images/buttons/board/press/backToMenu.png");
	backButtonBasic = IMG_LoadTexture(renderer, "./assets/images/buttons/mainMenu/default/back.png");


	emptySlotDefault = IMG_LoadTexture(renderer, "./assets/images/buttons/saveMenu/default/empty.png");
	emptySlotHover = IMG_LoadTexture(renderer, "./assets/images/buttons/saveMenu/hover/empty.png");
	emptySlotPress = IMG_LoadTexture(renderer, "./assets/images/buttons/saveMenu/press/empty.png");

	bool mainMenuActive = true;
	bool saveMenuActive = false;
	bool boardActive = false;
	bool levelMenuActive = false;
	bool popupActive = false;


	bool LeftClick = false;
	int selectedRow = -1;
	int selectedCol = -1;
	int selectedSlot = -1;
	int gameMoves[9][9] = {-1};
	int gameInitialMoves[9][9] = {-1};
	int mistakesCount = 0;
	bool createNewGame = false;
	int mistakeOverlayAlpha = 0;
	int level = 1;

	
	for(int i = 0; i < 9; i++){
		for(int j = 0; j < 9; j++){
			gameInitialMoves[i][j] = -1;
		}
	}
	scanSlots();
	bool quit = false;
	bool gameWon = false;
	// the game loop
	while (!quit)
	{
		//check for events
		SDL_Event event;
		bool eventHappened = SDL_PollEvent(&event);
		bool mouseReleased = false;
		bool mousePressed = false;
		if (eventHappened)
		{
			if (event.type == SDL_QUIT)
				break;
			if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
			{
				LeftClick = true;
				mousePressed = true;
				
			}
			if (event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT)
			{
				LeftClick = false;
				mouseReleased = true;
			}
			// if f11 is pressed, toggle fullscreen
			if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_F11)
			{
				if (!windowed)
				{
					SDL_SetWindowFullscreen(window, 0);
					windowed = true;
				}
				else
				{
					SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
					windowed = false;
				}
			} 
		}

		// clear the frame
		SDL_RenderClear(renderer);
		SDL_Rect boardRect;
		boardRect.x = 0;
		boardRect.y = 0;
		boardRect.w = 1055;
		boardRect.h = 700;
		SDL_RenderCopy(renderer, board, NULL, &boardRect);
		if(mistakesCount >= 3){
			popupActive = true;
		}

		// draw stuff
		if(boardActive){
			drawBoard(
				renderer, LeftClick, selectedRow, selectedCol, gameMoves, mistakesCount,
				gameInitialMoves, mouseReleased, popupActive
			);
			for(int i = 0; i < 9; i++){
				int x, y;
				getMousePos(renderer, x, y);
				if(x >=  buttonsCoords[i][0] + 68/2 && x <=  buttonsCoords[i][0] + 68/2 + 56 && y >=  buttonsCoords[i][1] + 68/2 && y <=  buttonsCoords[i][1] + 68/2 + 56  && !popupActive){
					if(mouseReleased){
						buttonUpSound();
						//change the number of the cell to the number of the pressed button
						if(selectedRow != -1 && selectedCol != -1){
							// validate move
							if(validateCell(gameMoves, selectedRow, selectedCol, i+1) && !badMove(gameMoves, selectedRow, selectedCol, i+1)){
								gameMoves[selectedRow][selectedCol] = i+1;
								if(checkWin(gameMoves)){
									gameWon = true;
									popupActive = true;
								}
							}
							else{
								mistakesCount++;
								mistakeOverlayAlpha = 255;
							}
							saveSlot(gameMoves, selectedSlot, mistakesCount, gameInitialMoves);
						}
					}
					if(mousePressed){
						buttonDownSound();
					}
				}
			}
			// if mouse is released on quit button:
			int x, y;
			getMousePos(renderer, x, y);
			if(x >= 754+34 && x <= 754 + 298 - 34 && y >= 507 + 29 && y <= 507 + 117 - 29){
				if(mouseReleased){
					buttonUpSound();
					saveSlot(gameMoves, selectedSlot, mistakesCount, gameInitialMoves);
					popupActive = false;
					mainMenuActive = true;
					boardActive = false;
					saveMenuActive = false;
					levelMenuActive = false;
					createNewGame = false;
					gameWon = false;
					mistakesCount = 0;
					selectedSlot = 0;
					level = 0;
					for(int i = 0; i < 9; i++){
						for(int j = 0; j < 9; j++){
							gameMoves[i][j] = -1;
							gameInitialMoves[i][j] = -1;
						}
					}
					scanSlots();
				}
				if(mousePressed){
					buttonDownSound();
				}
			}
		}
		else if (mainMenuActive){
			SDL_RenderCopy(renderer, mainMenu, NULL, NULL);
			// draw menu buttons, check for clicks
			for(int i = 0; i < 3; i++){
				SDL_Rect buttonRect = {menuButtonsCoords[i][0], menuButtonsCoords[i][1], 448, 143};
				// if the mouse is hovering over the button, draw the hover texture
				int x, y;
				getMousePos(renderer, x, y);
				if(x > menuButtonsCoords[i][0] + 34 && x < menuButtonsCoords[i][0] + 448 - 34 && y > menuButtonsCoords[i][1] + 29 && y < menuButtonsCoords[i][1] + 143 - 29){
					if(LeftClick)
						SDL_RenderCopy(renderer, menuButtonsPress[i], NULL, &buttonRect);
					else SDL_RenderCopy(renderer, menuButtonsHover[i], NULL, &buttonRect);
					//if mouse button is released, change the state of the game
					if(event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT){
						buttonUpSound();
						if(i == 0){
							mainMenuActive = false;
							saveMenuActive = false;
							levelMenuActive = true;
							createNewGame = true;
						}
						else if(i == 1){
							mainMenuActive = false;
							saveMenuActive = true;
							createNewGame = false;
						}
						else if(i == 2){
							quit = true;
						}
					}
					if(mousePressed){
						buttonDownSound();
					}
				}
				else{
					SDL_RenderCopy(renderer, menuButtonsDefault[i], NULL, &buttonRect);
				}
				
			}
		}
		else if(saveMenuActive){
			SDL_RenderCopy(renderer, saveMenu, NULL, NULL);
			if(!createNewGame){
				SDL_RenderCopy(renderer, backButtonBasic, NULL, NULL);
				// if mouse is pressed on backButtonBasic, go back to main menu
				int x, y;
				getMousePos(renderer, x, y);
				if(x >= 51 && x <= 51 + 74 && y >= 51 && y <= 51 + 66){
					if(mouseReleased){
						buttonUpSound();
						saveMenuActive = false;
						mainMenuActive = true;
					}
					if(mousePressed){
						buttonDownSound();
					}
				}
			}
			// draw save menu buttons, check for clicks
			for(int i = 0; i < 3; i++){
				bool saveExists = existingSaves[i];

				SDL_Rect buttonRect = {menuButtonsCoords[i][0], menuButtonsCoords[i][1], 448, 143};
				int x, y;
				getMousePos(renderer, x, y);
				if(!(saveExists == false && createNewGame == false) && x > menuButtonsCoords[i][0] + 34 && x < menuButtonsCoords[i][0] + 448 - 34 && y > menuButtonsCoords[i][1] + 29 && y < menuButtonsCoords[i][1] + 143 - 29){
					if(LeftClick)
						SDL_RenderCopy(renderer, saveExists?slotButtonPress[i]:emptySlotPress, NULL, &buttonRect);
					else SDL_RenderCopy(renderer, saveExists?slotButtonHover[i]:emptySlotHover, NULL, &buttonRect);
					//if mouse button is released, change the state of the game
					if(event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT){
						buttonUpSound();
						selectedSlot = i;
						if(createNewGame){
							generateBoard(gameMoves, gameInitialMoves, level);
						}
						else{
							loadSlot(gameMoves, selectedSlot, mistakesCount, gameInitialMoves);
						}
						saveSlot(gameMoves, selectedSlot, mistakesCount, gameInitialMoves);
						boardActive = true;
						saveMenuActive = false;
					}
					if(mousePressed){
						buttonDownSound();
					}
				}
				else{
					if(saveExists == false && createNewGame == false){
						SDL_SetTextureAlphaMod(emptySlotDefault, 200);
					}else{
						SDL_SetTextureAlphaMod(emptySlotDefault, 255);
					}
					SDL_RenderCopy(renderer, saveExists?slotButtonsDefault[i]:emptySlotDefault, NULL, &buttonRect);
				}
			}
		}
		else if(levelMenuActive){
			SDL_RenderCopy(renderer, levelMenu, NULL, NULL);
			// draw level menu buttons, check for clicks, and change the state of the game
			for(int i = 0; i < 3; i++){
				SDL_Rect buttonRect = {menuButtonsCoords[i][0], menuButtonsCoords[i][1], 448, 143};
				int x, y;
				getMousePos(renderer, x, y);
				if(x > menuButtonsCoords[i][0] + 34 && x < menuButtonsCoords[i][0] + 448 - 34 && y > menuButtonsCoords[i][1] + 29 && y < menuButtonsCoords[i][1] + 143 - 29){
					if(LeftClick)
						SDL_RenderCopy(renderer, levelButtonsPress[i], NULL, &buttonRect);
					else SDL_RenderCopy(renderer, levelButtonsHover[i], NULL, &buttonRect);
					//if mouse button is released, change the state of the game
					if(event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT){
						buttonUpSound();
						level = i + 1;
						levelMenuActive = false;
						saveMenuActive = true;
					}
					if(mousePressed){
						buttonDownSound();
					}
				}
				else{
					SDL_RenderCopy(renderer, levelButtonsDefault[i], NULL, &buttonRect);
				}
				
			}
		}

		// draw mistakeOverlay with alpha value of mistakeOverlayAlpha
		SDL_SetTextureAlphaMod(mistakeOverlay, mistakeOverlayAlpha);
		SDL_RenderCopy(renderer, mistakeOverlay, NULL, NULL);
		if(mistakeOverlayAlpha > 0){
			mistakeOverlayAlpha -= 0.1;
		}

		// draw popup
		if(popupActive){
			if(gameWon){
				SDL_RenderCopy(renderer, winPopup, NULL, NULL);
			}
			else{
				SDL_RenderCopy(renderer, lostPopup, NULL, NULL);
			}
			resetSlot(selectedSlot);
			SDL_Rect buttonRect = {306, 333, 443, 152};
			int x, y;
			getMousePos(renderer, x, y);
			if(x > 306 + 34 && x < 306 + 443 - 34 && y > 333 + 29 && y < 333 + 143 - 29){
				if(LeftClick)
					SDL_RenderCopy(renderer, backToMenuButtonPress, NULL, &buttonRect);
				else SDL_RenderCopy(renderer, backToMenuButtonHover, NULL, &buttonRect);
				if(event.type == SDL_MOUSEBUTTONUP && event.button.button == SDL_BUTTON_LEFT){
					buttonUpSound();
					popupActive = false;
					mainMenuActive = true;
					boardActive = false;
					saveMenuActive = false;
					levelMenuActive = false;
					createNewGame = false;
					gameWon = false;
					mistakesCount = 0;
					selectedSlot = 0;
					level = 0;
					for(int i = 0; i < 9; i++){
						for(int j = 0; j < 9; j++){
							gameMoves[i][j] = -1;
							gameInitialMoves[i][j] = -1;
						}
					}
					scanSlots();
				}
				if(mousePressed){
					buttonDownSound();
				}
			}
			else{
				SDL_RenderCopy(renderer, backToMenuButton, NULL, &buttonRect);
			}

		}


		SDL_RenderPresent(renderer);


	}
	// end the game
	SDL_DestroyTexture(board);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();

	
	
	return 0;
}


void drawBoard(SDL_Renderer *renderer, bool LeftClick, int &selectedRow, int &selectedCol, int gameMoves[][9], int mistakesCount, int gameInitialMoves[][9], bool mouseReleased, bool popupActive){
	for(int i = 0; i < 9; i++){
		SDL_Rect rect;
		rect.x = buttonsCoords[i][0];
		rect.y = buttonsCoords[i][1];
		rect.w = 124;
		rect.h = 124;


		int x, y;
		getMousePos(renderer, x, y);
		
		if(x >= rect.x + 68/2 && x <= rect.x + 68/2 + 56 && y >= rect.y + 68/2 && y <= rect.y + 68/2 + 56 && !popupActive){
			if(LeftClick){
				SDL_RenderCopy(renderer, buttonsPress[i], NULL, &rect);
			}else{
				SDL_RenderCopy(renderer, buttonsHover[i], NULL, &rect);
			}
			
		}
		else{
			SDL_RenderCopy(renderer, buttonsDefault[i], NULL, &rect);
		}

		
	}

	//draw mistakes
	SDL_Rect rect;
	rect.x = 809;
	rect.y = 152;
	rect.w = 199;
	rect.h = 30;
	SDL_RenderCopy(renderer, mistakes[mistakesCount], NULL, &rect);
	// check if mouse is over a cell
	if (mouseReleased) {
		int x, y;
		getMousePos(renderer, x, y);
		for (int i = 0; i < 9; i++) {
			for (int j = 0; j < 9; j++) {
				if(gameInitialMoves[j][i] != -1) continue;
				if (x >= boardCoords[i] && x <= boardCoords[i] + 68 && y >= boardCoords[j] && y <= boardCoords[j] + 68 && !popupActive) {
					if(selectedRow == j && selectedCol == i){
						selectedRow = -1;
						selectedCol = -1;
						break;
					}
					SDL_QueueAudio(cellSelectDeviceId, cellSelectBuffer, cellSelectLength);
					SDL_PauseAudioDevice(cellSelectDeviceId, 0);
					selectedRow = j;
					selectedCol = i;
					std::cout << "selected row: " << selectedRow << " selected col: " << selectedCol << std::endl;
				}
			}
		}
	}

	// if a cell is selected, apply the selectedRow texture on the row and selectedCol texture on the column
	if (selectedRow != -1) {
		SDL_Rect rect;
		rect.x = 10;
		rect.y = boardCoords[selectedRow];
		rect.w = 680;
		rect.h = 66;
		SDL_RenderCopy(renderer, selectedRowTex, NULL, &rect);
	}
	if (selectedCol != -1) {
		SDL_Rect rect;
		rect.x = boardCoords[selectedCol];
		rect.y = 10;
		rect.w = 66;
		rect.h = 680;
		SDL_RenderCopy(renderer, selectedColTex, NULL, &rect);
	}

	// if a cell is hovered, apply the cellHover texture on it
	int x, y;
	getMousePos(renderer, x, y);
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			if(gameInitialMoves[j][i] != -1) continue;
			if (x >= boardCoords[i] && x <= boardCoords[i] + 68 && y >= boardCoords[j] && y <= boardCoords[j] + 68 && !popupActive) {
				SDL_Rect rect;
				rect.x = boardCoords[i];
				rect.y = boardCoords[j]; 
				rect.w = 66;
				rect.h = 66;
				SDL_RenderCopy(renderer, cellHoverTex, NULL, &rect);
			}
		}
	}

	// draw the numbers on the board
	for (int i = 0; i < 9; i++) {
		for (int j = 0; j < 9; j++) {
			if (gameMoves[i][j] != -1 && gameMoves[i][j] != 0) {
				SDL_Rect rect;
				rect.x = boardCoords[j]; // j is column, which changes the x coord
				rect.y = boardCoords[i];
				rect.w = 66;
				rect.h = 66;
				SDL_RenderCopy(renderer, ((gameInitialMoves[i][j]==-1)?boardNumbers:boardNumbersDefault)[gameMoves[i][j] - 1], NULL, &rect);
			}
		}
	}
	// draw the quit button, if the mouse is over it, draw the hover texture
	// if the mouse is clicked, draw the press texture
	SDL_Rect rect2;
	rect2.x = 754;
	rect2.y = 507;
	rect2.w = 298;
	rect2.h = 117;
	getMousePos(renderer, x, y);
	if (x >= 754+34 && x <= 754 + 298 - 34 && y >= 507 + 29 && y <= 507 + 117 - 29 && !popupActive) {
		if (LeftClick) {
			SDL_RenderCopy(renderer, quitButtonPress, NULL, &rect2);
		}
		else {
			SDL_RenderCopy(renderer, quitButtonHover, NULL, &rect2);
		}

	}
	else {
		SDL_RenderCopy(renderer, quitButton, NULL, &rect2);
	}


}

bool validateCell(int board[][9], int row, int col, int num){
	// check if the number is already in the row
	for(int i = 0; i < 9; i++){
		if(board[row][i] == num){
			return false;
		}
	}
	// check if the number is already in the column
	for(int i = 0; i < 9; i++){
		if(board[i][col] == num){
			return false;
		}
	}
	// check if the number is already in the 3x3 box
	int boxRow = row - row % 3;
	int boxCol = col - col % 3;
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 3; j++){
			if(board[boxRow + i][boxCol + j] == num){
				return false;
			}
		}
	}
	return true;
}

bool badMove(int board[][9], int row, int col, int num){
	int tempBoard[9][9];
	for(int i = 0; i < 9; i++){
		for(int j = 0; j < 9; j++){
			tempBoard[i][j] = board[i][j];
		}
	}
	tempBoard[row][col] = num;
	if(!solveSudoku(tempBoard)){ // if the board is not solvable
		std::cout << "bad move" << std::endl;
		return true;
	}
	return false;
}

bool solveSudoku(int board[][9]){
	int row, col;
	if(!findEmptyCell(board, row, col)){
		return true;
	}
	for(int i = 1; i <= 9; i++){
		if(validateCell(board, row, col, i)){
			board[row][col] = i;
			if(solveSudoku(board)){
				return true;
			}
			board[row][col] = -1;
		}
	}
	return false;
}

bool findEmptyCell(int board[][9], int &row, int &col){
	for(row = 0; row < 9; row++){
		for(col = 0; col < 9; col++){
			if(board[row][col] == -1){
				return true;
			}
		}
	}
	return false;
}


void shuffleGivenRows(int board[][9], int index1, int index2, int index3){
	//index1, index2, index3 are the rows to be shuffled
	int temp[9];
	for(int i = 0; i < 9; i++){
		temp[i] = board[index1][i];
	}
	for(int i = 0; i < 9; i++){
		board[index1][i] = board[index2][i];
	}
	for(int i = 0; i < 9; i++){
		board[index2][i] = board[index3][i];
	}
	for(int i = 0; i < 9; i++){
		board[index3][i] = temp[i];
	}
	// theres a 90% chance that the rows will be shuffled again
	if(rand() % 10 < 9){
		shuffleGivenRows(board, index1, index2, index3);
	}
}

void shuffleRows(int board[][9]){
	// a random number, 1, 2, or 3
	// if number is 1, shuffle rows 1, 2, and 3
	// if number is 2, shuffle rows 4, 5, and 6
	// if number is 3, shuffle rows 7, 8, and 9
	while(true){
		int num = rand() % 3 + 1;
		if(num == 1){
			shuffleGivenRows(board, 0, 1, 2);
		}
		else if(num == 2){
			shuffleGivenRows(board, 3, 4, 5);
		}
		else{
			shuffleGivenRows(board, 6, 7, 8);
		}
		// theres a 5% chance that the rows will stop being shuffled
		if(rand() % 100 < 5){
			break;
		}
	}
}

void shuffleGivenColumns(int board[][9], int index1, int index2, int index3){
	//index1, index2, index3 are the columns to be shuffled
	int temp[9];
	for(int i = 0; i < 9; i++){
		temp[i] = board[i][index1];
	}
	for(int i = 0; i < 9; i++){
		board[i][index1] = board[i][index2];
	}
	for(int i = 0; i < 9; i++){
		board[i][index2] = board[i][index3];
	}
	for(int i = 0; i < 9; i++){
		board[i][index3] = temp[i];
	}
	// theres a 90% chance that the columns will be shuffled again
	if(rand() % 10 < 9){
		shuffleGivenColumns(board, index1, index2, index3);
	}
}
void shuffleColumns(int board[][9]){
	// a random number, 1, 2, or 3
	// if number is 1, shuffle columns 1, 2, and 3
	// if number is 2, shuffle columns 4, 5, and 6
	// if number is 3, shuffle columns 7, 8, and 9
	while(true){
		int num = rand() % 3 + 1;
		if(num == 1){
			shuffleGivenColumns(board, 0, 1, 2);
		}
		else if(num == 2){
			shuffleGivenColumns(board, 3, 4, 5);
		}
		else{
			shuffleGivenColumns(board, 6, 7, 8);
		}
		// theres a 5% chance that the columns will stop being shuffled
		if(rand() % 100 < 5){
			break;
		}
	}
}
void shuffleNonet(int board[][9]){
	//put row 4,5,6 at row 1,2,3
	// put row 7,8,9 at row 4,5,6
	// put row 1,2,3 at row 7,8,9
	int temp[9][9];
	for(int i = 0; i < 9; i++){
		for(int j = 0; j < 9; j++){
			temp[i][j] = board[i][j];
		}
	}
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 9; j++){
			board[i][j] = temp[i + 3][j];
		}
	}
	for(int i = 3; i < 6; i++){
		for(int j = 0; j < 9; j++){
			board[i][j] = temp[i + 3][j];
		}
	}
	for(int i = 6; i < 9; i++){
		for(int j = 0; j < 9; j++){
			board[i][j] = temp[i - 6][j];
		}
	}
	// theres a 90% chance that the nonets will be shuffled again
	if(rand() % 10 < 9){
		shuffleNonet(board);
	}
}

void transpose(int board[][9]){
	int temp[9][9];
	for(int i = 0; i < 9; i++){
		for(int j = 0; j < 9; j++){
			temp[i][j] = board[i][j];
		}
	}
	for(int i = 0; i < 9; i++){
		for(int j = 0; j < 9; j++){
			board[i][j] = temp[j][i];
		}
	}
}

void generateBoard(int gameMoves[][9], int gameInitial[][9], int level){
	for(int i = 0; i < 9; i++){
		for(int j = 0; j < 9; j++){
			gameMoves[i][j] = -1;
		}
	}
	solveSudoku(gameMoves);
	for(int i = 0; i < 1000; i++){
		// shuffle the rows or columns or transpose
		shuffleRows(gameMoves);
		shuffleColumns(gameMoves);
		transpose(gameMoves);
		shuffleNonet(gameMoves);
		// 1 ms delay 
		if(i%50==0) SDL_Delay(1);
		
	}

	int numsToRemove;
	if(level == 1){
		numsToRemove = 39;
	}
	else if(level == 2){
		numsToRemove = 52;
	}
	else{
		numsToRemove = 61;
	}

	for(int i = 0; i < numsToRemove; i++){
		int row = rand() % 9;
		int col = rand() % 9;
		gameMoves[row][col] = -1;
	}

	for(int i = 0; i < 9; i++){
		for(int j = 0; j < 9; j++){
			gameInitial[i][j] = gameMoves[i][j];
		}
	}
	

}

bool checkWin(int gameMoves[][9]){
	for(int i = 0; i < 9; i++){
		for(int j = 0; j < 9; j++){
			if(gameMoves[i][j] == -1){
				return false;
			}
		}
	}
	return true;
}

void loadSlot(int gameMoves[][9], int &slot, int &mistakesCount, int gameInitialMoves[][9]){
	/*
File should store the mistakes made, the moves made, and the board
File format:
-1+2+3-4+5+6+7+8+0
+1+2-3+4+5+6-7+8-9
+1+2+3+4+5+6+7+8+9
+1+2+3+4+5+6+7+8+9
+1+2+3+4+5+6+7+8+9
+1+2+3+4+5+6+7+8+9
+1+2+3+4+5+6+7+8+9
+1+2+3+4+5+6+7+8+9
mistakes: 0

+ve number means the number is given by user
-ve number means the number is defined by computer
0 for empty space
mistakes can be 0, 1, or 2
	*/
	std::ifstream file;
	file.open("./assets/saves/slot" + std::to_string(slot+1) + ".sav");
	if(file.is_open()){
		std::string line;
		int row = 0;
		while(std::getline(file, line)){
			if(line == "mistakes: 0"){
				mistakesCount = 0;
			}
			else if(line == "mistakes: 1"){
				mistakesCount = 1;
			}
			else if(line == "mistakes: 2"){
				mistakesCount = 2;
			}
			else{
				for(int i = 0; i < 18; i++){
					if(line[i] == '+'){
						gameMoves[row][i/2] = line[i+1] - '0';
					}
					else if(line[i] == '-'){
						gameMoves[row][i/2] = line[i+1] - '0';
						gameInitialMoves[row][i/2] = line[i+1] - '0';
					}
					if(gameMoves[row][i/2] == 0){
						gameMoves[row][i/2] = -1;
						gameInitialMoves[row][i/2] = -1;
					}
				}
				row++;
			}
		}
		file.close();
	}
	else{
		std::cout << "Unable to open file" << std::endl;
	}
	
}

void saveSlot(int gameMoves[][9], int &slot, int &mistakesCount, int gameInitialMoves[][9]){
	std::ofstream file;
	file.open("./assets/saves/slot" + std::to_string(slot+1) + ".sav");
	if(file.is_open()){
		for(int i = 0; i < 9; i++){
			for(int j = 0; j < 9; j++){
				if(gameInitialMoves[i][j] == -1){
					file << "+" << ((gameMoves[i][j]==-1)?0:gameMoves[i][j]);
				}
				else{
					file << "-" << gameMoves[i][j];
				}
			}
			file << std::endl;
		}
		file << "mistakes: " << mistakesCount;
		file.close();
	}
	else{
		std::cout << "Unable to open file" << std::endl;
	}
}

void scanSlots(){
	std::ifstream saveFile;
	for(int i = 1; i <= 3; i++){
		std::string path = "./assets/saves/slot" + std::to_string(i) + ".sav";
		saveFile.open(path);
		// if file has content, then it is an existing save
		if(saveFile.peek() != std::ifstream::traits_type::eof()){
			existingSaves[i-1] = 1;
		}else{
			existingSaves[i-1] = 0;
		}
		saveFile.close();
	}
}

void resetSlot(int &slot){
	std::ofstream file;
	file.open("./assets/saves/slot" + std::to_string(slot+1) + ".sav");
	if(file.is_open()){
		// delete file contents
		file << "";
		file.close();
	}
	else{
		std::cout << "Unable to open file" << std::endl;
	}
}

void getMousePos(SDL_Renderer *renderer, int &x, int &y){
	if(!windowed){
		int x1, y1;
		SDL_GetGlobalMouseState(&x1, &y1);
		float x2, y2;
		SDL_RenderWindowToLogical(renderer, x1, y1, &x2, &y2);
		x = x2;
		y = y2;
	}
	else{
		SDL_GetMouseState(&x, &y);
	}
}
