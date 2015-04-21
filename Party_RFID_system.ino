/*
RFID event / entry management system

Author: Nick Gammon
Date:   15 April 2015

 Copyright 2015 Nick Gammon.


 PERMISSION TO DISTRIBUTE

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense,
 and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be included in
 all copies or substantial portions of the Software.


 LIMITATION OF LIABILITY

 The software is provided "as is", without warranty of any kind, express or implied,
 including but not limited to the warranties of merchantability, fitness for a particular
 purpose and noninfringement. In no event shall the authors or copyright holders be liable
 for any claim, damages or other liability, whether in an action of contract,
 tort or otherwise, arising from, out of or in connection with the software
 or the use or other dealings in the software.
 
 CONFIGURATION:
 
 Copy Configuration.h.orig to be Configuration.h and then amend as required.
 
*/

const int NOT_USED = -1;

#include "Configuration.h"

#include <EEPROM.h>
#include <EEPROMAnything.h>
#include <Tone.h>
#include <memdebug.h>

#if USE_RTC
  #include <Wire.h>
  #include "RTClib.h"
#endif

#if USE_SD
  #include <SPI.h>
  #include <SdFat.h>
#endif

#if USE_SOFTWARE_SERIAL
  #include <ReceiveOnlySoftwareSerial.h>
#endif 

enum { STATE_EVENT_ACTIVE,
       STATE_ADMIN,
       STATE_ADD_CARD,
       STATE_ADD_BACKUP_CARD,
       STATE_PASS_OUT,
       STATE_DELETE_CARD,
       
       STATE_LAST_STATE,  // last state!
     } state = STATE_EVENT_ACTIVE;

enum { LED_ACTION_ACCEPTED = STATE_LAST_STATE,
       LED_ALREADY_THERE,
       LED_ERROR,
       
       LED_LAST_LED,   // last LED!
     };

typedef enum userState { 
       USER_UNKNOWN,
       USER_OUTSIDE, 
       USER_INSIDE,
  } userState;

const unsigned long TIMEOUT = 300;  // mS

// RFID reader
#if USE_SOFTWARE_SERIAL
  ReceiveOnlySoftwareSerial RFID_reader (RFID_PIN);
#endif

// where the first card info starts in EEPROM
const int FIRST_CARD = sizeof (int);

// the most recent card, zero if none
unsigned long cardCode;

// these will be zero with no clock
unsigned int year;
byte month, day, hour, minute, second;

// number of items in an array
#define ARRAY_SIZE(x) (sizeof(x) / sizeof((x)[0]))

int flashesToGo [ARRAY_SIZE (LED_pins)];

// card information in EEPROM
typedef struct {
    unsigned long mainCard;
    unsigned long backupCard;
    byte location;
}  userCode;

// for beeps
Tone tone1;

// when we unlocked the door, so we know when to lock it again
unsigned long timeDoorUnlocked;

// true if we detect corrupt memory
bool corrupt;

#define MAX_ID 13  // 12 bytes plus null-terminator
char id12_buffer [MAX_ID] = "";

#if USE_RTC
  // clock chip class
  RTC_DS1307 RTC;
#endif  // USE_RTC

#if USE_SD
  char disk_buf [65];
#endif  // USE_SD

// for date/time info
char timeBuf [30] = "";

void showLocation (const byte location)
  {
  switch (location)
    {
     case USER_UNKNOWN: Serial.print (F("Deleted")); break;
     case USER_OUTSIDE: Serial.print (F("Outside")); break;
     case USER_INSIDE:  Serial.print (F("Inside")); break;
     default: 
         Serial.print (F("Bad status")); 
         corrupt = true;
         break;
    }  // end of switch
    
  }  // end of showLocation
  
void listAllCards ()
  {
  int cards;
  EEPROM_readAnything (0, cards);  // find how many are in EEPROM
  Serial.println (F("-----------------"));
  Serial.println (F("LISTING ALL CARDS"));
  
  if (cards < 0)
    {
    corrupt = true;
    Serial.println (F("EEPROM not initialized."));
    return;
    }  // end of unintialized EEPROM
    
  Serial.print (F("Number on file: "));
  Serial.println (cards);
  
  int addr = FIRST_CARD;   // start past the counter
  for (int i = 0; i < cards; i++)
    {
    userCode thisEntry;
    EEPROM_readAnything (addr, thisEntry);  // read this one
    Serial.print (i);
    Serial.print (F(". "));
    Serial.print (thisEntry.mainCard, HEX);
    Serial.print (F(" / "));
    Serial.print (thisEntry.backupCard, HEX);
    Serial.print (F(", status: "));
    showLocation (thisEntry.location);
    Serial.println ();
    addr += sizeof (thisEntry);
    }  // end of for loop
    
  Serial.print (F("Free memory: "));
  Serial.println (getFreeMemory ());
  }  // end of listAllCards
  
