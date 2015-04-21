/*
RFID event / entry management system

Author: Nick Gammon
Date:   15 April 2015

 Copyright 2015 Nick Gammon.

 Disk file management. 
 
 Handles writing information to the SD card.
 
*/

#if USE_SD

#if USE_RTC
//This function returns the date and time for SD card file access and modify time.
void fileDateTime (uint16_t* date, uint16_t* time)
  {
  *date=FAT_DATE(year, month, day);
  *time=FAT_TIME(hour, minute, second);
  }  // end of fileDateTime
#endif // #if USE_RTC


void writeToDisk (const char * what)
  {

  // file system object
  SdFat sd;
  if (sd.begin (SD_CHIP_SELECT, SPI_HALF_SPEED))
    {
    // Log file.
    SdFile file;

    SPI.begin ();
    char fileName [30];
    snprintf (fileName, sizeof (disk_buf), "Door%i.txt", year);

    if (file.open (fileName,  O_CREAT | O_WRITE | O_APPEND))
      {
      file.println (what);
      file.sync ();
      file.close ();
      }    // end of opened file OK
    else
      {
      badDisk ();
      Serial.println (F("Failed to open disk file."));
      }
      
    delay (100);  // enough?

    Serial.print (F("Free memory: "));
    Serial.println (getFreeMemory ());

    SPI.end ();
   
    }  // end opened SD card OK
  else
    {
    badDisk ();
    Serial.println (F("Failed to activate SD card."));
    }
    
  }    // end of writeToDisk
  
  
#endif // USE_SD
