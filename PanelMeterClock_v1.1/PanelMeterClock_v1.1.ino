/*



   Clock Display for PSC Lab in Military University of Technology


  @by Jakub Kołodziej



 */
 
#include <Wire.h>
#include <RTClib.h>
#include <Adafruit_SSD1306.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <Adafruit_BMP085.h>
#include <EEPROM.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

RTC_DS1307 rtc;

//define mbp180
Adafruit_BMP085 bmp;

/*************************DS18B20 temperature settings****************************/
// Data wire is plugged into pin 2 on the Arduino 
#define ONE_WIRE_BUS 9 
/********************************************************************/
// Setup a oneWire instance to communicate with any OneWire devices  
// (not just Maxim/Dallas temperature ICs) 
OneWire oneWire(ONE_WIRE_BUS); 
/********************************************************************/
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);
/********************************************************************/ 

// define pins usage
#define HOURPWM 5    //hour pwm out
#define MINPWM  6    //minutes pwm out

#define ADJ_SET 2    // Button SET MENU
#define ADJ_MINUS 3  // Button -
#define ADJ_PLUS  4  // Button +

#define ALARM_ON_OFF 8 //pin to set on/off of the alarm

// // buzzer pin
// #define BUZZER 10

// leds pins 
#define LED1 A3
#define LED2 A2
#define LED3 A1

//pin to set ground to the analog hour display
#define GROUND_HOUR 14

//pin to set ground to the alarm pin
#define GROUND_ALARM 7

//other variables
unsigned long previousMillis = 0;
const long interval = 2000;

uint8_t alarmHours = 0, alarmMinutes = 0;  // Holds the current alarm time

//variables for setting the rtc data
int hourupg;
int minupg;
int yearupg;
int monthupg;
int dayupg;
int menu = 0;

//char days of the week
char daysOfTheWeek[7][12] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

DateTime theTime; 			// Current clock time

// PWM values for hours 0 to 12

byte hPwm[] = { 
	13, 33, 51, 70, 89, 108, 126, 145, 164, 183, 203, 222, 241 
};

// PWM values for Minuets 0 to 59 

byte mPwm[] = { 
	  4,   7,  11,  15,  20,  23,  28,  31,  35,  39,
	 42,  47,  51,  54,  58,  62,  66,  70,  74,  78,
	 82,  86,  90,  94,  97, 101, 105, 109, 113, 117,
	120, 124, 128, 132, 137, 141, 145, 149, 154, 158,
	161, 165, 169, 173, 177, 181, 185, 189, 193, 197,
	201, 205, 209, 213, 217, 221, 225, 229, 233, 237 
};



//notes for alarm song

#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978
#define REST      0


// change this to make the song slower or faster
int tempo = 76;

// change this to whichever pin you want to use
int buzzer = 10;

// notes of the moledy followed by the duration.
// a 4 means a quarter note, 8 an eighteenth , 16 sixteenth, so on
// !!negative numbers are used to represent dotted notes,
// so -4 means a dotted quarter note, that is, a quarter plus an eighteenth!!
int melody[] = {

  // Wiegenlied (Brahms' Lullaby)
  // Score available at https://www.flutetunes.com/tunes.php?id=54

  NOTE_G4, 4, NOTE_G4, 4, //1
  NOTE_AS4, -4, NOTE_G4, 8, NOTE_G4, 4,
  NOTE_AS4, 4, REST, 4, NOTE_G4, 8, NOTE_AS4, 8,
  NOTE_DS5, 4, NOTE_D5, -4, NOTE_C5, 8,
  NOTE_C5, 4, NOTE_AS4, 4, NOTE_F4, 8, NOTE_G4, 8,
  NOTE_GS4, 4, NOTE_F4, 4, NOTE_F4, 8, NOTE_G4, 8,
  NOTE_GS4, 4, REST, 4, NOTE_F4, 8, NOTE_GS4, 8,
  NOTE_D5, 8, NOTE_C5, 8, NOTE_AS4, 4, NOTE_D5, 4,

  NOTE_DS5, 4, REST, 4, NOTE_DS4, 8, NOTE_DS4, 8, //8
  NOTE_DS5, 2, NOTE_C5, 8, NOTE_GS4, 8,
  NOTE_AS4, 2, NOTE_G4, 8, NOTE_DS4, 8,
  NOTE_GS4, 4, NOTE_AS4, 4, NOTE_C5, 4,
  NOTE_AS4, 2, NOTE_DS4, 8, NOTE_DS4, 8,
  NOTE_DS5, 2, NOTE_C5, 8, NOTE_GS4, 8,
  NOTE_AS4, 2, NOTE_G4, 8, NOTE_DS4, 8,
  NOTE_AS4, 4, NOTE_G4, 4, NOTE_DS4, 4,
  NOTE_DS4, 2

};

