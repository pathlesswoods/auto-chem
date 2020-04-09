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
const int coarsePot = A1;
const int finePot = A2;
//LEDS
//const int runningLED = #;
//const int alertLED = #
//Buttons
const int selectButton = 7;
const int cancelButton = 6;
//const int emergencyButton = #;

/* 
 *  Common Global Variables and Initalizations
 */
//Real-time
RTCZero rtc;
const byte seconds = 0;
const byte minutes = 24;
const byte hours = 14;
const byte day = 9;
const byte month = 4;
const byte year = 20;
//SD
const int chipSelect = 28;
String fileName = "failed";
File logfile;
//LCD
LiquidCrystal_I2C lcd = LiquidCrystal_I2C(0x3F, 20, 4);

// ** states for the main loop ** //
int state = 7;
const int initial = 0;
const int selectParameters = 1;
const int calculations = 2;
const int startProcess = 3;
const int processRunning = 4;
const int stopProcess = 5;
const int cleanUpProcess = 6;
const int hardwareTesting = 7;

// ** states for UI ** //
const int MenuLanding = 0;
const int SelectFlowOne = 1;
const int SelectVolumeOne = 2;
const int ConfirmPumpOne = 3;
const int SelectFlowTwo = 4;
const int SelectVolumeTwo = 5;
const int ConfirmPumpTwo = 6;
const int ExitMenu = 7;
const int PrimePumps = 8;
const int ProcessRunning = 9;
const int ConfirmEndProcess = 10;
const int ConfirmClearedLines = 11;

//Runtime Variables
int flowOne;
int flowTwo;
int volumeOne;
int volumeTwo;

float runtime = 0;

void setup() {
  //**Establish serial communication for debugging**//
  Serial.begin(9600);
  while(!Serial){
    //gotta wait on the serial port connection
  }
  
  //**Setup SD**//
  Serial.print("Initializing SD card...\n");
  if(!SD.begin(chipSelect)){
    Serial.println("Card failed, or not present at all\n");
    //don't do anything more
    while(true);
  }
  Serial.println("Card initialized. \n");

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
  pinMode(selectButton, INPUT);
  pinMode(cancelButton, INPUT);
  //pinMode(emergencyButton, INPUT);
  //attachInterrupt(digitalPinToInterrupt(emergencyButton), emergencyShutdown,HIGH);

  //**Set up potentiometers **//
  pinMode(coarsePot, INPUT);
  pinMode(finePot, INPUT);

  //**Set up speaker**//
  //pinMode(speaker, OUTPUT);
  
}//end setup function


