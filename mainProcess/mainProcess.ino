//arduino libraries
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include <RTCZero.h>
#include <SPI.h>
#include <RTCZero.h>

//custom libraries
//#include <ValveControl.h>

/*
 * Definitions for Hardware Pins
 */
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
const int selectButton = #;
const int cancelButton = #;
//const int emergencyButton = #;

/* 
 *  Common Global Variables and Initalizations
 */
//Real-time
RTCZero rtc;
const byte seconds = 0;
const byte minutes = 16;
const byte hours = 14;
const byte day = 27;
const byte month = 2;
const byte year = 20;
//SD
const int chipSelect = 28;
String fileName = "failed";
//File logFile;
//LCD
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x3F, 20, 4);

//states for the main loop
const int initial = 0;
const int selectParameters = 1;
const int calculations = 2;
const int startProcess = 3;
const int processRunning = 4;
const int stopProcess = 5;
const int cleanUpProcess = 6;
const int hardwareTesting = 7;

int state = 0;
//float runtime = 0;

void setup() {
  //**Below establish serial communication for debugging**//
  Serial.begin(9600);
  while(!Serial){
    //gotta wait on the serial port connection
  }
  
  //**Setup SD**//
  Serial.print("Initializing SD card...");
  if(!SD.begin(chipSelect)){
    Serial.println("Card failed, or not present at all");
    //don't do anything more
    while(true);
  }
  Serial.println("Card initialized. ");

  //**Set up real-time clock**//
  rtc.begin();
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

  //**Initiate the LCD**//
  lcd.init();
  lcd.backlight();

  //**Set up LEDs**//
  //pinMode(alertLED, OUTPUT);
  //pinMode(runningLED, OUTPUT);

  //**Set up buttons**//
  //pinMode(selectButton, INPUT);

  //**Set up speaker**//
  //pinMode(speaker, OUTPUT);
  
}//end setup function


//main loop
void loop() {
  switch(state){
    case initial:
    {
      //initialize LEDs
      //digitalWrite(alertLED, LOW);
      //digitalWrite(runningLED, LOW);

      //Create unique filename by using date and time
      String minutesDec = String(minutes, DEC);
      String hoursDec = String(hours, DEC);
      String dayDec = String(day, DEC);
      String monthDec = String(month, DEC);
      fileName = String(monthDec + dayDec + hoursDec + minutesDec);
      Serial.println("File name is...");
      Serial.println(fileName);

      state = selectParameters;
      break;
    }
    case selectParameters:
    {
      //call UI function to get user entered parameters
      
      //open file and write user selected values to it.
      //fileName = "sweet nothings";
      //File logFile = SD.open(fileName, FILE_WRITE);
      //String dataString = "Test";
      /*
      if(logFile){
        logFile.println(dataString);
        logFile.close();
        Serial.println(dataString);
      }else{
        Serial.println("error with the file");
      }
      */

      //calculate the runtime
      //runtime=

      //log the runtime
     
      //state = startProcess;
      break;
    }
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
      state = initial;
      break;
    case hardwareTesting:
      //this case is to help trouble-shoot hardware
      break;
    default:
      //elegantly catch errors 
      break; 
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
        //display initial LCD message
        //setCursor(col,row) Both are zero indexed!
        lcd.setCursor(0,0);
        lcd.print("Select Parameters");
        lcd.setCursor(0,3);
        lcd.print("Select        Cancel");
        
        while(true){
          //check the value of select button
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

/*
void emergencyShutdown(){
  //send command to turn on emergencyLED  
  digitalWrite(alertLED, HIGH);
  
  //send command to turn off pumps
  digitalWrite(P1CTL, HIGH);
  digitalWrite(P2CTL, HIGH);
  analogWrite(P1Speed, 0);
  analogWrite(P2Speed, 0);
  
  //send command to turn valves to closed
  
  //TODO
  
  //Write current time and emergency message to SD
  // TODO: check scope on logFile
  File logFile = SD.open(fileName, FILE_WRITE);
  if(logFile){
      
    logFile.println("Emergency shutdown procedure initiated.");
        
    // Print date...
    //print2digits isn't a library function, but I guess it adds a zero for single digits
    //I started to write something similar, padDigits function above
      
    logFile.padDigits(rtc.getDay()); 
    logFile.print("/");
    logFile.padDigits(rtc.getMonth());
    logFile.print("/");
    logFile.padDigits(rtc.getYear());
    logFile.print(" ");

    // Print time
    logfile.padDigits(rtc.getHours());
    logFile.print(":");
    logFile.padDigits(rtc.getMinutes());
    logFile.print(":");
    logFile.padDigits(rtc.getSeconds());

    logFile.println();
    logFile.close();
    Serial.println("Date and time recorded successfully");
    
  }
  else{
       Serial.println("error with the file, emergency shutdown initiated.");
  }
  
  /*
  while(1){
    //sound alert through speaker
    digitalWrite(speaker, HIGH);
    delay(100);
    digitalWrite(speaker,LOW);
    delay(1000);
  }
  */
  
//}//end emergencyShutdown function
