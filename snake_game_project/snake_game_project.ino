/* BUGS:
  -Button debounce in submenu doesn't work properly
  -Sound ON/OFF function not implemented correctly
  -LCD turns off during gameplay
*/

#include <EEPROM.h>
#include <LiquidCrystal.h>
#include "LedControl.h"

// LCD Display variables
const byte rs = 9;
const byte en = 8;
const byte d4 = 7;
const byte d5 = 6;
const byte d6 = 5;
const byte d7 = 4;
const byte lcdBacklightPin = 3;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// Joystick variables
const int joystickButtonPin = 2;
const int xPin = A0;
const int yPin = A1;

// Thresholds for detecting joystick movement
const int minThreshold = 200;
const int maxThreshold = 600;
bool joyMoved = false;

// Menu variables
const int optionsPerPage = 2;
String menuItems[] = {"Start Game", "Settings", "About"};
String submenuItems[] = {"LCD Brightness", "Matrix Brightness", "Sound", "Back"}; // SOUND ON/OFF
int menuPage = 0;
int maxMenuPages = round(((sizeof(menuItems) / sizeof(String)) / 2) + .5);
int cursorPosition = 0;
int submenuPage = 0;
int maxSubmenuPages = round(((sizeof(submenuItems) / sizeof(String)) / 2) + .5);


//LCD Brightness control variables
int lcdBrightness;
int eepromAddress_lcdBrightness = 0;  // EEPROM address for storing/loading LCD brightness
int eepromAddress_matrixBrightness = 1;


// OTHER VARIABLES
const int debounceDelay = 50; // Adjust this value as needed
unsigned long lastButtonPress = 0;
unsigned long introMessageStartTime;

const int buzzerPin = 13;
byte arrowPosition = 0; // 0 represents "ON", 1 represents "OFF"

//---------------------SNAKE---------------------------------
const int dinPin = 12;
const int clockPin = 11;
const int loadPin = 10;

// Create an LedControl object to manage the LED matrix
LedControl lc = LedControl(dinPin, clockPin, loadPin, 1); // DIN, CLK, LOAD, No. DRIVER

int matrixBrightness = 2;// Variable to set the brightness level of the matrix
const byte matrixSize = 8 ;// Size of the LED matrix

// Variables to track the current and previous positions of the joystick-controlled LED
byte snakeLastCol = 0;
byte snakeLastRow = 0;

// 2D array representing the state of each LED (on/off) in the matrix
byte matrix[matrixSize][matrixSize] = {
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0},
  {0, 0, 0, 0, 0, 0, 0, 0}
};

// OTHER VARIABLES
int foodCol = -1;
int foodRow = -1;
unsigned long blinkingRate = 500; 

bool win = false; 
bool lose = false;

const int snakeLengthInit = 2; // initial snake length
int snakeLength = snakeLengthInit;
int snakeCol;
int snakeRow;
bool matrixChanged = false;
int snakeBody[8][8] = {}; // created an empty matrix to store the snake body
int snakeSpeed = 1;
int snakeDirection = 0; // 0: UP, 1: RIGHT, 2: DOWN, 3: LEFT

int buzzerTone = 1000;

// Notes and durations of songs
int sadSongNotes[] = {262, 196, 175, 131, 196, 175, 131, 196, 262};
int sadSongDurations[] = {400, 200, 200, 400, 200, 200, 400, 200, 400};
int happySongNotes[] = {262, 330, 392, 330, 349, 294};
int happySongDurations[] = {200, 200, 200, 200, 200, 400};

unsigned long delaySpeed = 300 / snakeSpeed;
bool playSound = true;
int eeprom_HighScore = 2;// EEPROM address to store the high score
bool gamePlaying;


// Creates 3 custom characters for the menu display
byte downArrow[8] = {
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b10101, // * * *
  0b01110, //  ***
  0b00100  //   *
};

byte upArrow[8] = {
  0b00100, //   *
  0b01110, //  ***
  0b10101, // * * *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100, //   *
  0b00100  //   *
};

