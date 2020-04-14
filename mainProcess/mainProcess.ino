//arduino libraries
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SD.h>
#include <SPI.h>
#include <RTCZero.h>
#include <MKRMotorCarrier.h>

//custom libraries
//#include <ValveControl.h>

/*
 * Definitions for Hardware Pins
 */
//LCD 
//uses I2C to communicate, no hardware pins defined
//Servo Motors 
//Have pins defined in the motorcarrier library
//Pumps
const int P1CTL = 2;
const int P2CTL = 7;
const int P1Speed = 5;
const int P2Speed = 4;
const int P1ReturnSignal = A6;
const int P2ReturnSignal = A1;
const int P1Alarm = 3;
const int P2Alarm = 1;
//Speaker
//const int speaker = 8;
//SD
//Potentiometers
const int coarsePot = A2;
//const int finePot = A2;
//LEDS
const int runningLED = 10;
const int alertLED = 9;
//Buttons
const int selectButton = 14;
const int cancelButton = 13;
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
//Pumps
bool pumpError = false;
//Motors
const int flowReagent1 = 30;
const int flowReagent2 = 30;
const int flowReagent3 = 30;
const int flowReagent4 = 30;
const int cleanLines1 = 90;
const int cleanLines2 = 90;
const int cleanLines3 = 90;
const int cleanLines4 = 90;
const int closed1 = 0;
const int closed2 = 0;
const int closed3 = 0;
const int closed4 = 0;

// ** states for the main loop ** //
const int initial = 0;
const int selectParameters = 1;
const int calculations = 2;
const int startProcess = 3;
const int processRunning = 4;
const int stopProcess = 5;
const int cleanUpProcess = 6;
const int emergencyShutdown = 7;
const int hardwareTesting = 8;
const int emergencyShutdownState = 9;
int state = initial;

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
const int emergencyNotification = 12;

//Runtime Variables
int flowOne;
int flowTwo;
int volumeOne;
int volumeTwo;

//runtimes in milliseconds
unsigned long runtime = 0;
unsigned long starttime = 0;

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

  //Establishing the communication with the motor shield
  if (controller.begin()) 
    {
      Serial.print("MKR Motor Shield connected, firmware version ");
      Serial.println(controller.getFWVersion());
    } 
  else 
    {
      Serial.println("Couldn't connect! Is the red led blinking? You may need to update the firmware with FWUpdater sketch");
      while (1);
    }

  // Reboot the motor controller; brings every value back to default
  Serial.println("reboot motor carrier");
  controller.reboot();
  
  //**Set up real-time clock**//
  rtc.begin();
  rtc.setTime(hours, minutes, seconds);
  rtc.setDate(day, month, year);

  //**Initiate the LCD**//
  lcd.init();
  lcd.backlight();

  //**Set up LEDs**//
  pinMode(alertLED, OUTPUT);
  pinMode(runningLED, OUTPUT);

  //**Set up buttons**//
  pinMode(selectButton, INPUT);
  pinMode(cancelButton, INPUT);
  //pinMode(emergencyButton, INPUT);
  //attachInterrupt(digitalPinToInterrupt(emergencyButton), emergencyShutdown,HIGH);

  //**Set up potentiometers **//
  pinMode(coarsePot, INPUT);
  //pinMode(finePot, INPUT);

  //**Set up speaker**//
  //pinMode(speaker, OUTPUT);

  //**Set up pumps **//
  pinMode(P1CTL, OUTPUT);
  digitalWrite(P1CTL, HIGH); //HIGH=OFF
  pinMode(P2CTL,OUTPUT);
  digitalWrite(P2CTL, HIGH); //HIGH=OFF
  pinMode(P1Speed, OUTPUT);
  analogWrite(P1Speed,150);//initial speed is 0
  Serial.print("\nSet P1Speed to 0.\n");
  pinMode(P2Speed, OUTPUT);
  analogWrite(P2Speed,255);//initial speed is 0
  Serial.print("\nSet P2Speed to 0.\n");
  pinMode(P1ReturnSignal, INPUT);
  pinMode(P2ReturnSignal, INPUT);
  pinMode(P1Alarm, INPUT);
  pinMode(P2Alarm, INPUT);
  
}//end setup function