void setup()
  {
#if USE_SOFTWARE_SERIAL  
  RFID_reader.begin (RFID_BAUD_RATE); // for the separate RFID reader
  Serial.begin (MAIN_BAUD_RATE);      // for debugging via USB
#else
  Serial.begin (RFID_BAUD_RATE);      // disconnect RFID reader to upload sketches
#endif 

  Serial.println (F("Start door entry system."));
 
  if (SPEAKER != NOT_USED)
    tone1.begin(SPEAKER);

#if USE_RTC
  // clock uses I2C
  Wire.begin();

  // activate clock
  RTC.begin();
  
  // set time in clock chip if not set before
  if (! RTC.isrunning() || ADJUST_TIME)
    {
    // following line sets the RTC to the date & time this sketch was compiled
    RTC.adjust(DateTime(__DATE__, __TIME__));
    }

#if USE_SD
  // Attach callback function to provide date time to files.
  SdFile::dateTimeCallback (fileDateTime);
#endif // USE_SD

#endif 

  for (int i = 0; i < ARRAY_SIZE (LED_pins); i++)
    if (LED_pins [i] != NOT_USED)
      pinMode (LED_pins [i], OUTPUT);

  if (DOORLOCK != NOT_USED)
    {  
    pinMode (DOORLOCK, OUTPUT);
    digitalWrite (DOORLOCK, LOW);   // lock door initially
    }  // if have a door lock
    
  listAllCards ();    // debugging
  flashAllLEDs ();
    
  getTime ();

#if USE_SD    
    snprintf (disk_buf, sizeof (disk_buf), "Rebooted");
    writeToDisk (disk_buf);
#endif // USE_SD  

  ack ();
  }  // end of setup

byte findCard (int * whereFound = NULL);  // prototype <sigh>
byte findCard (int * whereFound)
  {
  int cards;
  EEPROM_readAnything (0, cards);  // find how many are in EEPROM
  
  if (cards < 0)
    return USER_UNKNOWN;
  
  int addr = FIRST_CARD;   // start past the counter
  for (int i = 0; i < cards; i++)
    {
    userCode thisEntry;
    EEPROM_readAnything (addr, thisEntry);  // read this one
    if (((thisEntry.mainCard == cardCode) || (thisEntry.backupCard == cardCode))
        && thisEntry.location != USER_UNKNOWN)
      {
      if (whereFound)
        *whereFound = addr;
      return thisEntry.location;  // found!
      }
    addr += sizeof (thisEntry);
    }  // end of for loop
  Serial.println (F("Not found."));
  if (whereFound)
    *whereFound = 0;
  return USER_UNKNOWN;  // not there
  }  // end of findCard
  

void updateState (const byte newLocation);
void updateState (const byte newLocation)
  {
  Serial.print (F("Updating location to "));
  showLocation (newLocation);
  Serial.println ();
  
  int addr;
  findCard (&addr);
  if (addr == 0)
    return;  // not found

  userCode thisEntry ;
  EEPROM_readAnything (addr, thisEntry);  // read this one back in
  thisEntry.location = newLocation;
  EEPROM_writeAnything (addr, thisEntry);
  Serial.println (F("Done."));
  }  // end of updateState

void passOutCard ()
  {
  byte location = findCard ();
  if (location == USER_UNKNOWN)
    {
    Serial.println (F("Not on file."));
    fail ();
    flashLED (LED_ERROR);  // not there
    return;
    }

  updateState (USER_OUTSIDE);

  Serial.println (F("User now outside."));

  ack ();
  flashLED (LED_ACTION_ACCEPTED);  // done
     
#if USE_SD    
    snprintf (disk_buf, sizeof (disk_buf), "Pass-out: %lX %s", cardCode, timeBuf);
    writeToDisk (disk_buf);
#endif // USE_SD
     
  }  // end of passOutCard

