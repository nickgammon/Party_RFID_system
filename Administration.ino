/*
RFID event / entry management system

Author: Nick Gammon
Date:   15 April 2015

 Copyright 2015 Nick Gammon.

 Administration stuff.
 
 * Enter admin mode
 * Enter add card mode
 * Enter delete card mode
 * Enter pass-out mode
 * Delete all cards (requires admin mode)
 * Start new event (requires admin mode)
 
 Starting a new event marks all users as outside the venue.
 
*/

void cancelAdminMode ()
  {
  if (state == STATE_ADMIN) 
    {
    allLEDsOff ();
    state = STATE_EVENT_ACTIVE;
    cancel ();
    Serial.println (F("Cancelling admin mode."));
    }
  }  // end of cancelAdminMode
  
void enterAdminMode ()
  {
  if (state == STATE_ADMIN)  // if already adding, cancel admin mode
    {
    cancelAdminMode ();
    return;
    }
    
  state = STATE_ADMIN;
  ack ();
  Serial.println (F("Admin mode."));
  }  // end of enterAdminMode
  
void addNewCardMode ()
  {
  if (state == STATE_ADD_CARD || state == STATE_ADD_BACKUP_CARD)  // if already adding, cancel add mode
    {
    state = STATE_EVENT_ACTIVE;
    cancel ();
    Serial.println (F("Cancelling add mode."));
    }
  else
    {
    state = STATE_ADD_CARD;
    ack ();
    Serial.println (F("Add mode."));
    }
  }  // end of addNewCardMode

void deleteCardMode ()
  {
  if (state == STATE_DELETE_CARD)  // if already deleting, cancel delete mode
    {
    state = STATE_EVENT_ACTIVE;
    cancel ();
    Serial.println (F("Cancelling delete mode."));
    }
  else
    {
    state = STATE_DELETE_CARD;
    deleteWarning ();
    Serial.println (F("Delete mode."));
    }
  }  // end of deleteCardMode

void passOutMode ()
  {
  if (state == STATE_PASS_OUT)  // if already in pass-out mode, cancel pass-out mode
    {
    allLEDsOff ();
    state = STATE_EVENT_ACTIVE;
    cancel ();
    Serial.println (F("Cancelling pass-out mode."));
    }
  else
    {
    state = STATE_PASS_OUT;
    passOutTune ();
    Serial.println (F("Pass-out mode."));
    }
  }  // end of passOutMode

void eraseAllCards ()
  {
  if (state != STATE_ADMIN)
    {
    Serial.println (F("Not in admin mode."));
    fail ();
    flashLED (LED_ERROR);  // need admin mode
    state = STATE_EVENT_ACTIVE;
    return;            
    }
     
  Serial.println (F("Erase all cards."));
  done ();
  int cards = 0;
  EEPROM_writeAnything (0, cards);
  cancelAdminMode ();
  flashAllLEDs ();    
  
#if USE_SD    
    snprintf (disk_buf, sizeof (disk_buf), "Erased all cards");
    writeToDisk (disk_buf);
#endif // USE_SD

  corrupt = false;
  }  // end of eraseAllCards

void beginEvent ()
  {
  if (state != STATE_ADMIN)
    {
    Serial.println (F("Not in admin mode."));
    fail ();
    flashLED (LED_ERROR);  // need admin mode
    state = STATE_EVENT_ACTIVE;
    return;            
    }
     
  Serial.println (F("Begin event."));
  int cards;
  EEPROM_readAnything (0, cards);  // find how many are in EEPROM
  
  // mark all users as outside the event
  int addr = FIRST_CARD;   // start past the counter
  for (int i = 0; i < cards; i++)
    {
    userCode thisEntry;
    EEPROM_readAnything (addr, thisEntry);  // read this one
    if (thisEntry.location != USER_UNKNOWN)
      {
      thisEntry.location = USER_OUTSIDE;
      EEPROM_writeAnything (addr, thisEntry);
      }
    addr += sizeof (thisEntry);
    }  // end of for loop
  
  done ();
  cancelAdminMode ();
  flashLED (LED_ACTION_ACCEPTED);  // OK
  
#if USE_SD    
    snprintf (disk_buf, sizeof (disk_buf), "Started new event");
    writeToDisk (disk_buf);
#endif // USE_SD  
  }  // end of beginEvent