//main loop
void loop() {
  
  //check pump error flag every loop
  if(pumpError){
    //set state to emergency state
    //state = emergencyShutdownState;
  }
  
  switch(state){

    //** Initialize Necessary Variables **//
    case initial:
    {
      Serial.print("\nIn initial state.\n");
      //initialize LEDs
      digitalWrite(alertLED, LOW);
      digitalWrite(runningLED, HIGH);

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
      Serial.print("\nselectParameters, going to MenuLanding UI\n");
      //call UI function to get user entered parameters
      doUserInterface(MenuLanding);

      Serial.print("\nselectParameters state, just returned form menuLanding\n");

      //calculate the runtime
      //milliliters per minute for flowrate of pumps
      /*
      float tempRunTimeOne = volumeOne/flowOne;
      float tempRunTimeTwo = volumeTwo/flowTwo;
      if(tempRunTimeOne>tempRunTimeTwo){
        runtime = tempRunTimeOne;
      }else{
        runtime = tempRunTimeTwo;
      }
      */
      
      runtime = 3000;
      
      //open file and write user selected values and runtime to it.
      logfile = SD.open(fileName, FILE_WRITE);
      if(logfile){
        Serial.print("Entering paramters into file.\n");
        logfile.println("User selected the following parameters: \n");
        logfile.println("Pump One Flow: \n");
        logfile.print(flowOne);
        logfile.println("\nPump One Volume: \n");
        logfile.print(volumeOne);
        logfile.println("\nPump Two Flow: \n");
        logfile.print(flowTwo);
        logfile.println("\nPump Two Volume: \n");
        logfile.print(volumeTwo);
        logfile.println("Calculated runtime is: \n");
        logfile.print(runtime);
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
      Serial.println("startProcess state\n");
      //set valves to flow reagent position
      //servo1.setAngle(flowReagent1);
      //delay(3000);
      //servo2.setAngle(flowReagent2);
      //delay(3000);
      //servo3.setAngle(flowReagent3);
      //delay(3000);
      //servo4.setAngle(flowReagent4);
      //delay(3000);

      //call UI function to ask user to prime pumps
      doUserInterface(PrimePumps);

      Serial.println("\nTurning pumps on and setting speeds.\n");
      //Set speeds, then turn pumps on
      analogWrite(P1Speed, 200);
      analogWrite(P2Speed, 200);
      //will probably have to do some math to convert 
      //selected flow into a suitable speed
      //analogWrite(P1Speed, flowOne);
      //analogWrite(P2Speed, flowTwo);
      digitalWrite(P1CTL,LOW);
      digitalWrite(P2CTL,LOW);
      Serial.print("Finished turning pumps on and setting speeds.");
      
      //Log the time the process started
      logfile = SD.open(fileName, FILE_WRITE);
      if(logfile){
        logfile.println("Process started at: ");
        String minutesDec = String(minutes, DEC);
        String hoursDec = String(hours, DEC);
        String dayDec = String(day, DEC);
        String monthDec = String(month, DEC);
        String currentDateTime = String(monthDec + "/" + dayDec + " " + hoursDec + ":" + minutesDec);
        logfile.print(currentDateTime);
        logfile.close();
      }else{
        Serial.println("Error with the file");
        //queue emergency shutdown?
      }

      //capture current time
      starttime = millis();
      state = processRunning;
      break;

    //** Process Running **//
    case processRunning:
      Serial.println("in processRunning");
      //periodically check pumps feedback for errors
      //should be receiving some analog signal
      if((analogRead(P1ReturnSignal)<15)||(analogRead(P2ReturnSignal)<15)){
        pumpError=true;
      }

      if((millis()-starttime)>runtime){
        state = stopProcess;
      }
      
      break;

    //** Stop Process **//
    case stopProcess:
    {
      Serial.println("stopProcess state, turning pumps off.");
      //command pump to turn off/stop pump
      digitalWrite(P1CTL,HIGH);
      digitalWrite(P2CTL,HIGH);
      //analogWrite(P1Speed, 0);
      //analogWrite(P2Speed, 0);
      Serial.print("Finished turning pumps off.");
 
      //Log the time the process ended
      logfile = SD.open(fileName, FILE_WRITE);
      if(logfile){
        logfile.println("Process ended at: ");
        String minutesDec = String(minutes, DEC);
        String hoursDec = String(hours, DEC);
        String dayDec = String(day, DEC);
        String monthDec = String(month, DEC);
        String currentDateTime = String(monthDec + "/" + dayDec + " " + hoursDec + ":" + minutesDec);
        logfile.print(currentDateTime);
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

    //** Clean up Lines **//
    case cleanUpProcess:
      Serial.println("cleanUpProcess state");
      //Set valves to flow inert gas to clear the lines
      //servo1.setAngle(cleanLines1);
      //delay(3000);
      //servo2.setAngle(cleanLines2);
      //delay(3000);
      //servo3.setAngle(cleanLines3);
      //delay(3000);
      //servo4.setAngle(cleanLines4);

      //Call UI to ask user if done clearing the lines.
      doUserInterface(ConfirmClearedLines);

      //Set valves to closed.
      //servo1.setAngle(closed1);
      //delay(3000);
      //servo2.setAngle(closed2);
      //delay(3000);
      //servo3.setAngle(closed3);
      //delay(3000);
      //servo4.setAngle(closed4);
      
      state = initial;
      break;

    //** Emergency Shutdown**//
    case emergencyShutdownState :
      Serial.println("emergencyShutdownState");
      //call emergency function
      doEmergencyShutdown();
      //notify user of emergency
      doUserInterface(emergencyNotification);
      //return to initial state.
      state = cleanUpProcess;
      break;
      
    //** Test and Troubleshoot Hardware **//
    case hardwareTesting:
    {  
      Serial.print("\nhardwareTesting\n");

      /*
      lcd.setCursor(0,0);
      lcd.print("Test input");
      int test = getPumpSetting();
      Serial.print("\nValue selected is...\n");
      Serial.print(test);
      */
      int select = digitalRead(selectButton);
      Serial.println("\nSelect button reads: ");
      Serial.print(select);
      int cancel = digitalRead(cancelButton);
      Serial.println("\ncancel button reads: ");
      Serial.print(cancel);
      delay(2000);
      
      
      /*
      if(captureButtons()){
        Serial.print("\nCaptured a select.\n");
      }else{
        Serial.print("\nCaptured a cancel.\n");
      }
      */
      /*
      Serial.print("\nCancel Button status: \n");
      Serial.print(digitalRead(cancelButton));
      Serial.print("\nSelect Button status: \n");
      Serial.print(digitalRead(selectButton));
      */
        
      //delay(5000);
    }
      break;
      
    //** Elegantly catch errors **//
    default:
      break; 
  }
  
  //delay for testing purposes
  //delay(500);
  
}//end main loop function


//Pad a zero in front of digits
String padDigits(int number){
  if(number <10){
    return String("0" + number);
  }else{
    return String(number);
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

        Serial.println("\nMenuLanding UIState\n");
        //if select, then go to next menu page, otherwise do nothing
        if(captureButtons()){
          UIState = SelectFlowOne;
        }
        
        lcd.clear();
        break;

      //** Menu Page to Select Flow for Pump One **//
      case SelectFlowOne :
        Serial.println("\nselectFlowOne UIState\n");
        //display initial LCD message
        lcd.setCursor(3,0);
        lcd.print("Select Flow for");
        lcd.setCursor(6,1);
        lcd.print("Pump One");
        lcd.setCursor(7,2);
        lcd.print("0000");
        lcd.setCursor(0,3);
        lcd.print("Select        Cancel");

        flowOne = getPumpSetting();
        lcd.setCursor(7,2);
        lcd.print("    ");
        lcd.setCursor(7,2);
        lcd.print(flowOne);


        //call captureInputs function
        if(captureButtons()){
          UIState = SelectVolumeOne;
        }else{
          UIState = MenuLanding;
        }
        
        lcd.clear();
        break;

      //** Menu Page to Select Volume for Pump One **//
      case SelectVolumeOne :
        Serial.println("\nselectVolumeOne UIState\n");
        lcd.setCursor(0,0);
        lcd.print("Select Volume for");
        lcd.setCursor(6,1);
        lcd.print("Pump One");
        lcd.setCursor(7,2);
        lcd.print("0000");
        lcd.setCursor(0,3);
        lcd.print("Select        Cancel");

        volumeOne = getPumpSetting();
        lcd.setCursor(7,2);
        lcd.print("    ");
        lcd.setCursor(7,2);
        lcd.print(volumeOne);
        
        //call captureInputs function
        if(captureButtons()){
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
        Serial.println("ConfirmPumpOne UIState");
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
        lcd.setCursor(9,2);
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
        
      //** Menu Page to Select Flow for Pump Two **//
      case SelectFlowTwo :
        Serial.println("selectFlowTwo UIState");
        lcd.setCursor(0,0);
        lcd.print("Select Flow for");
        lcd.setCursor(6,1);
        lcd.print("Pump Two");
        lcd.setCursor(7,2);
        lcd.print("0000");
        lcd.setCursor(0,3);
        lcd.print("Select        Cancel");

        flowTwo = getPumpSetting();
        lcd.setCursor(7,2);
        lcd.print("    ");
        lcd.setCursor(7,2);
        lcd.print(flowTwo);
        
        
        //if press select, then continue with menu, else go back
        if(captureButtons()){
          UIState = SelectVolumeTwo;
        }else{
          UIState = ConfirmPumpOne;
        }
        
        lcd.clear();
        break;

      //** Menu Page to Select Volume for Pump Two *//
      case SelectVolumeTwo :
        Serial.println("selectVolumeOne UIState");
        lcd.setCursor(0,0);
        lcd.print("Select Volume for");
        lcd.setCursor(6,1);
        lcd.print("Pump Two");
        lcd.setCursor(7,2);
        lcd.print("0000");
        lcd.setCursor(0,3);
        lcd.print("Select        Cancel");

        //get selected setting from user and display
        volumeTwo = getPumpSetting();
        lcd.setCursor(7,2);
        lcd.print("    ");
        lcd.setCursor(7,2);
        lcd.print(volumeTwo);
        
        //if select pressed, continue menu flow, otherwise go back
        if(captureButtons()){
          UIState = ConfirmPumpTwo;
        }else{
          UIState = SelectFlowTwo;
        }
        
        lcd.clear();
        break;

      //** Menu Page to Confirm Flow and Volume for Pump Two **//
      case ConfirmPumpTwo :
        Serial.println("ConfirmPumpTwo UIState");
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
        lcd.setCursor(9,2);
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
        Serial.println("ExitMenu UIState");
        //clearly exit menu
        return;

      case PrimePumps :
        Serial.println("PrimePumps UIState");
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Please prime pumps.");
        lcd.setCursor(0,1);
        lcd.print("Please press select"); 
        lcd.setCursor(5,2);
        lcd.print("when done");
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
        Serial.println("ProcessRunning UIState");
        lcd.setCursor(0,2);
        lcd.print("Process running...");
        return;

        break;

      //** Confirm the Process Has Finished **//
      case ConfirmEndProcess : 
        Serial.println("ConfirmEndProcess UIState");
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
        Serial.println("ConfirmClearedLines UIState");
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

      //** Emergency Shutdown notification **//
      case emergencyNotification :
        Serial.println("emergencyNotification UIState");
        lcd.clear();
        lcd.setCursor(7,0);
        lcd.print("Pump Fault");
        lcd.setCursor(0,1);
        lcd.print("Emergency Shutdown");
        //lcd.setCursor(0,3);
        //lcd.print("Select");
        if(captureButtons()){
          return;
        }
        break;
        
      default :
        break;
    }
  }
}//end doUserInterface function


void doEmergencyShutdown(){
  Serial.println("in emergency shutdown function");
  //send command to turn on emergencyLED  
  digitalWrite(alertLED, HIGH);
  
  //send command to turn off pumps
  digitalWrite(P1CTL, HIGH);
  digitalWrite(P2CTL, HIGH);
  //analogWrite(P1Speed, 0);
  //analogWrite(P2Speed, 0);
  
  //send command to turn valves to closed
  //servo1.setAngle(closed1);
  //servo2.setAngle(closed2);
  //servo3.setAngle(closed3);
  //servo4.setAngle(closed4);
 
  //Write current time and emergency message to SD
  logfile = SD.open(fileName, FILE_WRITE);
  if(logfile){
    logfile.println("Emergency shutdown procedure initiated.");
        
    // Print date...  
    logfile.print(padDigits(rtc.getDay())); 
    logfile.print("/");
    logfile.print(padDigits(rtc.getMonth()));
    logfile.print("/");
    logfile.print(padDigits(rtc.getYear()));
    logfile.print(" ");

    // Print time
    logfile.print(padDigits(rtc.getHours()));
    logfile.print(":");
    logfile.print(padDigits(rtc.getMinutes()));
    logfile.print(":");
    logfile.print(padDigits(rtc.getSeconds()));

    logfile.println();
    logfile.close();
    Serial.println("Date and time recorded successfully");
    
  }
  else{
       Serial.println("Error with the file, emergency shutdown initiated.");
  }

  //don't want this here, will never be able to turn it off.
  /*
  while(1){
    //sound alert through speaker
    //digitalWrite(speaker, HIGH);
    delay(100);
    //digitalWrite(speaker,LOW);
    delay(1000);
  }
  */
  return;
  
}//end emergencyShutdown function


/* 
 * Function that gets a 4 digit number from user
 * Inputs: 
 * Returns: Integer value of pump setting chosen
*/ 
int getPumpSetting(){
  //variables for switch statment
  int digit=1;
  const int firstDigit=1;
  const int secondDigit=2;
  const int thirdDigit=3;
  const int fourthDigit=4;
  const int finish=5;

  //Variables to set
  int value=0;
  int firstValue=0;
  int secondValue=0;
  int thirdValue=0;
  int fourthValue=0;

  Serial.println("in getPumpSettings");
  //run until all positions are filled
  while(1){
    switch(digit){
      case firstDigit :
        if(captureInput(&firstValue, firstDigit)){
          Serial.print("\nFirst digit: ");
          Serial.print(firstValue); 
          digit=secondDigit;
          Serial.print("\ndigit is now currently: \n");
          Serial.print(digit);
        }//no else, ignore if user hits cancel
        break;
      case secondDigit :
        if(captureInput(&secondValue, secondDigit)){
          Serial.print("\nSecond digit: ");
          Serial.print(secondValue);
          digit=thirdDigit;
        }else{
          Serial.print("\nSelected cancel, back to first digit.\n");
          digit=firstDigit;
        }
        break;
      case thirdDigit :
        if(captureInput(&thirdValue, thirdDigit)){
          Serial.print("\nThird digit: ");
          Serial.print(thirdValue);
          digit=fourthDigit;
        }else{
          Serial.print("\nSelected cancel, back to second digit.\n");
          digit=secondDigit;
        }
        break;
      case fourthDigit :
        if(captureInput(&fourthValue, fourthDigit)){
          Serial.print("\nFourth digit: ");
          Serial.print(fourthValue);
          digit=finish;
        }else{
          Serial.print("\nSelected cancel, back to third digit.\n");
          digit=thirdDigit;
        }
        break;
      case finish : 
        value = (firstValue*1000)+(secondValue*100)+(thirdValue*10)+fourthValue;
        Serial.print("\nMade it to finish, value is...\n");
        Serial.print(value);
        return value;
      default:
        //something went wrong if you're here
        Serial.print("Error reading in pump value.");
        break;
    }
  }
}//end getPumpSettings

/* Inputs: Address for value to be chosen, integer value of place
 *  of value (1-4)
 * Returns: Integer value of Select = 1, Cancel = 0 
*/
bool captureInput(int* chosenValue, int placeValue){
  
  //holds state of buttons and pots
  int selectButtonState = digitalRead(selectButton);
  int cancelButtonState = digitalRead(cancelButton);
  int lastSelectButtonState = selectButtonState;
  int lastCancelButtonState = cancelButtonState;
  
  int readingSelectButton;
  int readingCancelButton;

  //holds calculated value based on two pots
  int lastValue=0;
  int value=0;

  //debounce variables
  unsigned long lastSelectButtonPress = 0;
  unsigned long lastCancelButtonPress = 0;
  unsigned long debounceTime = 120;
  
  while(true){
    //get button and potentiometer status
    readingSelectButton=digitalRead(selectButton);
    readingCancelButton=digitalRead(cancelButton);

    //Display potentiometer readings to LCD only on 
    //change otherwise LCD can't keep up with frequency of writes.
    value = map(analogRead(coarsePot), 0, 1023, 0, 9);
    if(value!=lastValue){
      //added 6 to place value to get it in middle-ish of LCD
      lcd.setCursor((placeValue+6),2);
      lcd.print(value);
      lastValue=value;
    }

    //check state of buttons changed, reset debounce timer
    if(lastSelectButtonState!=readingSelectButton){
      lastSelectButtonPress = millis();
    }
    if(lastCancelButtonState!=readingCancelButton){
      lastCancelButtonPress = millis();
    }
    
    //Make sure waited for debounce delay on select button
    if(millis()-lastSelectButtonPress>debounceTime){
      //Want this action to happen only once per select button press
      if(readingSelectButton==HIGH && selectButtonState==LOW){
         *chosenValue=value;
         return true;
      }else{
        selectButtonState=readingSelectButton;
      }
    }

    //make sure waited debounce delay on cancel button
    if(millis()-lastCancelButtonPress>debounceTime){
       //Want this action to happen only once per cancel button press
       if(readingCancelButton == HIGH && cancelButtonState==LOW){
          return false;
       }else{
        cancelButtonState=readingCancelButton;
       }
    }
    lastSelectButtonState = readingSelectButton;
    lastCancelButtonState = readingCancelButton;
  }
}//end captureInput


/* Function to specifically capture 
 * Select or Cancel Button presses
 * Returns: 1 for Select, 0 for Cancel
*/ 
bool captureButtons(){
 
  //holds state of buttons and pots
  int selectButtonState = digitalRead(selectButton);
  int cancelButtonState = digitalRead(cancelButton);
  int lastSelectButtonState = selectButtonState;
  int lastCancelButtonState = cancelButtonState;
  
  int readingSelectButton;
  int readingCancelButton;
  
  //debounce variables
  unsigned long lastSelectButtonPress = 0;
  unsigned long lastCancelButtonPress = 0;
  unsigned long debounceTime = 120;

  while(true){
    //get button and potentiometer status
    readingSelectButton=digitalRead(selectButton);
    readingCancelButton=digitalRead(cancelButton);
    /*
    Serial.println("\ncaptureButton, Select = ");
    Serial.print(readingSelectButton);
    Serial.println("\ncaptureButton, Cancel = ");
    Serial.print(readingCancelButton);
    */

    //check state of buttons changed, reset debounce timer
    if(lastSelectButtonState!=readingSelectButton){
      lastSelectButtonPress = millis();
    }
    if(lastCancelButtonState!=readingCancelButton){
      lastCancelButtonPress = millis();
    }
    
    //Make sure waited for debounce delay on select button
    if(millis()-lastSelectButtonPress>debounceTime){
      //Want this action to happen only once per select button press
      if(readingSelectButton==HIGH && selectButtonState==LOW){
         return true;
      }else{
        selectButtonState=readingSelectButton;
      }
    }

    //make sure waited debounce delay on cancel button
    if(millis()-lastCancelButtonPress>debounceTime){
       //Want this action to happen only once per cancel button press
       if(readingCancelButton == HIGH && cancelButtonState==LOW){
          return false;
       }else{
        cancelButtonState=readingCancelButton;
       }
    }
    lastSelectButtonState = readingSelectButton;
    lastCancelButtonState = readingCancelButton;
  }
}//end captureButtons()
