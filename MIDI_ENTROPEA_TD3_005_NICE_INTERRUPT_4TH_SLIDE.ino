/*************************************************
 * MIDI Out Melody Example
 *
 * Author: James Saunders
 *************************************************/
/* ^^ idk who this dude is but whatever.

*/

// FIRE UP THE REACTOR!
/*
 * This is set up to use the TD3 and works pretty well,
 * the main issue being that the timing is affected by
 * the note length which can mean the timing gets a bit
 * funky on long notes
 * 
 * Also works grat wiht the xsttion on patch 134 Beauty lives
 * 
 * DONE!! the plan is to switch to an interrupt-based system
 * using 
 * 
 * long int t1 = millis();
 * 
 * to measure the timing
 * 
 * Also would like to add another sensor on the right 
 * to allow something like "slide" (like we have accent
 * on the left), which would likely also work better 
 * on an interrupt-based system.
 * 
 * on X-station
 * 
 * Attack:      108
 * Release:     111
 * LFO depth:   102
 * Mod Attack:  114
 * Mod Release: 117
 * 
 * 
 * 
 */




// ---------------- screen stuff 
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
// The pins for I2C are defined by the Wire-library. 
// On an arduino UNO:       A4(SDA), A5(SCL)
// On an arduino MEGA 2560: 20(SDA), 21(SCL)
// On an arduino LEONARDO:   2(SDA),  3(SCL), ...
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES     10 // Number of snowflakes in the animation example


#define LOGO_HEIGHT   16
#define LOGO_WIDTH    16
static const unsigned char PROGMEM logo_bmp[] =
{ 0b00000000, 0b11000000,
  0b00000001, 0b11000000,
  0b00000001, 0b11000000,
  0b00000011, 0b11100000,
  0b11110011, 0b11100000,
  0b11111110, 0b11111000,
  0b01111110, 0b11111111,
  0b00110011, 0b10011111,
  0b00011111, 0b11111100,
  0b00001101, 0b01110000,
  0b00011011, 0b10100000,
  0b00111111, 0b11100000,
  0b00111111, 0b11110000,
  0b01111100, 0b11110000,
  0b01110000, 0b01110000,
  0b00000000, 0b00110000 };



// ---------------- Ultrasonic oinker
#include <HCSR04.h>
UltraSonicDistanceSensor ds1(3, 2);  // Initialize sensor that uses digital pins 3 and 2.
UltraSonicDistanceSensor ds2(4, 5);  // Initialize sensor that uses digital pins 4 and 5.
UltraSonicDistanceSensor ds3(6, 7);  // Initialize sensor that uses digital pins 6 and 7.
UltraSonicDistanceSensor ds4(8, 9);  // Initialize sensor that uses digital pins 8 and 9.
UltraSonicDistanceSensor ds5(11, 10);  // Initialize sensor that uses digital pins 8 and 9.

// ---------------- MIDI stuff 

#include "midiNote2Frequency.h"
#include <MIDI.h>

int midichan = 3;

const int beat_length = 500;  //ms = 120bpm
int noteDuration = beat_length/4;


long int t1 = millis();
const int i1 = beat_length/64;  // ms interrupt/timing accuracy. Effectively quantizing at 1/16th note
int ino = false;                // is note on
long int noti = 0;                    // note on time
int nodu = 0;                    // note on duration - i.e. the note's duration at time of creation (could be overridden by slide later)
int cn = 0;                     // current note
long int pbnti = 0;                   // pause between notes time
int pbndu = 0;                   // pause between notes duration
int slidecnt = 0;


const uint8_t midiChannel1 = 5;
const uint8_t midiChannel2 = 4;
const uint8_t midiChannel3 = 5;

// Notes in the melody.
//int melody[] = {MIDI_E3,  MIDI_E4, MIDI_E3, MIDI_G3, MIDI_A3, MIDI_G3, NULL, MIDI_B3, MIDI_C4};
//int melody[] = {MIDI_E3, MIDI_E4, MIDI_E3, MIDI_G3, MIDI_A3, MIDI_G3, MIDI_B3, MIDI_C4};
//int melody[] = {MIDI_E2, MIDI_F2, MIDI_G2, MIDI_A3, MIDI_B3, MIDI_C3, MIDI_D3, MIDI_E3, MIDI_G3, MIDI_B3, MIDI_C4};