byte menuCursor[8] = {
  B01000, //  *
  B00100, //   *
  B00010, //    *
  B00001, //     *
  B00010, //    *
  B00100, //   *
  B01000, //  *
  B00000  //
};


void setup() {
  Serial.begin(9600);
  // Set up the LCD's number of columns and rows:
  lcd.begin(16, 2);
  lcd.clear();

  pinMode(joystickButtonPin, INPUT_PULLUP); // activate pull-up resistor
  
  introMessageStartTime = millis();

  // Creates the byte for the 3 custom characters
  lcd.createChar(0, menuCursor);
  lcd.createChar(1, upArrow);
  lcd.createChar(2, downArrow);

  lcdBrightness = EEPROM.read(eepromAddress_lcdBrightness);
  analogWrite(lcdBacklightPin, lcdBrightness);

  matrixBrightness = EEPROM.read(eepromAddress_matrixBrightness);

  //------------------------------SNAKE---------------------
  lc.shutdown(0, false); // turn off power saving, enables display
  lc.setIntensity(0, matrixBrightness); // sets brightness (0~15 possible values)
  lc.clearDisplay(0); // Clear the matrix display

  randomSeed(analogRead(A5));
  snakeCol = random(8); // generate the snake in a random position
  snakeRow = random(8);
}

void loop() {
  // Check if the intro message should still be displayed
  if (millis() - introMessageStartTime < 3000) {
    // Display intro message
    lcd.setCursor(0, 0);
    lcd.print("Welcome to Your");
    lcd.setCursor(0, 1);
    lcd.print("Project!");
  } else {
    // The intro message has been displayed for 3 seconds, proceed with the main loop
    mainMenuDraw();
    drawCursor();
    operateMainMenu();
  }
}


// This function will generate the 2 menu items that can fit on the screen. 
//They will change as you scroll through your menu. Up and down arrows will indicate your current menu position.
void mainMenuDraw() {
  Serial.print(menuPage);
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(menuItems[menuPage]);
  lcd.setCursor(1, 1);
  lcd.print(menuItems[menuPage + 1]);
  if (menuPage == 0) { // User can only scroll down
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
  } else if (menuPage > 0 and menuPage < maxMenuPages-1) { // User can scroll up & down
    lcd.setCursor(15, 1);
    lcd.write(byte(2));
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  } else if (menuPage == maxMenuPages-1) { // User can only scroll up
    lcd.setCursor(15, 0);
    lcd.write(byte(1));
  }
}


// When called, this function will erase the current cursor and redraw it based on the cursorPosition and menuPage variables.
void drawCursor() {
  for (int i = 0; i < 2; i++) {
    lcd.setCursor(0, i);
    lcd.print(" ");
  }

  // The menu is set up to be progressive (menuPage 0 = Item 1 & Item 2, menuPage 1 = Item 2 & Item 3, menuPage 2 = Item 3 & Item 4), so
  // in order to determine where the cursor should be you need to see if you are at an odd or even menu page and an odd or even cursor position.
  if (menuPage % 2 == 0) {
    if (cursorPosition % 2 == 0) {  // If the menu page is even and the cursor position is even that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    } else { // If the menu page is even and the cursor position is odd that means the cursor should be on line 2
        lcd.setCursor(0, 1);
        lcd.write(byte(0));
    }
  } else {
    if (cursorPosition % 2 == 0) {  // If the menu page is odd and the cursor position is even that means the cursor should be on line 2
      lcd.setCursor(0, 1);
      lcd.write(byte(0));
    } else {  // If the menu page is odd and the cursor position is odd that means the cursor should be on line 1
      lcd.setCursor(0, 0);
      lcd.write(byte(0));
    }
  }
}


