//arduino libraries
#include <LiquidCrystal.h>
#include <SD.h>
#include <RTCZero.h>
#include <SPI.h>
#include <RTCZero.h>

//custom libraries
//#include <ValvePositions.h>

//Defines for hardware pins
//LCD
//Motors <-variables should go in custom library
//Pumps
//Speaker
//int speaker = #;
//SD
//Potentiometers
//int coarsePot = #;
//int finePot = #;
//LEDS
//int runningLED = #;
//int alertLED = #
//Buttons
//int selectButton = #;
//int cancelButton = #;
//int emergencyButton = #;

//common global variables
//Real-time clock variables
RTCZero rtc;
const byte seconds = 0;
const byte minutes = 16;
const byte hours = 14;
const byte day = 27;
const byte month = 2;
const byte year = 20;
//SD
const int chipSelect=28;

void setup() {
  //Below establish serial communication for debugging 
  Serial.begin(9600);
  while(!Serial){
    //gotta wait on the serial port connection
  }
  
  //setup SD
  Serial.print("Initializing SD card...");
  if(!SD.begin(chipSelect)){
    Serial.println("Card failed, or not present at all");
    //don't do anything more
    while(true);
  }
  Serial.println("Card initialized. ");

  //set up real-time clock
  rtc.begin();
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

  //set up LEDs
  //pinMode(alertLED, OUTPUT);
  //pinMode(runningLED, OUTPUT);
}

//main loop
void loop() {
  //initialize LEDs
  //digitalWrite(alertLED, LOW);
  //digitalWrite(runningLED, LOW);
  
  //Create unique filename by using date and time
  String minutesDec = String(minutes, DEC);
  String hoursDec = String(hours, DEC);
  String dayDec = String(day, DEC);
  String monthDec = String(month, DEC);
  String fileName = String(monthDec + dayDec + hoursDec + minutesDec);
  Serial.println("File name is...");
  Serial.println(fileName);
  
  //call ui function

  //open file and write user selected values to it.
  File logFile = SD.open(fileName, FILE_WRITE);
  String dataString = "Test";
  if(logFile){
    logFile.println(dataString);
    logFile.close();
    Serial.println(dataString);
  }else{
    Serial.println("error with the file");
  }

  //calculate the runtime from the entered variables. 

  //Set valves to their flow reagent position
  
  //Call UI function to ask user to prime pumps
  
  //command pump to turn on

  //Log the time the process started

  //run for alloted time

  //log the end time

  //Call UI to ask user if done.


  
  delay(5000);
  
}//end main loop function

//Pad a zero in front of digits
void padDigits(int number){
  if(number <10){
    
  }
}

//handle everything the user interacts with (LCD, BUTTONS, POTS)
void doUserInterface(){
  const int MenuLanding = 0;
  const int SelectFlowOne = 1;
  const int SelectVolumeOne = 2;
  const int ConfirmPumpOne = 3;
  const int SelectFlowTwo = 4;
  const int SelectVolumeTwo = 5;
  const int ConfirmPumpTwo = 6;

  int CurrentMenuPage =0;

  while(true){
    switch(CurrentMenuPage){
      case MenuLanding :
        //blah blah
        while(true){
          //send the LCD message <-might do this outside switch statement
          //""
          //check the values of both buttons
          //if(button1 || button2){
            //a button had been pressed
            //figure out which button it is and adjust CurrentMenuPage accordingly
            //also need to save current value being set.
          //}
          //
        }
        break;
      case SelectFlowOne :
        //blah blah
        break;
      case SelectVolumeOne :
        //blah blah
        break;
      default :
        break;
    }
  }
}//end doUserInterface function
