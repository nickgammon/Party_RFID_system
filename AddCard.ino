/*
RFID event / entry management system

Author: Nick Gammon
Date:   15 April 2015

 Copyright 2015 Nick Gammon.

 Code to manage adding a new card.
 
 We need a main card and a backup card. The backup card is retained by 
 the administrator in case the main card is lost. It can then be used
 to delete the main card.
 
*/

userCode newEntry;
    
void addCard ()
  {
  byte location = findCard ();
  if (location != USER_UNKNOWN)
    {
    Serial.println (F("Already there."));
    fail ();
    flashLED (LED_ALREADY_THERE);  // already there
    return;
    }

  newEntry.mainCard = cardCode;
  newEntry.location = USER_OUTSIDE; 
  state = STATE_ADD_BACKUP_CARD;
  ack ();
  
  Serial.println (F("Waiting for backup card to be presented."));
  
  }  // end of addCard
  
void addBackupCard ()
  {
  byte location = findCard ();
  if (location != USER_UNKNOWN)
    {
    Serial.println (F("Already there."));
    fail ();
    flashLED (LED_ALREADY_THERE);  // already there
    return;
    }

  if (cardCode == newEntry.mainCard)
    {
    Serial.println (F("Backup card same as main card."));
    fail ();
    flashLED (LED_ALREADY_THERE);  // already there
    flashLED (LED_ERROR);          // same as main
    return;
    }
    
  newEntry.backupCard = cardCode;

  int cards;
  EEPROM_readAnything (0, cards);  // find how many are in EEPROM

  // this will either find an empty slot, or end up at the end of the last used location
  int addr = FIRST_CARD;
  int i;
  for (i = 0; i < cards; i++)
    {
    userCode thisEntry;
    EEPROM_readAnything (addr, thisEntry);  // read this one
    if (thisEntry.location == USER_UNKNOWN)
      break;
    else
      addr += sizeof (thisEntry);
    }  // end of for loop
    
  // if not found, we will be adding one more card to the end of the list
  if (i >= cards)
    cards++;
  
  if (addr + sizeof (newEntry) > (E2END + 1))
    {
    Serial.println (F("EEPROM full."));
    fail ();
    flashLED (LED_ERROR);  // EEPROM full
    return;
    }

  Serial.print (F("Adding cards to address "));
  Serial.println (addr);
  
  // add card to list
  EEPROM_writeAnything (addr, newEntry);
  // update counter in EEPROM
  EEPROM_writeAnything (0, cards);
  ack ();
  flashLED (LED_ACTION_ACCEPTED);  // done
  
#if USE_SD    
    snprintf (disk_buf, sizeof (disk_buf), "Added cards: %lX, %lX %s", newEntry.mainCard, newEntry.backupCard, timeBuf);
    writeToDisk (disk_buf);
#endif // USE_SD  
  }  // end of addBackupCard