void checkCard ()
  {
  byte location = findCard ();
  Serial.println (F("Checking card ..."));
  
  if (location == USER_INSIDE)
    {
    Serial.println (F("Already there."));
    alreadyThere ();
    flashLED (LED_ALREADY_THERE);  // is already there
    return;
    }
  else if (location == USER_OUTSIDE)
    {
    Serial.println (F("Now inside."));
    if (DOORLOCK != NOT_USED)
      {
      timeDoorUnlocked = millis ();
      digitalWrite (DOORLOCK, HIGH);   // unlock door
      }
    nowInside ();
      
    flashLED (LED_ACTION_ACCEPTED);  // is there

#if USE_SD    
    snprintf (disk_buf, sizeof (disk_buf), "Opened: %lX %s", cardCode, timeBuf);
    writeToDisk (disk_buf);
#endif // USE_SD

#if ONE_ENTRY_ONLY    
    updateState (USER_INSIDE);
#endif  // ONE_ENTRY_ONLY

    return;
    }
    
  Serial.println (F("User not known."));
  fail ();
  flashLED (LED_ERROR);  // not there

#if USE_SD    
    snprintf (disk_buf, sizeof (disk_buf), "Unknown: %lX %s", cardCode, timeBuf);
    writeToDisk (disk_buf);
#endif // USE_SD
    
  } // end of checkCard
  

  
void processNewCard ()
  {
  getTime ();  // this saves the time in global variables

  Serial.print (F("Got card "));
  Serial.println (cardCode, HEX); 
  
  switch (cardCode)
    {
    case MASTER_ADMIN_CARD: 
         enterAdminMode ();
         break;

    case MASTER_ADD_CODE_CARD: 
         addNewCardMode ();
         break;
         
    case MASTER_DELETE_CODE_CARD:
         deleteCardMode ();
         break;
 
    case MASTER_PASS_OUT_CARD:
         passOutMode ();
         break;
         
    case MASTER_ERASE_ALL_CARD: 
         eraseAllCards ();
         break;
         
    case MASTER_BEGIN_EVENT_CARD:
         beginEvent ();
         break;
         
    default:
         cancelAdminMode ();
         switch (state)
           {
           case STATE_ADD_CARD:  
                 addCard ();
                 break;
           
           case STATE_ADD_BACKUP_CARD:
                 addBackupCard ();
                 state = STATE_EVENT_ACTIVE;
                 break;
                 
           case STATE_DELETE_CARD:  
                 deleteCard ();
                 state = STATE_EVENT_ACTIVE;
                 break;

           case STATE_PASS_OUT:  
                 passOutCard ();
                 state = STATE_EVENT_ACTIVE;
                 break;

           default: 
                 checkCard (); 
                 break;
             
           }  // end of switch on current state
         break;
    }  // end of switch
    
  listAllCards ();  // debugging
  }  // end of processNewCard
  

void loop()
  {
  // if serial data available, process it
#if USE_SOFTWARE_SERIAL  
  while (RFID_reader.available () > 0)
    processIncomingByte (RFID_reader.read ());
#else
  while (Serial.available () > 0)
    processIncomingByte (Serial.read ());
#endif

  manageLEDs ();

  // do a warning beep to remind them to present a card
  static unsigned long lastBeep;
  if (state == STATE_ADD_CARD || 
     state == STATE_ADD_BACKUP_CARD ||
     state == STATE_DELETE_CARD ||
     state == STATE_PASS_OUT)
    {
    if (millis () - lastBeep >= TIME_BETWEEN_WARNINGS)  
      {
      waiting ();
      // two beeps means we want the main card
      if (state == STATE_ADD_CARD)
        {
        delay (200);
        waiting ();
        }
      lastBeep = millis ();
      }  // end if
    }  // end of waiting for backup card

  // relock door if time up    
  if (DOORLOCK != NOT_USED && millis () - timeDoorUnlocked >= UNLOCK_TIME)
    digitalWrite (DOORLOCK, LOW);   // lock door again

  
  // do a warning beep if EEPROM corrupt
  static unsigned long lastWarning;
  if (corrupt)
    {
    if (millis () - lastWarning >= TIME_BETWEEN_WARNINGS)  
      {
      corruptWarning ();
      lastWarning = millis ();
      }  // end if
    }  // end of waiting for backup card

  }  // end of loop