void operateMainMenu() {
  int activeButton = 0;
  int prevJoystickDirection = 0;

  while (activeButton == 0){
    int joystickX = analogRead(xPin);
    int joystickDirection = evaluateJoystickDirection(joystickX);

    if(joystickDirection != prevJoystickDirection){
      switch (joystickDirection) {
        case 0:
          break;
        case 1:  // Joystick moved up
          if (cursorPosition > 0) {
            cursorPosition--;
          } else if (menuPage > 0) {
            menuPage--;
            cursorPosition = 1;  // Move to the second item on the new page
          }
          break;
        case 2:  // Joystick moved down
          if (cursorPosition < 1 && menuPage < maxMenuPages - 1) {
            cursorPosition++;
          } else if (cursorPosition == 1 && menuPage < maxMenuPages - 1) {
            menuPage++;
            cursorPosition = 0;  // Move to the first item on the new page
          }
          break;
      }

      mainMenuDraw();
      drawCursor();
    }
    
     //Check if the button is pressed
    if (digitalRead(joystickButtonPin) == LOW && millis() - lastButtonPress > debounceDelay) {
      Serial.println("Button pressed!"); //TEST
      lastButtonPress = millis();
      handleMenuSelection(menuPage * optionsPerPage + cursorPosition); // Joystick button is pressed, select the current menu option
      activeButton = 1;
      mainMenuDraw();
      drawCursor();
    }

    prevJoystickDirection = joystickDirection;
  }
}



int evaluateJoystickDirection(int x) {
  if (x > maxThreshold) {
    return 1;  // Joystick moved up
  } else if (x < minThreshold) {
    return 2;  // Joystick moved down
  }

  return 0;  // No movement
}



// Handle the selection logic based on the selected menu option
void handleMenuSelection(int selectedOption) {
  switch (selectedOption) {
    case 0:
      menuItem1();
      break;
    case 1:
      menuItem2();
      break;
    case 2:
      menuItem3();
      break;
  }
}

// START GAME
void menuItem1() {
  lcd.clear();
  lcd.print("Starting Game...");
  delay(1000);
  lcd.clear();
  gamePlaying = true;
  while(gamePlaying == true){
    generateFood();
	  snakeMove();
	  handleGameStates();
  }
}



// START GAME
void menuItem2() {
  cursorPosition = 0;
  submenuPage = 0;  // Reset submenuPage

  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.print(submenuItems[submenuPage]);
  lcd.setCursor(1, 1);
  lcd.print(submenuItems[submenuPage + 1]);

  drawCursor();

  int activeButton = 0;
  int prevJoystickDirection = 0;

  while (activeButton == 0) {
    int joystickX = analogRead(xPin);
    int joystickDirection = evaluateJoystickDirection(joystickX);

    if (joystickDirection != prevJoystickDirection) {
      switch (joystickDirection) {
        case 0:
          break;
        case 1:  // Joystick moved up
          if (cursorPosition > 0) {
            cursorPosition--;
          } else if (submenuPage > 0) {
            submenuPage--;
            cursorPosition = 1;  // Move to the second item on the new page
          }
          break;
        case 2:  // Joystick moved down
          if (cursorPosition < 1 && submenuPage < maxSubmenuPages) {
            cursorPosition++;
          } else if (cursorPosition == 1 && submenuPage < maxSubmenuPages - 1) {
            submenuPage++;
            cursorPosition = 0;  // Move to the first item on the new page
          }
          break;
      }

      lcd.clear();
      lcd.setCursor(1, 0);
      lcd.print(submenuItems[submenuPage]);
      lcd.setCursor(1, 1);
      lcd.print(submenuItems[submenuPage + 1]);
      drawCursor();
    }

    // Check if the button is pressed
    if (digitalRead(joystickButtonPin) == LOW && millis() - lastButtonPress > debounceDelay) {
      Serial.println("Button pressed!"); // TEST
      lastButtonPress = millis();
      
        // Handle other submenu selections
        handleSubmenuSelection(submenuPage * optionsPerPage + cursorPosition);
        lcd.clear();
        lcd.setCursor(1, 0);
        lcd.print(submenuItems[submenuPage]);
        lcd.setCursor(1, 1);
        lcd.print(submenuItems[submenuPage + 1]);
        drawCursor();
      }

    // Set activeButton to 1 if "Back"
    activeButton = (submenuItems[submenuPage + cursorPosition] == "Back") ? 1 : 0;
    prevJoystickDirection = joystickDirection;
  }
}