// sizeof gives the number of bytes, each int valsmaue is composed of two bytes (16 bits)
// there are two values per note (pitch and duration), so for each note there are four bytes
int notes = sizeof(melody) / sizeof(melody[0]) / 2;

// this calculates the duration of a whole note in ms
int wholenote = (60000 * 4) / tempo;

int divider = 0, noteDuration = 0;


void setup() {
	//Serial for debugging
	Serial.begin(9600);
		// start clock
	Wire.begin();
	rtc.begin();

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }

if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    // When time needs to be set on a new device, or after a power loss, the
    // following line sets the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

	pinMode(HOURPWM, OUTPUT);			// hour pwm pin
	pinMode(MINPWM, OUTPUT);			// minute pwm pin

	pinMode(ADJ_SET, INPUT);	
	pinMode(ADJ_MINUS, INPUT);		
  pinMode(ADJ_PLUS, INPUT);		
  pinMode(ALARM_ON_OFF, INPUT_PULLUP);	    
  
  pinMode(GROUND_HOUR, OUTPUT);
  pinMode(GROUND_ALARM, OUTPUT);

  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);



  pinMode(buzzer, OUTPUT);
    

//set the ground to the pin of the analog hour display
  digitalWrite(GROUND_HOUR, LOW);	

//set the ground to the pin of the alarm
  digitalWrite(GROUND_ALARM, LOW);	

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.

  // Clear the buffer
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);

 // Start up the library of temperature sensor
 sensors.begin(); 


//interrupt
attachInterrupt(digitalPinToInterrupt(2), read_adj_set, FALLING);

bmp.begin();


  //update alarm sets from eeprom

    alarmHours = EEPROM.read(1);
    alarmMinutes = EEPROM.read(16);



}
void loop() {




if(digitalRead(ADJ_MINUS)== HIGH && digitalRead(ADJ_PLUS)== HIGH)                                                                                                                                                                     
  {
    display.setCursor(0, 0);
    display.print("SET ALARM");
    display.display();      // Show initial text
    delay(1000);
    display.clearDisplay();

    DisplaySetHourAll();
    DisplaySetMinuteAll();

    display.setCursor(0, 0);
    display.print("ALARM");
    display.setCursor(0, 18);
    display.print(alarmHours);
    display.print(":");
    display.print(alarmMinutes);
    display.display();      // Show initial text
    display.clearDisplay();
    delay(1500);
  }


//set the time with buttons
 if(digitalRead(ADJ_SET)==HIGH)
  {
   menu=menu+1;
   delay(100);
  }
// in which subroutine should we go?
  if (menu==0)
    {
     DisplayDateTime(); // void DisplayDateTime
     Alarm(); // Alarm control
    }
  if (menu==1)
    {
    DisplaySetHour();
    }
  if (menu==2)
    {
    DisplaySetMinute();
    }
  if (menu==3)
    {
    DisplaySetYear();
    }
  if (menu==4)
    {
    DisplaySetMonth();
    }
  if (menu==5)
    {
    DisplaySetDay();
    }
  if (menu==6)
    {
    StoreAgg(); 
    delay(500);
    menu=0;
    }
    delay(100);

}

