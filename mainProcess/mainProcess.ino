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
//const int speaker = #;
//SD
//Potentiometers
//const int coarsePot = #;
//const int finePot = #;
//LEDS
//const int runningLED = #;
//const int alertLED = #
//Buttons
//const int selectButton = #;
//const int cancelButton = #;
//const int emergencyButton = #;

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

//states for the main loop
const int init = 0;
const int selectParameters = 1;
const int calculations = 2;
const int startProcess = 3;
const int processRunning = 4;
const int stopProcess = 5;
const int cleanUpProcess = 6;

int state = 0;

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

  //set up buttons
  //pinMode(selectButton, INPUT);

  //set up speaker
  //pinMode(speaker, OUTPUT);
}//end setup function


//main loop
void loop() {
  switch(state){
    case init:
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

      state = selectParameters;
      break;
      
    case selectParameters:
      //call UI function to get user entered parameters
      
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

      //calculate the runtime

     //log the runtime
     
      state = startProcess;
      break;
    case startProcess:

      //set valves to flow reagent position

      //call UI function to ask user to prime pumps

      //command pump to turn on/start pump

       //Log the time the process started
      
      state = processRunning;
      break;
    case processRunning:
      //run for alloted time
      state = stopProcess;
      break;
    case stopProcess:
      //command pump to turn off/stop pump
  
      //log the end time

      //Call UI to ask user if done.
      
      state = cleanUpProcess;
      break;
    case cleanUpProcess:
      //Set valves to flow inert gas to clear the lines

      //Call UI to ask user if done clearing the lines.

      //Set valves to closed.
      state = init;
      break;
    default:
      //elegantly catch errors  
  }
  
  //delay for testing purposes
  delay(5000);
  
}//end main loop function


//Pad a zero in front of digits
void padDigits(int number){
  if(number <10){
    
  }
}//end padDigits function


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


void emergencyShutdown(){
  //send command to turn on emergencyLED
  //send command to turn off pumps
  //send command to turn valves to closed
  //Write current time and emergency message to SD
  //-scope issue here, as the name of file is not known to this funciton
  /*
  while(1){
    //sound alert through speaker
    digitalWrite(speaker, HIGH);
    delay(100);
    digitalWrite(speaker,LOW);
    delay(1000);
  }
  */
  
}//end emergencyShutdown function