//main loop
void loop() {
  //Allow 12 bit resolution on analog pins
  analogReadResolution(12);
  
  switch(state){

    //** Initialize everything needed **//
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

    //** Select Runtime Parameters **//
    case selectParameters:
    {
      //call UI function to get user entered parameters
      doUserInterface(MenuLanding);

      //calculate the runtime
      //millileters per minute for flowrate of pumps
      float tempRunTimeOne = volumeOne/flowOne;
      float tempRunTimeTwo = volumeTwo/flowTwo;
      if(tempRunTimeOne>tempRunTimeTwo){
        runtime = tempRunTimeOne;
      }else{
        runtime = tempRunTimeTwo;
      }
      
      //open file and write user selected values and runtime to it.
      logfile = SD.open(fileName, FILE_WRITE);
      if(logfile){
        logfile.println("User selected the following parameters: \n");
        logfile.println("Pump One Flow: \n");
        logfile.println(flowOne);
        logfile.println("\nPump One Volume: \n");
        logfile.println(volumeOne);
        logfile.println("\nPump Two Flow: \n");
        logfile.println(flowTwo);
        logfile.println("\nPump Two Volume: \n");
        logfile.println(volumeTwo);
        logfile.println("Calculated runtime is: \n");
        logfile.println(runtime);
        logfile.close();
      }else{
        Serial.println("Error with the file");
        //queue emergency shutdown?
      }

      state = startProcess;
      break;
    }

    //** Process Startup **//
    case startProcess:

      //set valves to flow reagent position

      //call UI function to ask user to prime pumps
      doUserInterface(PrimePumps);

      //command pump to turn on/start pump
      

      //Log the time the process started
      logfile = SD.open(fileName, FILE_WRITE);
      if(logfile){
        logfile.println("Process started at: ");
        logfile.println();
        String minutesDec = String(minutes, DEC);
        String hoursDec = String(hours, DEC);
        String dayDec = String(day, DEC);
        String monthDec = String(month, DEC);
        String currentDateTime = String(monthDec + "/" + dayDec + " " + hoursDec + ":" + minutesDec);
        logfile.println(currentDateTime);
        logfile.close();
      }else{
        Serial.println("Error with the file");
        //queue emergency shutdown?
      }
      
      state = processRunning;
      break;

    //** Process Running **//
    case processRunning:
      //run for alloted time
      //periodically check pumps feedback for errors
      //check flag for errors during this time
      state = stopProcess;
      break;
      
    case stopProcess:
    {
      //command pump to turn off/stop pump
 
      //Log the time the process ended
      logfile = SD.open(fileName, FILE_WRITE);
      if(logfile){
        logfile.println("Process ended at: ");
        logfile.println();
        String minutesDec = String(minutes, DEC);
        String hoursDec = String(hours, DEC);
        String dayDec = String(day, DEC);
        String monthDec = String(month, DEC);
        String currentDateTime = String(monthDec + "/" + dayDec + " " + hoursDec + ":" + minutesDec);
        logfile.println(currentDateTime);
        logfile.close();
      }else{
        Serial.println("Error with the file");
        //queue emergency shutdown?
      }

      //Call UI to ask user if done.
      doUserInterface(ConfirmEndProcess);
    
      state = cleanUpProcess;
      break;
    }
    case cleanUpProcess:
      //Set valves to flow inert gas to clear the lines

      //Call UI to ask user if done clearing the lines.
      doUserInterface(ConfirmClearedLines);

      //Set valves to closed.

      
      state = initial;
      break;

    //** Test and Troubleshoot Hardware **//
    case hardwareTesting:
    {
      //Serial.print("Accessing Menu...");
      //doUserInterface(MenuLanding);
      
      Serial.print("Testing coarse pot...\n");
      int tempCoarsePot = map(analogRead(coarsePot), 0, 4095, 0, 400);
      //int tempFinePot = map(analogRead(finePot), 0, 4095, 0, 9); 
      //int value = (tempCoarsePot*10) + tempFinePot;
      //catch overflow from adding finePot if coarsePot is maxed out
      Serial.print("\ncoarsePot=\n");
      Serial.print(analogRead(coarsePot));
      //Serial.print("\nfinePot=\n");
      //Serial.print(analogRead(finePot));
      //lcd.clear();
      //lcd.setCursor(6,2);
      //lcd.print(value);
    }
      break;
      
    //** Elegantly catch errors **//
    default:
      break; 
  }
  
  //delay for testing purposes
  delay(1000);
  
}//end main loop function


//Pad a zero in front of digits
void padDigits(int number){
  if(number <10){
    
  }
}//end padDigits function