// ABOUT
void menuItem3(){
  int numRows = 4;
  String about[] = {"Game Name: Mini Matrix Game", "Author: Ruka Mirela", "GitHub: github.com/Mirela89"};
  lcd.clear();

  for (int i = 0; i < numRows - 1; i++) {
    lcd.setCursor(0, 0);
    lcd.print("About");
    lcd.setCursor(0, 1);
    lcd.print(about[i]);
    delay(1000);

    for (int positionCounter = 0; positionCounter < 13; positionCounter++) {
      lcd.scrollDisplayLeft();
      delay(150);
    }

    // If it's not the last line, delay and clear the line
    if (i < numRows - 2) {
      delay(1000);
      lcd.setCursor(0, i);
      lcd.print("                ");  // Clear the line
    }
    lcd.clear();
  }
  delay(1000);  // Add an extra delay after the last line for better readability
}


void handleSubmenuSelection(int selectedOption) {
  switch (selectedOption) {
    case 0: // LCD Brightness Control, Save it to eeprom
      lcdBrightnessControl();
      break;
    case 1: // Matrix brightness control,  Make sure to display something on the matrix when selecting it. Save it to eeprom.
      matrixBrightnessControl();
      break;
    case 2: // SOUND on or off. Save it to eeprom
      soundControl();
      break;
  }
}

void soundControl() {
  lcd.clear();
  lcd.print("Sound:");
  lcd.setCursor(11, 1);
  lcd.print("ON");
  lcd.setCursor(11, 2);
  lcd.print("OFF");

  while (true) {
    lcd.setCursor(10, 1);
    lcd.print(" "); // Clear previous arrow position
    lcd.setCursor(10, arrowPosition + 1);
    lcd.print(">");

    int joystickValueY = analogRead(yPin);

    if (joystickValueY < minThreshold && arrowPosition > 0) {
      arrowPosition--; // Move arrow up
    } else if (joystickValueY > maxThreshold && arrowPosition < 1) {
      arrowPosition++; // Move arrow down
    }

    // Check if the joystick button is pressed to exit the loop
    if (digitalRead(joystickButtonPin) == LOW && millis() - lastButtonPress > debounceDelay) {
      lastButtonPress = millis();
      playSound = (arrowPosition == 0); // Set sound state based on arrow position
      break;
    }

    delay(100);
  }
}

// Function to control LCD brightness
void lcdBrightnessControl() {
  lcd.clear();
  lcd.print("LCD Brightness:");
  lcd.setCursor(7, 1);
  lcd.print("<   0   >");

  int storedBrightness = EEPROM.read(eepromAddress_lcdBrightness);  // Read stored brightness value
  lcdBrightness = storedBrightness;

  lcd.setCursor(9, 1);
  lcd.print(lcdBrightness);

  while (true) {
    int joystickValueY = analogRead(yPin);

    // Check if the joystick button is pressed to exit the loop
    if (digitalRead(joystickButtonPin) == LOW && millis() - lastButtonPress > debounceDelay) {
      lastButtonPress = millis();
      EEPROM.update(eepromAddress_lcdBrightness, lcdBrightness);
      break;
    }

    // Adjust brightness based on joystick position
    if (joystickValueY < minThreshold) {
      lcdBrightness = max(0, lcdBrightness - 5);
      analogWrite(lcdBacklightPin, lcdBrightness);
    } else if (joystickValueY > maxThreshold) {
      lcdBrightness = min(255, lcdBrightness + 5);
      analogWrite(lcdBacklightPin, lcdBrightness);
    }

    lcd.setCursor(9, 1);
    lcd.print("   ");  // Clear previous brightness value
    lcd.setCursor(9, 1);
    lcd.print(lcdBrightness);

    delay(100);  // Adjust the delay based on your preference and responsiveness
  }
  Serial.print(lcdBrightness);
}

