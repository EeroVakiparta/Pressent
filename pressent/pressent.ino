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

#define OLED_RESET 0  // GPIO0
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

// Define step constant
#define FULLSTEP 8

// See your proper pins for the screen, D2&3 for por micro in this case
// Using old project as reference (feed-o-pony) let the copy/paste commence!!
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pro micro has funky numbers, no not mind. also look closely needed pin sequence:
// :D IN1-IN3-IN2-IN4
AccelStepper stepper1(FULLSTEP, 15, 16, 14, 10);
AccelStepper stepper2(FULLSTEP, 21, 19, 20, 18);

// Setup
const int stepsNeededForRevolution = 4096;  // 28BYJ-48 used, for other change the value
int defaultMaxSpeed = 1000; // read the values over 1000 would be unreliable?
int defautlAcceleration = 500.0; // max acceleration (steps per second^2)

// fags
int running = 0;
int currenctCommand = 0;
int nextCommandNeeded = 1;

void setup() {
  Serial.begin(9600);

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
}


void loop() {
  if (nextCommandNeeded && !running) {
    executeCommand(currenctCommand);
    newCommandPrint(currenctCommand);
    delay(1000);
    showSteps(stepsNeededForRevolution);
    delay(20);
  }


  if (stepper1.distanceToGo() == 0 && stepper2.distanceToGo() == 0) {
    //stepper1.moveTo(-stepper1.currentPosition());
    //stepper2.moveTo(-stepper1.currentPosition());
    currenctCommand++;
    running = 0;
  } else {
    stepper1.run();
    stepper2.run();
    running = 1;
  }

  if (currenctCommand > 5) {
    currenctCommand = 0;
  }
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