// full note range
int melody[] = { MIDI_G9, MIDI_FS9, MIDI_F9, MIDI_E9, MIDI_DS9, MIDI_D9, MIDI_CS9, MIDI_C9, MIDI_B8, MIDI_AS8, MIDI_A8, MIDI_GS8, MIDI_G8, MIDI_FS8, MIDI_F8, MIDI_E8, MIDI_DS8, MIDI_D8, MIDI_CS8, MIDI_C8, MIDI_B7, MIDI_AS7, MIDI_A7, MIDI_GS7, MIDI_G7, MIDI_FS7, MIDI_F7, MIDI_E7, MIDI_DS7, MIDI_D7, MIDI_CS7, MIDI_C7, MIDI_B6, MIDI_AS6, MIDI_A6, MIDI_GS6, MIDI_G6, MIDI_FS6, MIDI_F6, MIDI_E6, MIDI_DS6, MIDI_D6, MIDI_CS6, MIDI_C6, MIDI_B5, MIDI_AS5, MIDI_A5, MIDI_GS5, MIDI_G5, MIDI_FS5, MIDI_F5, MIDI_E5, MIDI_DS5, MIDI_D5, MIDI_CS5, MIDI_C5, MIDI_B4, MIDI_AS4, MIDI_A4, MIDI_GS4, MIDI_G4, MIDI_FS4, MIDI_F4, MIDI_E4, MIDI_DS4, MIDI_D4, MIDI_CS4, MIDI_C4, MIDI_B3, MIDI_AS3, MIDI_A3, MIDI_GS3, MIDI_G3, MIDI_FS3, MIDI_F3, MIDI_E3, MIDI_DS3, MIDI_D3, MIDI_CS3, MIDI_C3, MIDI_B2, MIDI_AS2, MIDI_A2, MIDI_GS2, MIDI_G2, MIDI_FS2, MIDI_F2, MIDI_E2, MIDI_DS2, MIDI_D2, MIDI_CS2, MIDI_C2, MIDI_B1, MIDI_AS1, MIDI_A1, MIDI_GS1, MIDI_G1, MIDI_FS1, MIDI_F1, MIDI_E1, MIDI_DS1, MIDI_D1, MIDI_CS1, MIDI_C1, MIDI_B0, MIDI_AS0, MIDI_A0 };

// for TD3
//int melody[] = { MIDI_AS6, MIDI_A6, MIDI_GS6, MIDI_G6, MIDI_FS6, MIDI_F6, MIDI_E6, MIDI_DS6, MIDI_D6, MIDI_CS6, MIDI_C6, MIDI_B5, MIDI_AS5, MIDI_A5, MIDI_GS5, MIDI_G5, MIDI_FS5, MIDI_F5, MIDI_E5, MIDI_DS5, MIDI_D5, MIDI_CS5, MIDI_C5, MIDI_B4, MIDI_AS4, MIDI_A4, MIDI_GS4, MIDI_G4, MIDI_FS4, MIDI_F4, MIDI_E4, MIDI_DS4, MIDI_D4, MIDI_CS4, MIDI_C4, MIDI_B3, MIDI_AS3, MIDI_A3, MIDI_GS3, MIDI_G3, MIDI_FS3, MIDI_F3, MIDI_E3, MIDI_DS3, MIDI_D3, MIDI_CS3, MIDI_C3 };

// Note durations (4 = quarter note, 8 = eighth note etc).
int noteDurations[] = {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4};

int multiplier = 255;

int lastNote;

int rangeMax = 30;
int rangeMin = 0;

int cc1 = 34;
int cc2 = 64;
int cc3 = 64;


// Created and binds the MIDI interface to the default hardware serial port.
MIDI_CREATE_DEFAULT_INSTANCE();

void setup() {
  // Listen to all incoming messages
  MIDI.begin(MIDI_CHANNEL_OMNI);

  MIDI.setHandleControlChange(handlecc);
  //Serial.begin(9600);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    //Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  
  display.display();
  delay(1000); // Pause for 2 seconds

  // Clear the buffer
  display.clearDisplay();

  // Draw a single pixel in white
  //display.drawPixel(10, 10, SSD1306_WHITE);

  display.setTextSize(1);
  display.setTextColor(WHITE);
  
  display.setCursor(14, 14);
  display.print("ENTROPEA AWAKENS");
  display.display();
  delay(2000);

  display.print(".");
  display.display();
  delay(200);

  display.print(".");
  display.display();
  delay(200);

  allnotesoff();

}



void handlecc(byte channel, byte number, byte value)
  {
  multiplier = value;
  }


