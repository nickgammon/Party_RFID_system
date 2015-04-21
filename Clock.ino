/*
RFID event / entry management system

Author: Nick Gammon
Date:   15 April 2015

 Copyright 2015 Nick Gammon.

 Stuff to do with the RTC (real-time clock)
 
*/

void getTime ()
  {
#if USE_RTC

  // find the time
  DateTime now = RTC.now();

  // copy to global variables for SD card
  year = now.year ();
  month = now.month ();
  day = now.day ();
  hour = now.hour ();
  minute = now.minute ();
  second = now.second ();

  snprintf (timeBuf, sizeof (timeBuf), " at: %02i:%02i:%02i on %02i/%02i/%04i",
           now.hour (), now.minute (), now.second (),
           now.day (), now.month (), now.year ());
           
  Serial.print (F("Time is "));
  Serial.println (timeBuf);
#endif // USE_RTC
  }  // end of getTime
  

