// go for pro micro, nano doesnt have enough I/Os for expansion

//Notes: - maybe should do the truns on half speed just in case.

//TODO:  - check out https://github.com/mateusjunges/accel-stepper-with-distances
//       - check out https://forum.arduino.cc/index.php?topic=515370.0
//       - check out https://github.com/thomasfredericks/Bounce2



#include <AccelStepper.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Fonts/FreeSans18pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Bounce2.h>

#define OLED_RESET 0  // GPIO0
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// Define step constant
#define FULLSTEP 8

// See your proper pins for the screen, D2&3 for por micro in this case
// Using old project as reference (feed-o-pony) let the copy/paste commence!!
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pro micro has funky numbers, no not mind. Also look closely needed pin sequence:
// :D IN1-IN3-IN2-IN4
AccelStepper stepper1(FULLSTEP, 15, 16, 14, 10);
AccelStepper stepper2(FULLSTEP, 21, 19, 20, 18);

/*
  enum menu {
  start,
  program
  };

  enum startMenuSelection {
  startProgram,
  editProgram,
  deleteProgram
  };
*/
// For better animations menuarray is better
// TODO make better
byte mainMenu = 0; // start, edit, delete
byte editMenuSelection = 0; // start, edit, delete
bool editingProgram = false; // true if ... editing
char upArrow = 24;
char downArrow = 25;
char rightArrow = 26;
char leftArrow = 27;


// Setup
const int stepsNeededForRevolution = 4096;  // 28BYJ-48 used, for other change the value
int defaultMaxSpeed = 200; // read the values over 1000 would be unreliable?
int defautlAcceleration = 50.0; // max acceleration (steps per second^2)
int buttonDelay = 100;

// fags
int currenctCommand = 0;
int nextCommandNeeded = 1;
int buttonPressed = 0;
int stopped = 1;

// toggleIndexes
int currentMainMenuSelection = 0;
int currentEditMenuSelection = 0;

int indexToEdit = 0;
int lastEditedIndex = 0;;

//
static byte clearCommands[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                               0, 0, 0, 0, 0, 0, 0, 0, 0, 0
                              };
byte commands[100];

// Pin definitnos
const int upButtonPin = 7;
const int downButtonPin = 8;
const int selectButtonPin = 9;
const int backButtonPin = 4;
/*
    Pinmap:
    2-3 screen
    4-8 buttons
    9 free, possibly for servo
    10-21 steppers ( note: pins 11,12,13 and 17 do not excist)
*/

//TODO: Check the bounce library one day gain
int upButton;
int downButton;
int selectButton;
int backButton;


void setup() {
  Serial.begin(9600);

  pinMode(upButtonPin, INPUT);
  pinMode(downButtonPin, INPUT);
  pinMode(selectButtonPin, INPUT);
  pinMode(backButtonPin, INPUT);


  //Stepper setups
  stepper1.setMaxSpeed(defaultMaxSpeed);
  stepper1.setAcceleration(defautlAcceleration);
  //stepper1.setSpeed(500); // steps per second
  stepper2.setMaxSpeed(defaultMaxSpeed);
  stepper2.setAcceleration(defautlAcceleration);
  //stepper2.setSpeed(500); // steps per second

  //Screen setup
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.clearDisplay();
  display.display();
  display.clearDisplay();
  display.setFont(&FreeSans9pt7b);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  delay(1000);
  Serial.println("setup complete");

  stepper1.setCurrentPosition(0);
  stepper2.setCurrentPosition(0);
  delay(10);
}


int readButtoni(int pin) {
  pin = digitalRead(pin);
  if (pin == 1) {
    delay(50);
    pin = digitalRead(pin);
    if (pin == 0) {
      return HIGH;
    }
  }
}