void DisplayDateTime ()
{
// We show the current date and time
  DateTime now = rtc.now();

unsigned long currentMillis = millis();

if (currentMillis - previousMillis >= interval) {
  previousMillis = currentMillis;

    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, HIGH);

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();

    //display things
    display.setCursor(0, 0);
    display.print(now.year());
    display.print('/');
    display.print(now.month());
    display.print('/');
    display.print(now.day());
    display.setCursor(0, 18);
    display.print(daysOfTheWeek[now.dayOfTheWeek()]);
    display.print(" ");
    display.print(now.hour());
    display.print(':');

    //if the minute is less than 10 display "0" before the single numer of minute
    if(now.minute() < 10){
      display.print("0");
      display.print(now.minute());
    }
    else{
    display.print(now.minute());
    }

    
    display.display();      // Show initial text
    display.clearDisplay();
    



  analogWrite(HOURPWM, hPwm[now.hour()%12]);
	analogWrite(MINPWM, mPwm[now.minute()]);  


  monthupg=now.month();
  yearupg=now.year();
}
else{

    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);

sensors.requestTemperatures();

Serial.println(sensors.getTempCByIndex(0));

    display.setCursor(0, 0);
    display.print(sensors.getTempCByIndex(0));
    display.print((char)247);
    display.print("C");
    display.setCursor(0, 18);
    display.print(bmp.readPressure()/100);
    display.print(" hPa");

    display.display();      // Show initial text
    display.clearDisplay();


}

    

}

void DisplaySetHour()
{
// time setting
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Set hour");
    Serial.println("Set hour");
    
  DateTime now = rtc.now();
  if(digitalRead(ADJ_PLUS)==HIGH)
  {
    if(hourupg==23)
    {
      hourupg=0;
    }
    else
    {
      hourupg=hourupg+1;
    }
  }
   if(digitalRead(ADJ_MINUS)==HIGH)
  {
    if(hourupg==0)
    {
      hourupg=23;
    }
    else
    {
      hourupg=hourupg-1;
    }
  }
      display.setCursor(0, 18);
    display.print(hourupg);
    Serial.println(hourupg);
    display.display();
    delay(200);
    display.clearDisplay();
//  
}

void DisplaySetMinute()
{
// Setting the minutes
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Set minute");
    Serial.println("Set minute");
    display.display();

  if(digitalRead(ADJ_PLUS)==HIGH)
  {
    if (minupg==59)
    {
      minupg=0;
    }
    else
    {
      minupg=minupg+1;
    }
  }
   if(digitalRead(ADJ_MINUS)==HIGH)
  {
    if (minupg==0)
    {
      minupg=59;
    }
    else
    {
      minupg=minupg-1;
    }
  }
    
    display.setCursor(0, 18);
    display.print(minupg);
    Serial.println(minupg);
    display.display();
    delay(200);
    display.clearDisplay();
}
  
void DisplaySetYear()
{
// setting the year
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Set year");
    Serial.println("Set year");
    
  if(digitalRead(ADJ_PLUS)==HIGH)
  {    
    yearupg=yearupg+1;
  }
   if(digitalRead(ADJ_MINUS)==HIGH)
  {
    yearupg=yearupg-1;
  }
      display.setCursor(0, 18);
    display.print(yearupg);
    Serial.println(yearupg);
    display.display();
    delay(200);
    display.clearDisplay();
}

void DisplaySetMonth()
{
// Setting the month
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Set month");
    Serial.println("Set month");
  if(digitalRead(ADJ_PLUS)==HIGH)
  {
    if (monthupg==12)
    {
      monthupg=1;
    }
    else
    {
      monthupg=monthupg+1;
    }
  }
   if(digitalRead(ADJ_MINUS)==HIGH)
  {
    if (monthupg==1)
    {
      monthupg=12;
    }
    else
    {
      monthupg=monthupg-1;
    }
  }
      display.setCursor(0, 18);
    display.print(monthupg);
    Serial.println(monthupg);
    display.display();
    delay(200);
    display.clearDisplay();  delay(200);
}

void DisplaySetDay()
{
// Setting the day
    display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Set day");
    Serial.println("Set day");

  if(digitalRead(ADJ_PLUS)==HIGH)
  {
    if (dayupg==31)
    {
      dayupg=1;
    }
    else
    {
      dayupg=dayupg+1;
    }
  }
   if(digitalRead(ADJ_MINUS)==HIGH)
  {
    if (dayupg==1)
    {
      dayupg=31;
    }
    else
    {
      dayupg=dayupg-1;
    }
  }
    display.setCursor(0, 18);
    display.print(dayupg);
    Serial.println(dayupg);
    display.display();
    delay(200);
    display.clearDisplay();
}

