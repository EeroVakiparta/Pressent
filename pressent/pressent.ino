// go for pro micro, nano doesnt have enough I/Os for expansion

//Notes: - maybe should do the truns on half speed just in case.
//       - do I need to introduce "slow acceleration" ; maybe not cause of high torgue

//TODO:  - check out https://github.com/mateusjunges/accel-stepper-with-distances

const int stepsNeededForRevolution = 4096;  // 28BYJ-48 used, for other change the value

#include <AccelStepper.h>
// #include <Stepper.h>

// Define step constant
#define FULLSTEP 8


// Pro micro has funky numbers, no not mind. also look closely needed pin sequence:
// :D IN1-IN3-IN2-IN4
AccelStepper stepper1(FULLSTEP, 15, 16, 14, 10);

void setup() {
  stepper1.setMaxSpeed(1000);
  stepper1.setAcceleration(500.0); // max acceleration (steps per second^2)
 // stepper1.setSpeed(50); // steps per second
  
}

void loop() {
  // warmup, drive revolution forward and chill
  stepper1.setCurrentPosition(0);
  stepper1.moveTo(stepsNeededForRevolution);
  stepper1.runToPosition();
  delay(2000);

  
}
