
//
//#include <Adafruit_Keypad.h>
//#include <Adafruit_Keypad_Ringbuffer.h>


//SEJAL PATEL
//6.1.23
//SAFE CLOCK
//Copyright <YEAR> <COPYRIGHT HOLDER>
//Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
//The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

//*************************
//GLOBALS
//************************

#include <Wire.h>
#include <DS3231.h>
#include <LiquidCrystal.h>
//#include <Keypad.h>
#include <Servo.h>
#include <Arduino.h>
#include <radio.h>
#include <TEA5767.h>
#include <Key.h>
#include <Keypad.h>



//servo safe
Servo myservo;  // create servo object to control a servo\
//pin 14

//LCD
// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
//numbers are in the middle
//all on yellow
const int rs = 6, en = 5, d4 = 4, d5 = 3, d6 = 2, d7 = 1;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

//keypad
const byte ROWS = 4; //four rows
const byte COLS = 4; //4 columns
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'.', '0', '#', 'D'},
};
byte rowPins[ROWS] = {8, 9, 10, 11}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {23, 22, 21, 20}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
String outPass =  "";  // finished string location
String tempPass = " "; //String to build on
//int counter;
char key; // keypad characters

//RTC
RTClib myRTC;
// red on 19 yellow
//orange on 18 yellow
//yellow on 17 red
//green on 17 black
int thour; //current hour
int tmin; //current minute

bool sausage = false; //initializes the clock and other functions

String alarmHour; //alarm hour
String alarmMin; //alarm minute

//radio

String radioFM; //radio channel inputted from keypad

/// The band that will be tuned by this sketch is FM.
#define FIX_BAND RADIO_BAND_FM

/// The station that will be tuned by this sketch is 89.30 MHz.
#define FIX_STATION
//String chanel = "FIX_STATION 9650";
TEA5767 radio;    // Create an instance of Class for Si4703 Chip
//green on A2 black
//blue on A2 red
//white on A5
//orange on A4

//button
const int buttPin = 7;
int buttState = 0;
bool muted;
bool switchButt = false;
String vol;

//***********************************
//SETUP
//***********************************
void setup() {
  lcd.begin(16, 2);
  Serial.begin(9600);
  Wire.begin();
  delay(500);
  myservo.attach(14);  // attaches the servo on pin 14 to the servo object
  //lcd.write('G');
  key = keypad.getKey(); // get key
  lcd.clear();
  lcd.print(key); // key is printed
  lcd.clear();
  pinMode(7, INPUT_PULLUP); //button pin
  radio.init();
}
//***********************************
//LOOP
//***********************************
void loop() {
  if (sausage == false) {
    ticktock();
  }
  safe();
  checktime();
  radioButt();

}

//***********************
//radio
//*************************
void radioButt() {
  //  radioFM = "96.5";
  buttState = digitalRead(buttPin);
  Serial.println(buttState);
  if (buttState == 0) {
    delay(50);
    if (switchButt == false) { //turns on the radio
      //      delay(1500);
      Serial.println("state = " + String(switchButt)); // check state
      radio.setBandFrequency(FIX_BAND, radioFM.toInt()); //sets the radio from the keypad
      //      radio.setVolume(9);
      //      radio.setMute(false);
      switchButt = true;
      Serial.println("chan = " + String(radioFM.toFloat())); // check channel

    }
    else if (switchButt == true) { // turns off the radio
      //      delay(1500);
      Serial.println("state = " + String(switchButt)); //check state
      radio.setMute(true);
      switchButt = false;
    }
  }
}

//*******************
//clock
//******************
void ticktock() { // sets date and time
  delay(1000);

  DateTime now = myRTC.now();

  lcd.clear();
  lcd.cursor();
  lcd.setCursor(0, 0);
  thour = now.hour();
  //  lcd.print(now.hour(), DEC);
  lcd.print(thour);
  lcd.write(':');
  tmin = now.minute();
  if (tmin < 10) { //sets 0 infront of a single digit
    lcd.write('0');
    lcd.print(now.minute(), DEC);
  }
  else {
    lcd.print(now.minute(), DEC);
  }
  lcd.write(':');
  if (now.second() < 10) {  //sets 0 infront of a single digit
    lcd.write('0');
    lcd.print(now.second(), DEC);
  }
  else {
    lcd.print(now.second(), DEC);
  }
  lcd.setCursor(0, 1);
  lcd.print(now.month(), DEC); //curent month
  lcd.write('/');
  lcd.print(now.day(), DEC); //current day
  lcd.write('/');
  lcd.print(now.year(), DEC); // current year
}
void print2digits(int number) { // honestly useless but it's here cuz it doesn't mess anything up
  if (number >= 0 && number < 10) {
    lcd.write('0');
  }
  lcd.print(number);
}

//********************************
//safe
//*********************************


void safe() {
  key = keypad.getKey(); // get key
  if (key) { //if key is pressed
    sausage = true;
    lcd.clear();
    lcd.print(key); // key is printed
    if (key == '#') { // password over
      lcd.clear();
      lcd.setCursor(0, 0);
      outPass = tempPass;  // check password
      tempPass += key;  // append #
      //      Serial.println(tempPass);
      if (tempPass == "3425#") { //correct pass
        lcd.setCursor(0, 1);
        lcd.print("correct");
        myservo.write(220);            // UNLOCKS SAFE
        delay(1500);
        lcd.setCursor(0, 0);
        sausage = false;
      }
      //      else if (tempPass == "2625#") {
      //        lcd.clear();
      //        sausage = false;
      //      }
      else { //wrong pass
        lcd.setCursor(0, 1);
        lcd.print("incorrect");
        lcd.setCursor(0, 0);
        myservo.write(0);
        sausage = false;
      }
      tempPass = "";        // Clear tempStr Buffer
    }
    else if (key == 'A') {//alarm hour
      alarmHour = tempPass;
      tempPass = ' ';
      sausage = true;
    }
    else if (key == 'B') {//alarm min
      alarmMin = tempPass;
      tempPass = ' ';
      sausage = false;
    }
    else if (key == 'C') { // radio chan
      radioFM = tempPass;
      tempPass = ' ';
      sausage = false;
    }
    //    else if (key == 'D') {
    //      vol = tempPass;
    //      radio.setVolume(vol.toInt()); // might have to put this is radiobutt
    //      tempPass = ' ';
    //      sausage = false;
    //    }
    else { //upload to temppass
      tempPass += key; // append characters until a '#' is found.
//      tempPass = tempPass.trim();
      tempPass.trim();
      sausage = true;
    }
  }
}
void checktime() { //alarm
  if (alarmHour.toInt() == thour && alarmMin.toInt() == tmin) { // if it is the hour and minute of the alarm
    //    sausage = true;
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(radioFM);
    radio.setBandFrequency(FIX_BAND, radioFM.toInt()); //play radio
    delay(1000);
  }
//  else {
//    alarmHour = "";
//    alarmMin = "";
//  }
}