void StoreAgg()
{
      display.clearDisplay();
    display.setCursor(0, 0);
    display.print("Saving...");
    Serial.println("Saving");
    display.setCursor(0, 18);
    display.print("That's all!");
    display.display();
    delay(200);
    display.clearDisplay();
  rtc.adjust(DateTime(yearupg,monthupg,dayupg,hourupg,minupg,0));
    delay(200);
}

void read_adj_set(){
  
}


////alarm////

void DisplaySetHourAll()// Setting the alarm minutes
{
  while(digitalRead(ADJ_SET)==LOW){



  if(digitalRead(ADJ_PLUS)==HIGH)
  {
    if(alarmHours==23)
    {
      alarmHours=0;
    }
    else
    {
      alarmHours=alarmHours+1;
    }
  }
   if(digitalRead(ADJ_MINUS)==HIGH)
  {
    if(alarmHours==0)
    {
      alarmHours=23;
    }
    else
    {
      alarmHours=alarmHours-1;
    }
  }
    display.setCursor(0, 0);
    display.print("Set HOUR:");
    display.setCursor(0, 18);
    display.print(alarmHours);

    display.display();      // Show initial text
    display.clearDisplay();
    delay(200);
 }
  EEPROM.write(1, alarmHours);
 delay(200);
}




void DisplaySetMinuteAll()// Setting the alarm minutes
 {
  while(digitalRead(ADJ_SET)==LOW){ 


  if(digitalRead(ADJ_PLUS)==HIGH)
  {
    if (alarmMinutes==59)
    {
      alarmMinutes=0;
    }
    else
    {
      alarmMinutes=alarmMinutes+1;
    }
  }
   if(digitalRead(ADJ_MINUS)==HIGH)
  {
    if (alarmMinutes==0)
    {
      alarmMinutes=59;
    }
    else
    {
      alarmMinutes=alarmMinutes-1;
    }
  }

    display.setCursor(0, 0);
    display.print("Set MIN:");
    display.setCursor(0, 18);
    display.print(alarmMinutes);

    display.display();      // Show initial text
    display.clearDisplay();
    delay(200);
 }
   EEPROM.write(16, alarmMinutes);
 delay(200);
}



void Alarm(){

  if (digitalRead(ALARM_ON_OFF)== HIGH)
    {
    // printAllOff();
     noTone (buzzer);
     digitalWrite(LED3,LOW);
     }
  if (digitalRead(ALARM_ON_OFF)== LOW)
    {

     //printAllOn();    
     //led that state the alarm is on
    digitalWrite(LED3,HIGH);

       DateTime now = rtc.now();
     if ( (now.hour() == alarmHours) && (now.minute() == alarmMinutes) ) 
        {
        DateTime now = rtc.now();
         
        //  tone(BUZZER,880); //play the note 
        //  delay (300);
        //  tone(BUZZER,698); //play the note 
//-----------------------------------------------------------------------
          // iterate over the notes of the melody.
  // Remember, the array is twice the number of notes (notes + durations)
  for (int thisNote = 0; thisNote < notes * 2; thisNote = thisNote + 2) {

    // calculates the duration of each note
    divider = melody[thisNote + 1];
    if (divider > 0) {
      // regular note, just proceed
      noteDuration = (wholenote) / divider;
    } else if (divider < 0) {
      // dotted notes are represented with negative durations!!
      noteDuration = (wholenote) / abs(divider);
      noteDuration *= 1.5; // increases the duration in half for dotted notes
    }

    // we only play the note for 90% of the duration, leaving 10% as a pause
    tone(buzzer, melody[thisNote], noteDuration * 0.9);

    // Wait for the specief duration before playing the next note.
    delay(noteDuration);

    // stop the waveform generation before the next note.
    noTone(buzzer);
  }

  //----------------------------------------------------
        }
    else{
         noTone (buzzer);
        }
    
    } 
    delay(200);
}