void allnotesoff()
  {
  int n = 0;
  for ( n=0; n<sizeof(melody)-1; n++)
    {
    MIDI.sendNoteOff(melody[n], 0, midichan);
    }
  slidecnt = 0;
  noti = 0;
  nodu = 0;
  cn = 0;
  ino = false;
  pbnti = t1;
  pbndu = noteDuration * 0.75;  
  }


void loop() 
  {
  t1 = millis();
  
  //int noteDuration = beat_length/4;

  int thisNote;
  int pauseBetweenNotes;
  int note;
  int velocity;
  
  boolean inRange1 = false;
  boolean inRange2 = false;
  boolean inRange3 = false;
  boolean inRange4 = false;
  boolean inRange5 = false;
  
  if ( !cc1 )
    { cc1 = 34; }
  
  // ======================================== SENSOR STUFF

  float us1 = ds1.measureDistanceCm();
  float us2 = ds2.measureDistanceCm();
  float us3 = ds3.measureDistanceCm();
  float us4 = ds4.measureDistanceCm();
  float us5 = ds5.measureDistanceCm();
    
  if ( us1 < rangeMax and us1 > 0 )
    { 
    inRange1 = true;
    note = int((us1*2)+40);
    }
//  else
//    {
//    noti = 0;
//    nodu = 0;
//    cn = 0;
//    ino = false;
//    }


  if ( us2 < rangeMax and us2 > 0 )
    { 
    inRange2 = true;
    // noteDuration = int(us2*10); // << this was the last active
    if ( us2 < (rangeMax/16) )
      {
      noteDuration = beat_length/32;
      }
    else if ( us2 < (rangeMax/8) )
      {
      noteDuration = beat_length/16;
      }
    else if ( us2 < (rangeMax/4) )
      {
      noteDuration = beat_length/8;
      }
    else if ( us2 < (rangeMax/2) )
      {
      noteDuration = beat_length/4;
      }
    else if ( us2 < ((rangeMax/4)*3) )
      {
      noteDuration = beat_length/2;
      }
//    else if ( us2 < ((rangeMax/8)*7) )
//      {
//      noteDuration = beat_length/2;
//      }
    else
      {
      noteDuration = beat_length/1;
      }
    }
  else
    {
    //noteDuration = 200;
    if ( !noteDuration )
      { 
      noteDuration = beat_length/4;
      }
    }

    
  if ( us3 < rangeMax and us3 > 0 )
    { 
    inRange3 = true;
    cc1 = int(us3*3);
    }  
  velocity = 75;
  if ( inRange3 ) 
    {
    velocity = 100;
    }
  else
    {
    velocity = 60;
    }


  if ( us4 < rangeMax and us4 > 0 )
    { 
    inRange4 = true;
    cc2 = int(us4*3);
    }  
  
  if ( us5 < rangeMax and us5 > 0 )
    { 
    inRange5 = true;
    cc3 = int(us5*3);
    }  
     
    
  int rnd = random(sizeof(melody)-1);
  int totalNotes = sizeof(melody)-1;
  
  //Serial.println(sizeof(melody)-1);
  if ( inRange1 )
    { thisNote = note; }
  else
    {
    thisNote = 0;
    //thisNote = rnd;
    }

  if ( thisNote > 100 )
    { thisNote = 100; }

  if ( (thisNote < 56) && (thisNote > 0) )
    { thisNote = 56; }


  if ( inRange3 ) 
    {
    if ( cc1 > 127 )
      { cc1 = 127; }
    if ( cc1 < 0 )
      { cc1 = 0; }

    cc1 = 127 - cc1; 
    }

  if ( inRange4 )
    {
    if ( cc2 > 127 )
      { cc2 = 127; }
    if ( cc2 < 0 )
      { cc2 = 0; }

    cc2 = 127 - cc2; 
    }

  if ( inRange5 )
    {
    if ( cc3 > 127 )
      { cc3 = 127; }
    if ( cc3 < 0 )
      { cc3 = 0; }

    cc3 = 127 - cc3; 
    }

    
  // ===================================================== ACTUAL MIDI NOTE STUFF

  if( (melody[thisNote]) && (thisNote != 0) && (ino == 0 ) && ( t1 >= (pbnti + pbndu)) ) 
    {  
    MIDI.sendNoteOn(melody[thisNote], velocity, midichan);
    //MIDI.sendNoteOn(59, 127, midichan);
    noti = t1;
    nodu = noteDuration;
    cn = thisNote;
    ino = true;
    lastNote = cn;

    pbnti = 0;
    pbndu = noteDuration * 0.75;
    }

  //delay(noteDuration);
  
  if ( ino )
    {
    if ( t1 >= (noti + nodu) )
      {
      if(melody[cn]) 
        {
        MIDI.sendNoteOff(melody[cn], velocity, midichan);
        noti = 0;
        nodu = 0;
        cn = 0;
        ino = false;
        pbnti = t1;
        pbndu = noteDuration * 0.75;
        }
      }  
    }

  int n = 0;
  if ( !inRange1 )  // && !inRange2 && !inRange3 && !inRange4 )
    {
    allnotesoff();
    }


  if ( inRange3 )
    {
    MIDI.sendControlChange(105, cc1, midichan);
    }

  if ( inRange4 )
    {
    MIDI.sendControlChange(108, cc2, midichan);
    }

  if ( inRange5 )
    {
    MIDI.sendControlChange(111, cc3, midichan);
    }
  
  // ===================================================== DISPLAY STUFF
  display.clearDisplay();
  display.setTextSize(1); // Draw 2X-scale text
    
  display.setTextColor(WHITE);
  
  display.setCursor(0, 0);
  display.print("N:");
  display.print(thisNote);
  display.print("/");
  display.print(velocity);
  display.print("/");
  display.print(ino);
  


  //display.setCursor(70, 14);
  //display.print("VEL: ");
  //display.print(velocity);


  display.setCursor(0, 6);
  display.print("D:");
  if ( noteDuration == beat_length/32 )
    { display.print("1/32"); }
  else if ( noteDuration == beat_length/16 )
    { display.print("1/16"); }
  else if ( noteDuration == round(beat_length/8) )
    { display.print("1/8"); }
  else if ( noteDuration == round(beat_length/4) )
    { display.print("1/4"); }
  else if ( noteDuration == round(beat_length/2) )
    { display.print("1/2"); }
  else if ( noteDuration == round(beat_length/1) )
    { display.print("1/1"); }
  else
    { display.print(noteDuration); }

    
  display.setCursor(0, 12);
  display.print("CC1:");
  display.print(cc1);

  display.setCursor(0, 18);
  display.print("CC2:");
  display.print(cc2);

  display.setCursor(0, 24);
  display.print("CC3:");
  display.print(cc3);



  display.setCursor(64, 0);
  display.print("U1:");
  if ( us1 < 10 ) { display.print(" "); }
  if ( us1 < 100) { display.print(" "); }
  display.print(us1, 1);
  if ( inRange1 == true )
    {
    display.print(" O");
    }

  display.setCursor(64, 6);
  display.print("U2:");
  if ( us2 < 10 ) { display.print(" "); }
  if ( us2 < 100) { display.print(" "); }
  display.print(us2, 1);
  if ( inRange2 == true )
    {
    display.print(" O");
    }
      
  display.setCursor(64, 12);
  display.print("U3:");
  if ( us3 < 10 ) { display.print(" "); }
  if ( us3 < 100) { display.print(" "); }
  display.print(us3, 1);
  if ( inRange3 == true )
    {
    display.print(" O");
    }

  display.setCursor(64, 18);
  display.print("U4:");
  if ( us4 < 10 ) { display.print(" "); }
  if ( us4 < 100) { display.print(" "); }
  display.print(us4, 1);
  if ( inRange4 == true )
    {
    display.print(" O");
    }

  display.setCursor(64, 24);
  display.print("U5:");
  if ( us5 < 10 ) { display.print(" "); }
  if ( us5 < 100) { display.print(" "); }
  display.print(us5, 1);
  if ( inRange5 == true )
    {
    display.print(" O");
    }

  
  //display.setCursor(70, 0);
  //display.print("CC1: ");
  //display.print(cc1);

  //display.setCursor(70, 7);
  //display.print("CC2: ");
  //display.print(cc2);

  //display.setCursor(70, 14);
  //display.print("CC3: ");
  //display.print(cc3);
  




  //display.setCursor(70, 24);
  //display.print("PBN: ");
  //display.print(pauseBetweenNotes);



  //display.setCursor(0, 28);
  //display.print("TIM: ");
  //display.print(t1, 10);


  //display.setCursor(70, 26);
  //display.print("INO: ");
  //display.print(ino, 10);

    
  display.display();      // Show initial text
    


  delay(i1);
  }