// Function to control the matrix brightness
void matrixBrightnessControl(){
  lcd.clear();
  lcd.print("Matrix Brightness:");
  lcd.setCursor(7, 1);
  lcd.print("<   0   >");

  lcd.setCursor(9, 1);
  lcd.print(matrixBrightness);

  for (int row = 0; row < matrixSize; row++) {
    for (int col = 0; col < matrixSize; col++) {
      lc.setLed(0, row, col, 1);
    }
  }

  while (true) {
    int joystickValueY = analogRead(yPin);

    // Check if the joystick button is pressed to exit the loop
    if (digitalRead(joystickButtonPin) == LOW && millis() - lastButtonPress > debounceDelay) {
      lastButtonPress = millis();
      EEPROM.update(eepromAddress_matrixBrightness, matrixBrightness);
      break;
    }

    // Adjust brightness based on joystick position
    if (joystickValueY < minThreshold && matrixBrightness > 0) {
      matrixBrightness = max(0, matrixBrightness - 1);
      lc.setIntensity(0, matrixBrightness);  // Adjust matrix brightness using LedControl library
    } else if (joystickValueY > maxThreshold && matrixBrightness < 15) {  // 15 is the maximum intensity value for LedControl
      matrixBrightness = min(15, matrixBrightness + 1);
      lc.setIntensity(0, matrixBrightness);  // Adjust matrix brightness using LedControl library
    }
    

    lcd.setCursor(9, 1);
    lcd.print("   ");  // Clear previous brightness value
    lcd.setCursor(9, 1);
    lcd.print(matrixBrightness);

    delay(100);  // Adjust the delay based on your preference and responsiveness
  }
  lc.clearDisplay(0);
}



//------------------------------SNAKE----------------------------------------------------
// Function to read joystick input and update snake
void snakeMove(){
  int xValue = analogRead(xPin);
  int yValue = analogRead(yPin);

  // Store the last positions of the LED
  snakeLastCol = snakeCol;
  snakeLastRow = snakeRow;

  // Update direction based on joystick movement (X-axis)
  if (xValue < minThreshold) { // RIGHT
    if (snakeDirection != 3) { // Make sure that the snake doesn't do an 180 degree turn
      snakeDirection = 1;
    }
  }

  if (xValue > maxThreshold) { // LEFT
    if (snakeDirection != 1) {
      snakeDirection = 3;
    }
  }

  // Update direction based on joystick movement (Y-axis)
  if (yValue > maxThreshold) { // DOWN
    if (snakeDirection != 0) {
      snakeDirection = 2;
    }
  }

  if (yValue < minThreshold) { // UP
    if (snakeDirection != 2) {
      snakeDirection = 0;
    }
  }

  // Update snake position based on the direction
  switch (snakeDirection) {
    case 0: // UP
      if (snakeRow > 0) {
        snakeRow--;
      } else {
        snakeRow = matrixSize - 1; // Wrap around to the bottom if at the top position
      }
      break;

    case 1: // RIGHT
      if (snakeCol < matrixSize - 1) {
        snakeCol++;
      } else {
        snakeCol = 0;
      }
      break;

    case 2: // DOWN
      if (snakeRow < matrixSize - 1) {
        snakeRow++;
      } else {
        snakeRow = 0;
      }
      break;

    case 3: // LEFT
      if (snakeCol > 0) {
        snakeCol--;
      } else {
        snakeCol = matrixSize - 1;
      }
      break;
  }

  // Display the updated snake position
  lc.setLed(0, snakeRow, snakeCol, 1); // Turn on the LED at the new position

  // if there is a snake body segment, this will cause the end of the game (snake must be moving)
	if (snakeBody[snakeRow][snakeCol] > 1 ) {
		lose = true;
    gamePlaying = false;

    // Play the sad song
    for (int i = 0; i < sizeof(sadSongNotes) / sizeof(sadSongNotes[0]); i++) {
      tone(buzzerPin, sadSongNotes[i], sadSongDurations[i]);
      delay(sadSongDurations[i] * 1.2); // Add a small delay between notes
    }

    noTone(buzzerPin); // Turn off the buzzer
		return;
	}

	// check if the food was eaten
	if (snakeRow == foodRow && snakeCol == foodCol) {
    // make a sound when food is eaten
    tone(buzzerPin, buzzerTone, 500);
    delay(100);
    noTone(buzzerPin);

		foodRow = -1; // reset food
		foodCol = -1;
		
		snakeLength++; // increment snake length
    // Increase snake speed at certain snake lengths
    if (snakeLength % 5 == 0) {
      snakeSpeed++;
    }

		// increment all the snake body segments
		for (int row = 0; row < 8; row++) {
			for (int col = 0; col < 8; col++) {
				if (snakeBody[row][col] > 0 ) {
					snakeBody[row][col]++;
				}
			}
		}
	}

	// add new segment at the snake head location
	snakeBody[snakeRow][snakeCol] = snakeLength + 1; // will be decremented in a moment

	// decrement all the snake body segments, if segment is 0, turn the corresponding led off
	for (int row = 0; row < 8; row++) {
		for (int col = 0; col < 8; col++) {
			// if there is a body segment, decrement it's value
			if (snakeBody[row][col] > 0 ) {
				snakeBody[row][col]--;
			}

			// display the current pixel
			lc.setLed(0, row, col, snakeBody[row][col] == 0 ? 0 : 1);
		}
	}

  // Display information on the LCD during gameplay
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Length: ");
  lcd.print(snakeLength);
  lcd.setCursor(0, 1);
  lcd.print("Speed: ");
  lcd.print(snakeSpeed);
  lcd.setCursor(14, 0);
  //lcd.print("Time: ");
  lcd.print(millis() / 1000); // Display time in seconds

  unsigned long startTime = millis();
  while (millis() - startTime < delaySpeed) {
    
  }

}


