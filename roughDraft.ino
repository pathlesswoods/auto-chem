//arduino libraries
#include <LiquidCrystal.h>
#include <SD.h>
#include <RTCZero.h>
#include <SPI.h>

//custom libraries
//#include <MotorPresets.h>

//Defines for pins and variables for hardware
//LCD
//Motors <-variables should go in custom library
//Pumps
//Speaker
//SD
const int chipSelect=4;
//LEDS
//Buttons

//common global variables

void setup() {
  //setup SD
  Serial.begin(9600);
  while(!Serial){
    //gotta wait on the serial port connection
  }
  Serial.print("Initializing SD card...");
  if(!SD.begin(chipSelect)){
    Serial.println("Card failed, or not present at all");
    //don't do anything more
    while(true);
  }
  Serial.println("Card initialized. ");

  //set up clock
  
}

//main loop
void loop() {
  //create file using date and time to make the name
  //call menu function
  //File dataFile = SD.open("nameoffile", FILE_WRITE);
  //close after writing to it
}

//menu function
void menu(){
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
}
