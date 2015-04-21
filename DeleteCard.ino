/*
RFID event / entry management system

Author: Nick Gammon
Date:   15 April 2015

 Copyright 2015 Nick Gammon.

 Delete an existing card. Marks that slot in EEPROM as free.
 
*/
  
void deleteCard ()
  {
  int addr;
  byte location = findCard (&addr);
  if (location == USER_UNKNOWN)
    {
    Serial.println (F("Not on file."));
    fail ();
    flashLED (LED_ERROR);  // not there
    return;
    }

  // find both card codes, not just the one presented
  userCode thisEntry ;
  EEPROM_readAnything (addr, thisEntry);  // read this one back in

  updateState (USER_UNKNOWN);

  Serial.println (F("Deleting card."));

  ack ();
  flashLED (LED_ACTION_ACCEPTED);  // done
  
#if USE_SD    
    snprintf (disk_buf, sizeof (disk_buf), "Deleted cards: %lX, %lX %s", thisEntry.mainCard, thisEntry.backupCard, timeBuf);
    writeToDisk (disk_buf);
#endif // USE_SD  

  }  // end of deleteCard
  