void loop() {
  buttonPressed = 0;

  // JUST FOR TESTING REMOVE WHEN USING STEPPERS
  delay(100);

  /*
    First of all try to check if program excecution is going on and try not to disturb it as
    doing anything else while operating stepper motors will distract or delay their operation.

    So trying to observe only the stop button, others only if not running
  */

  // 1. Make sure program is not running
  if (!stepper1.isRunning() && !stepper2.isRunning()) {

    upButton = readButtoni(upButtonPin);
    downButton = readButtoni(downButtonPin);
    selectButton = readButtoni(selectButtonPin);
    backButton = readButtoni(backButtonPin);


    //    Serial.print("Buttonreads udsb: ");
    //    Serial.print(upButton);
    //    Serial.print(downButton);
    //    Serial.print(selectButton);
    //    Serial.println(backButton);




    //TODO delete confirmation -------------

    // 2. Check if we are in programming menu or main menu
    if (editingProgram) {
      Serial.println("Is edit mode");

      indexToEdit = lastEditedIndex;
      // select area could be good and also selected queue, maybe left to right
      printEditMenuSelection(editMenuSelection);

      if (selectButton == HIGH && buttonPressed == 0 ) {// Edit menu, select selected, button pressed
        Serial.println("selectButton pressed (edit)");
        delay(buttonDelay);

        buttonPressed = 1;
        
      } else if (upButton == HIGH && buttonPressed == 0) { // Edit menu, up selected, button pressed
        Serial.println("upButton pressed (edit)");
        toggleEditSelection(editMenuSelection, 0);
        delay(buttonDelay);


        buttonPressed = 1;
        
      } else if (downButton == HIGH && buttonPressed == 0  ) { // Edit menu, down selected, button pressed
        Serial.println("downButton pressed (edit)");
        toggleEditSelection(editMenuSelection, 1);
        delay(buttonDelay);

        buttonPressed = 1;
        
      } else if (backButton == HIGH && buttonPressed == 0  ) { // Edit menu, back selected, button pressed
        Serial.println("backButton pressed (edit)");
        delay(buttonDelay);
        buttonPressed = 1;
        editingProgram = false;
      }

    } else {
      printMainMenu(mainMenu);
      Serial.println("Is mainmanu mode");


      if (selectButton == HIGH && buttonPressed == 0 && mainMenu == 0 ) {// Main menu, START selected, button pressed
        Serial.println("selectButton pressed (main)");
        printScreenMessage("Motor Start");
        delay(buttonDelay);
        stopped = 1;
        nextCommandNeeded = 1;
        buttonPressed = 1;

      } else if (selectButton == HIGH && buttonPressed == 0 && mainMenu == 1 ) { // Main menu, EDIT selected, button pressed
        Serial.println("selectButton pressed (main)");
        printScreenMessage("Edit program");
        delay(buttonDelay);
        editingProgram = true;
        stopped = 1;
        nextCommandNeeded = 0;
        buttonPressed = 1;

      } else if (selectButton == HIGH && buttonPressed == 0 && mainMenu == 2 ) { // Main menu, DELETE selected, button pressed
        Serial.println("selectButton pressed (main)");
        printScreenMessage("delete program");
        delay(buttonDelay);
        memcpy(commands, clearCommands, sizeof(clearCommands));
        stopped = 1;
        nextCommandNeeded = 0;
        buttonPressed = 1;

      } else if (selectButton == HIGH && buttonPressed == 0 ) { // Main menu, up or down button pressed
        Serial.println("selectButton pressed (main)");
        toggleMainMenu(mainMenu, 0);
        delay(buttonDelay);
        stopped = 1;
        nextCommandNeeded = 0;
        buttonPressed = 1;


      } else if (upButton == HIGH && buttonPressed == 0 ) { // Main menu, up  button pressed
        Serial.println("upButton pressed (main)");
        toggleMainMenu(mainMenu, 0);
        delay(buttonDelay);
        stopped = 1;
        nextCommandNeeded = 0;
        buttonPressed = 1;

      } else if (downButton == HIGH && buttonPressed == 0 ) { // Main menu, down button pressed
        Serial.println("downButton pressed (main)");
        toggleMainMenu(mainMenu, 1);
        delay(buttonDelay);
        stopped = 1;
        nextCommandNeeded = 0;
        buttonPressed = 1;
      }



      if (nextCommandNeeded && !stopped) {
        executeCommand(currenctCommand);
        newCommandPrint(currenctCommand);
        delay(1000);
        //showSteps(stepsNeededForRevolution);
        delay(20);
        nextCommandNeeded = 0;
      }
    }



    // run the steppers untill they reach their destination
    if (stepper1.distanceToGo() == 0 && stepper2.distanceToGo() == 0 && !stopped) {
      Serial.println("destination reached");
      stepper1.stop();
      stepper2.stop();
      currenctCommand++;
      nextCommandNeeded = 1;
    } else {
      stepper1.run();
      stepper2.run();
    }


    // temporary for debugging:
    if (currenctCommand > 5) {
      currenctCommand = 0;
    }


  }
}


//TODO MAKE BETTER
// 1,2,3,4 -- ylös,alas,vasen,oikea --- 0,1 ---- up, down
void toggleEditSelection(int currentEditMenuSelection, int directionUpDown) {
  Serial.print("toggleEditSelection direction: ");
  Serial.println(directionUpDown);
  if (directionUpDown == 0) { // UP
    if (currentMainMenuSelection < 2) {
      currentMainMenuSelection++;
    } else {
      currentMainMenuSelection = 0;
    }
  } else if ( directionUpDown == 1) { // DOWN
    if (currentMainMenuSelection > 0) {
      currentMainMenuSelection++;
    } else {
      currentMainMenuSelection = 2;
    }
  } else {
    Serial.print("toggleMainMenu Error");
    printScreenMessage("Error");
  }
  editMenuSelection = currentMainMenuSelection;
}