// Function to generate food in random places
void generateFood(){
  if(foodCol == -1 && foodRow == -1){
    if(snakeLength == 64){ // snake has reached max capacity, which means the user won the game
      win = true;
      gamePlaying = false;

      // Play the short happy song
      for (int i = 0; i < sizeof(happySongNotes) / sizeof(happySongNotes[0]); i++) {
        tone(buzzerPin, happySongNotes[i], happySongDurations[i]);
        delay(happySongDurations[i] * 1.2); // Add a small delay between notes
      }

      noTone(buzzerPin); // Turn off the buzzer
      return;
    }
      
    // generate food until it is in the right position
		do {
			foodCol = random(8);
			foodRow = random(8);
		} while (snakeBody[foodRow][foodCol] > 0);
  }

  // Control the food LED blinking
  if (millis() % (2 * blinkingRate) < blinkingRate) {
    lc.setLed(0, foodRow, foodCol, 1);
  } else {
    lc.setLed(0, foodRow, foodCol, 0);
  }
}


// Function to handle state of the game (win/lose)
void handleGameStates(){
  if(lose == true || win == true){
    // Display relevant game information
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Snake Length: ");
    lcd.print(snakeLength);

    // Read the high score from EEPROM
    int storedHighScore = EEPROM.read(eeprom_HighScore);

    // Check if the player beat the high score
    if (snakeLength > storedHighScore) {
      // Update the high score
      storedHighScore = snakeLength;
      EEPROM.update(eeprom_HighScore, storedHighScore);

      lcd.setCursor(0, 1);
      lcd.print("New High Score!");
    } else {
      lcd.setCursor(0, 1);
      lcd.print("High Score: ");
      lcd.print(storedHighScore);
    }

    // Wait for the player to press the joystick button to close the menu
    while (digitalRead(joystickButtonPin) == HIGH) {
      delay(50); // Adjust the delay based on your preference and responsiveness
    }

    // Wait for the player to release the joystick button
    while (digitalRead(joystickButtonPin) == LOW) {
      delay(50); // Adjust the delay based on your preference and responsiveness
    }

    // reset all the variables
    snakeSpeed = 1; 
		win = false;
		lose = false;
		snakeCol = random(8);
		snakeRow = random(8);
		foodCol = -1;
		foodRow = -1;
		snakeLength = snakeLengthInit;
    lc.setLed(0, foodRow, foodCol, 0);
    //delay(1000);

    for (int row = 0; row < 8; ++row) {
      for (int col = 0; col < 8; ++col) {
        snakeBody[row][col] = 0;
      }
    }

		lc.clearDisplay(0);
  }
}