//** doUserInterface Function **//
//Handle everything the user interacts with (LCD, BUTTONS, POTS)
//Inputs: an integer representing the UI state to initially enter
//Outputs: none
void doUserInterface(int UIState){
  //holds current state of buttons
  int selectButtonState = LOW;
  int cancelButtonState = LOW;
  int lastSelectButtonState = LOW;
  int lastCancelButtonState = LOW;
  int readingSelectButton;
  int readingCancelButton;

  //debounce variables
  unsigned long lastSelectButtonPress = 0;
  unsigned long lastCancelButtonPress = 0;
  unsigned long debounceTime = 50;
  
  while(true){
    
    switch(UIState){
      
      // ** Initial LCD Screen ** //
      case MenuLanding :
        //display initial LCD message
        lcd.setCursor(1,1);
        lcd.print("Choose Parameters?");
        lcd.setCursor(0,3);
        lcd.print("Select");

        //if select, then go to next menu page, otherwise do nothing
        if(captureButtons()){
          UIState = SelectFlowOne;
        }
        
        lcd.clear();
        break;

      //** Menu Page to Select Flow for Pump One **//
      case SelectFlowOne :
        //display initial LCD message
        lcd.setCursor(3,0);
        lcd.print("Select Flow for");
        lcd.setCursor(6,1);
        lcd.print("Pump One");
        lcd.setCursor(0,3);
        lcd.print("Select        Cancel");

        //call captureInputs function
        if(captureInput(&flowOne)){
          UIState = SelectVolumeOne;
        }else{
          UIState = MenuLanding;
        }
        
        lcd.clear();
        break;

      //** Menu Page to Select Volume for Pump One **//
      case SelectVolumeOne :
        lcd.setCursor(0,0);
        lcd.print("Select Volume for");
        lcd.setCursor(6,1);
        lcd.print("Pump One");
        lcd.setCursor(6,2);
        lcd.print("0000");
        lcd.setCursor(0,3);
        lcd.print("Select        Cancel");
        
        //call captureInputs function
        if(captureInput(&volumeOne)){
          //if Select pushed, on to next menu page
          UIState = ConfirmPumpOne;
        }else{
          //else cancel pushed, back a state
          UIState = SelectVolumeOne;
        }
        
        lcd.clear();
        break;

      //** Menu Page to Confirm Chosen Volume and Flow for Pump One **//
      case ConfirmPumpOne :
        lcd.setCursor(0,0);
        lcd.print("Confirm Pump1 Values");
        lcd.setCursor(4,1);
        lcd.print("Flow");
        //display flow
        lcd.setCursor(9,1);
        lcd.print(flowOne);
        lcd.setCursor(2,2);
        lcd.print("Volume");
        //display volume
        lcd.setCursor(10,2);
        lcd.print(volumeOne);
        lcd.setCursor(0,3);
        lcd.print("Select        Cancel");

        //if select, then carry on with menu, else go back
        if(captureButtons()){
          UIState = SelectFlowTwo;
        }else{
          UIState = SelectFlowOne;
        }

        lcd.clear();
        break;
        
      //** Menu Page to Select Flow for Pump Two *//
      case SelectFlowTwo :
        lcd.setCursor(0,0);
        lcd.print("Select Flow for");
        lcd.setCursor(6,1);
        lcd.print("Pump Two");
        lcd.setCursor(0,3);
        lcd.print("Select        Cancel");

        //if press select, then continue with menu, else go back
        if(captureInput(&flowTwo)){
          UIState = SelectVolumeTwo;
        }else{
          UIState = ConfirmPumpOne;
        }
        
        lcd.clear();
        break;

      //** Menu Page to Select Volume for Pump Two *//
      case SelectVolumeTwo :
        lcd.setCursor(0,0);
        lcd.print("Select Volume for");
        lcd.setCursor(6,1);
        lcd.print("Pump Two");
        lcd.setCursor(6,2);
        lcd.print("0000");
        lcd.setCursor(0,3);
        lcd.print("Select        Cancel");

        //if select pressed, continue menu flow, otherwise go back
        if(captureInput(&volumeTwo)){
          UIState = ConfirmPumpTwo;
        }else{
          UIState = SelectFlowTwo;
        }
        
        lcd.clear();
        break;

      //** Menu Page to Confirm Flow and Volume for Pump Two **//
      case ConfirmPumpTwo :
        lcd.setCursor(0,0);
        lcd.print("Confirm Pump2 Values");
        lcd.setCursor(4,1);
        lcd.print("Flow");
        //display flow
        lcd.setCursor(9,1);
        lcd.print(flowTwo);
        lcd.setCursor(2,2);
        lcd.print("Volume");
        //display volume
        lcd.setCursor(10,2);
        lcd.print(volumeTwo);
        lcd.setCursor(0,3);
        lcd.print("Select        Cancel");

        //if select pressed, continue on with menu, otherwise go back
        if(captureButtons()){
          UIState = ExitMenu;
        }else{
          UIState = SelectFlowTwo;
        }

        lcd.clear();
        break;
      //** Menu Exit Page**//
      case ExitMenu : 
        //clearly exit menu
        return;

      case PrimePumps :
        lcd.setCursor(0,0);
        lcd.print("Please prime pumps.");
        lcd.setCursor(0,1);
        lcd.print("Please press select when done");
        lcd.setCursor(0,3);
        lcd.print("Select");

        //if select pressed, move to process running display
        if(captureButtons()){
          UIState = ProcessRunning;
        }

        lcd.clear();
        break;

      //** Display Process Running to LCD **//
      case ProcessRunning :
        lcd.setCursor(0,2);
        lcd.print("Process running...");
        return;

        break;

      //** Confirm the Process Has Finished **//
      case ConfirmEndProcess : 
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Please confirm");
        lcd.setCursor(0,1);
        lcd.print("process has ended.");
        lcd.setCursor(0,3);
        lcd.print("Select");

        //if select pressed, move to process running display
        if(captureButtons()){
          lcd.clear();
          return;
        }

        break;

      //** Confirm Lines are Clear **//
      case ConfirmClearedLines :
        lcd.setCursor(0,0);
        lcd.print("Please confirm");
        lcd.setCursor(0,1);
        lcd.print("lines are cleared.");
        lcd.setCursor(0,3);
        lcd.print("Select");

        //if select pressed, return to main loop
        if(captureButtons()){
          lcd.clear();
          return;
        }
        
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


//** Inputs: Address for value to be chosen **//
//** Returns: Integer value of Select = 1, Cancel = 0 **//
int captureInput(int* chosenValue){
  
  //holds state of buttons and pots
  int selectButtonState = LOW;
  int cancelButtonState = LOW;
  int lastSelectButtonState = LOW;
  int lastCancelButtonState = LOW;
  int readingSelectButton;
  int readingCancelButton;
  int readingCoarsePot;
  int readingFinePot;

  //holds calculated value based on two pots
  int tempFinePot;
  int tempCoarsePot;
  int value;

  //debounce variables
  unsigned long lastSelectButtonPress = 0;
  unsigned long lastCancelButtonPress = 0;
  unsigned long debounceTime = 50;

  
  while(true){
    //get button and potentiometer status
    readingSelectButton=digitalRead(selectButton);
    readingCancelButton=digitalRead(cancelButton);

    //display potentiometer readings to LCD
    //might want to make this a thing that happens on change
    //lcd can't keep up with the frequency of writes, dims
    //assumes 12 bit resolution on pins
    tempCoarsePot = map(analogRead(coarsePot), 0, 4095, 0, 400);
    tempFinePot = map(analogRead(finePot), 0, 4095, 0, 9); 
    value = (tempCoarsePot*10) + tempFinePot;
    //catch overflow from adding finePot if coarsePot is maxed out
    if(value>4000){
      value=4000;
    }
    lcd.setCursor(6,2);
    lcd.print(value);

    //check state of buttons changed, reset debounce timer
    if(lastSelectButtonState!=readingSelectButton){
      lastSelectButtonPress = millis();
    }else if(lastCancelButtonState!=readingCancelButton){
      lastCancelButtonPress = millis();
    }
    
    //Make sure waited for debounce delay on select button
    if(millis()-lastSelectButtonPress>debounceTime){
      //Want this action to happen only once per select button press
      if(readingSelectButton!=selectButtonState){
         selectButtonState = readingSelectButton;
         //only do something if the button is pressed
         if(selectButtonState = HIGH){
            //save the values for chosenValue for reference outside this function
            *chosenValue = value;
            return 1;
         }
      }
    }

    //make sure waited debounce delay on cancel button
    if(millis()-lastCancelButtonPress>debounceTime){
      //Want this action to happen only once per cancel button press
      if(readingCancelButton!=cancelButtonState){
         cancelButtonState = readingCancelButton;
         //only do something if the button is pressed
         if(cancelButtonState = HIGH){
            //go to menu landing state
            return 0;
         }
      }
    }
    lastSelectButtonState = readingSelectButton;
    lastCancelButtonState = readingCancelButton;
  }
}//end captureInput() function


int captureButtons(){
  
  //holds state of buttons and pots
  int selectButtonState = LOW;
  int cancelButtonState = LOW;
  int lastSelectButtonState = LOW;
  int lastCancelButtonState = LOW;
  int readingSelectButton;
  int readingCancelButton;

  //debounce variables
  unsigned long lastSelectButtonPress = 0;
  unsigned long lastCancelButtonPress = 0;
  unsigned long debounceTime = 50;

  while(true){
    //get button and potentiometer status
    readingSelectButton=digitalRead(selectButton);
    readingCancelButton=digitalRead(cancelButton);

    //check state of buttons changed, reset debounce timer
    if(lastSelectButtonState!=readingSelectButton){
      lastSelectButtonPress = millis();
    }else if(lastCancelButtonState!=readingCancelButton){
      lastCancelButtonPress = millis();
    }
    
    //Make sure waited for debounce delay on select button
    if(millis()-lastSelectButtonPress>debounceTime){
      //Want this action to happen only once per select button press
      if(readingSelectButton!=selectButtonState){
         selectButtonState = readingSelectButton;
         //only do something if the button is pressed
         if(selectButtonState = HIGH){
            return 1;
         }
      }
    }

    //make sure waited debounce delay on cancel button
    if(millis()-lastCancelButtonPress>debounceTime){
      //Want this action to happen only once per cancel button press
      if(readingCancelButton!=cancelButtonState){
         cancelButtonState = readingCancelButton;
         //only do something if the button is pressed
         if(cancelButtonState = HIGH){
            return 0;
         }
      }
    }
    lastSelectButtonState = readingSelectButton;
    lastCancelButtonState = readingCancelButton;
  }
}//end captureButtons()