//TODO MAKE BETTER
// 0,1,2 -- start,edit,delete --- 0,1 ---- up, down
void toggleMainMenu(int currentMainMenuSelection, int directionUpDown) {
  Serial.print("toggleMainMenu direction: ");
  Serial.println(directionUpDown);
  if (directionUpDown == 0) { // UP
    if (currentMainMenuSelection < 2) {
      currentMainMenuSelection++;
    } else {
      currentMainMenuSelection = 0;
    }
  } else if ( directionUpDown == 1) { // DOWN
    if (currentMainMenuSelection > 0) {
      currentMainMenuSelection++;
    } else {
      currentMainMenuSelection = 2;
    }
  } else {
    Serial.print("toggleMainMenu Error");
    printScreenMessage("Error");
  }
  mainMenu = currentMainMenuSelection;
}

// TODO: make better structure
void executeCommand(int commandIndex) {
  Serial.print("executeCommand: ");
  Serial.println(commandIndex);

  switch (commandIndex) {
    case 0:
      driveForwards();
      break;
    case 1:
      driveBackwards();
      break;
    case 2:
      turnLeft();
      break;
    case 3:
      turnRight();
      break;
    case 4:
      tankLeft();
      break;
    case 5:
      tankRight();
      break;
    default:
      Serial.println("Bad commandIndex");
      break;
  }
}

void driveForwards() {
  stepper1.moveTo(stepper1.currentPosition() + stepsNeededForRevolution);
  stepper2.moveTo(stepper2.currentPosition() + stepsNeededForRevolution);
}

void driveBackwards() {
  stepper1.moveTo(stepper1.currentPosition() - stepsNeededForRevolution);
  stepper2.moveTo(stepper2.currentPosition() - stepsNeededForRevolution);
}

void turnLeft() {
  stepper1.moveTo(stepper1.currentPosition() + stepsNeededForRevolution);
}

void turnRight() {
  stepper2.moveTo(stepper2.currentPosition() + stepsNeededForRevolution);
}

void tankLeft() {
  stepper1.moveTo(stepper1.currentPosition() + stepsNeededForRevolution);
  stepper2.moveTo(stepper2.currentPosition() - stepsNeededForRevolution);
}

void tankRight() {
  stepper1.moveTo(stepper1.currentPosition() - stepsNeededForRevolution);
  stepper2.moveTo(stepper2.currentPosition() + stepsNeededForRevolution);
}


void printMainMenu(int menu) {
  String menuPring = "error";
  display.clearDisplay();
  display.setCursor(16, 22);     // x,y
  switch (menu) {
    case 0:
      menuPring = "-START-";
      break;
    case 1:
      menuPring = "-EDIT-";
      break;
    case 2:
      menuPring = "-DELETE-";
      break;
    default:
      Serial.println("Bad printMainMenu");
      break;
  }
  display.print(menuPring);
  display.display();
  delay(10);
}

void printEditMenuSelection(int editMenu) {
  char editMenuPring = 'e';
  display.clearDisplay();
  display.setCursor(16, 22);     // x,y
  switch (editMenu) {
    case 1:
      editMenuPring = upArrow;
      break;
    case 2:
      editMenuPring = downArrow;
      break;
    case 3:
      editMenuPring = leftArrow;
      break;
    case 4:
      editMenuPring = rightArrow;
      break;
    default:
      Serial.println("Bad printEditMenuSelection");
      break;
      display.print(editMenuPring);
      display.display();
      delay(10);
  }
}


void printArrow(char arrowChar) {
  display.clearDisplay();
  display.setCursor(16, 22);     // x,y
  display.print(arrowChar);
  display.display();
  delay(10);
}

void printScreenMessage(String message) {
  display.clearDisplay();
  display.setCursor(16, 22);     // x,y
  display.print(message);
  display.display();
  delay(10);
}

void newCommandPrint(int commandIndex) {
  Serial.println("driving");
  display.clearDisplay();
  display.setCursor(16, 22);     // x,y
  display.print("command: ");
  display.print(commandIndex);
  display.display();
  delay(10);
}

void showSteps(int stepCount) {
  Serial.println("showSte");
  display.clearDisplay();
  display.setCursor(16, 22);     // x,y
  display.print(stepCount);
  display.print(" steps");
  display.display();
  delay(10);
}
