// DIY Sat Tracker Software by Tysonpower.de
// NOTE: The code works but it is NOT optimal nor bug free

// Implementation of the Yaesu GS232B Protocol
// B              - Report elevation                                EL=eee
// C              - Report azimuth                                  AZ=aaa
// C2             - Report azimuth and elevation                    AZ=888 EL=eee
// S              - Stop all rotation
// A              - Stop azimuth rotation
// E              - Stop elevation rotation
// L              - Rotate azimuth left (CCW)
// R              - Rotate azimuth right (CW)
// D              - Rotate elevation down
// U              - Rotate elevation up
// Mxxx           - Move to azimuth
// Wxxx yyy       - Move to azimuth xxxx and elevation yyy
// X1             - Change to azimuth rotation speed setting 1
// X2             - Change to azimuth rotation speed setting 2
// X3             - Change to azimuth rotation speed setting 3
// X4             - Change to azimuth rotation speed setting 4
// O              - Azimuth offset calibration
// F              - Azimuth full scale calibration
// O2             - Elevation offset calibration
// F2             - Elevation full scale calibration
// P36            - Switch to 360 degree mode
// P45            - Switch to 450 degree mode
// Z              - Toggle north / south centered mode
// H              - Help

#include "BluetoothSerial.h" 

//----- SETUP -----
//#define setup_EL_steps 76800              // Steps for 360 degrees on the Elevation axis
//#define setup_AZ_steps 76800              // Steps for 360 degrees on the Azimuth axis
#define setup_steps 86000                   // Steps for 360 degrees on both axis

#define setup_EL_endstop 35                  // Endstop Signal EL
#define setup_AZ_endstop 34                  // Endstop Signal AZ

#define setup_EnaStepper 32                  // Enable Steppers (off = Steppers enabled)

// Pins Stepper EL
#define setup_EL_PulPin 33                   // Pulse pin
#define setup_EL_DirPin 25                   // Direction pin
// Pins Stepper AZ
#define setup_AZ_PulPin 26                   // Pulse pin
#define setup_AZ_DirPin 27                   // Direction pin

#define ONBOARD_LED  2

//----- Start of Programm -----
String serialIn;
BluetoothSerial ESP_BT; //Object for Bluetooth

// PUBlic variables
int pub_EL = 0;                     // Elevation angle
int pub_AZ = 0;                     // Azimuth angle
boolean pub_stopEL = false;         // Stop flag for EL
boolean pub_stopAZ = false;         // Stop flag for az
int pub_stepDelay = 300;           // delay between steps in microseconds / Set by speed commands

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(50);

  ESP_BT.begin("SatTracker"); //Name of your Bluetooth Signal
  Serial.println("Bluetooth Device is Ready to Pair");

  pinMode(setup_EnaStepper, OUTPUT);

  pinMode(setup_EL_PulPin, OUTPUT);
  pinMode(setup_EL_DirPin, OUTPUT);

  pinMode(setup_AZ_PulPin, OUTPUT);
  pinMode(setup_AZ_DirPin, OUTPUT);

  pinMode(setup_EL_endstop, INPUT);
  pinMode(setup_AZ_endstop, INPUT);

  pinMode(ONBOARD_LED, OUTPUT);

  disableSteppers();

}

void loop() {

// Check if there i something new on Serial
  if(ESP_BT.available() > 0){

    serialIn = ESP_BT.readString();

    digitalWrite(ONBOARD_LED, HIGH);   // turn the LED on (HIGH is the voltage level)

    // There is, so let's see if it is a valid cmd
    if (serialIn == "B"){                                 // Report elevation (el)
      ESP_BT.print("EL=");
      ESP_BT.println(pub_EL);
    } else if(serialIn == "C"){                           // Report azimuth (az)
      ESP_BT.print("AZ=");
      ESP_BT.println(pub_AZ);
    } else if(serialIn == "C2"){                          // Report az and el
      ESP_BT.print("AZ=");
      ESP_BT.print(pub_AZ);
      ESP_BT.print(" EL=");
      ESP_BT.println(pub_EL);
    } else if(serialIn == "S"){                           // Stop all rotation
      cmd_stopEL();
      cmd_stopAZ();
    } else if(serialIn == "A"){                           // Stop az rotation
      cmd_stopAZ();
    } else if(serialIn == "E"){                           // Stop el rotation
      cmd_stopEL();
    } else if(serialIn == "L"){                           // rotate azimuth left (CCW)

    } else if(serialIn == "R"){                           // rotate azimuth right (CW)

    } else if(serialIn == "D"){                           // rotate elevation down

    } else if(serialIn == "U"){                           // rotate elevation up

    } else if(serialIn.substring(0, 1) == "M"){           // Move to azimuth
      enableSteppers();
      cmd_moveToAZ(serialIn.substring(1, 4));
    } else if(serialIn.substring(0, 1) == "W"){           // Move to azimuth and elevation
      enableSteppers();
      cmd_moveToAZ(serialIn.substring(1, 4));
      cmd_moveToEL(serialIn.substring(5, 8));
    } else if(serialIn == "X1"){                          // Azimuth rotation speed 1
      pub_stepDelay = 2000;
    } else if(serialIn == "X2"){                          // Azimuth rotation speed 2
      pub_stepDelay = 1000;
    } else if(serialIn == "X3"){                          // Azimuth rotation speed 3
      pub_stepDelay = 500;
    } else if(serialIn == "X4"){                          // Azimuth rotation speed 4
      pub_stepDelay = 300;
    } else if(serialIn == "O"){                           // Azimuth offset calibration
      enableSteppers();
      cmd_Offset_cal_AZ();
    } else if(serialIn == "F"){                           // Azimuth full scale calibration

    } else if(serialIn == "O2"){                          // Elevation offset calibration
      enableSteppers();
      cmd_Offset_cal_EL();
    } else if(serialIn == "F2"){                          // Elevation full scale calibration
      enableSteppers();
      // cmd_calibrateEL();
    } else if(serialIn == "P36"){                         // Switch to 360 degree mode

    } else if(serialIn == "P45"){                         // Switch to 450 degree mode

    } else if(serialIn == "Z"){                           // Toggle north/South centered mode

    } else if(serialIn == "H"){                           // Help

    }

    disableSteppers();

    digitalWrite(ONBOARD_LED, LOW);    // turn the LED off by making the voltage LOW
  }
}


// Function to drive a stepper one degree. rev = false = CW rotation
void moveStepperOneDeg(int PULpin, int DIRpin, boolean rev){
  int i;
  //Serial.println("moving Stepper");
  if(rev){
    digitalWrite(DIRpin, LOW);
  } else {
    digitalWrite(DIRpin, HIGH);
  }
  i = 0;
  while(i < setup_steps / 360){
    digitalWrite(PULpin, HIGH);
    delayMicroseconds(100);
    digitalWrite(PULpin, LOW);
    delayMicroseconds(pub_stepDelay);
    i++;
  }

}

void moveStepperOneStep(int PULpin, int DIRpin, boolean rev){
  if(rev){
    digitalWrite(DIRpin, LOW);
  } else {
    digitalWrite(DIRpin, HIGH);
  }


    digitalWrite(PULpin, HIGH);
    delayMicroseconds(100);
    digitalWrite(PULpin, LOW);
    delayMicroseconds(pub_stepDelay);
}

void enableSteppers() {  
  digitalWrite(setup_EnaStepper, LOW);
}

void disableSteppers() {  
  digitalWrite(setup_EnaStepper, HIGH);
}
