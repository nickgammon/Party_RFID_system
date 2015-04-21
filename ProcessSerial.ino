/*
RFID event / entry management system

Author: Nick Gammon
Date:   15 April 2015

 Copyright 2015 Nick Gammon.

 Process incoming serial data from the RFID reader.
 
*/

unsigned long timeLastSerialInput;
byte count;

void processIncomingByte (const byte inByte)
  {
  unsigned long now = millis ();
  
  // if a long time has elapsed, assume new card
  if (now - timeLastSerialInput >= TIMEOUT)
    {
    cardCode = 0;
    count = 0; 
    }  // end of timeout
  
  timeLastSerialInput = now;
 
#if USE_ID12

   switch (inByte)
    {
    case 2:
      count = 0;
      break;

    case 3:
      id12_buffer [count] = 0;
      id12_buffer [10] = 0;  // chop off sumcheck
      
      // skip manufacturer code, convert 8 bytes to cardCode
      char * endptr;
      cardCode = strtoul (&id12_buffer [2], &endptr, 16);
      // should have had 12 bytes from the card, hex all the way to the end, 
      // and it should convert to hex OK
      if (count != 12 || endptr != &id12_buffer [10] || cardCode == 0)
         {
         fail ();
         flashLED (LED_ERROR);  // bad card
         }
      else 
        processNewCard ();
        
      count = 0;
      cardCode = 0;
      break;

    case 10:
    case 13:
      break;

    default:
      if (count >= (MAX_ID - 1))
        break;

      id12_buffer [count++] = inByte;
      break;

    }  // end of switch

#else
  cardCode <<= 8;
  cardCode |= inByte;
  count++;
  
  if (count >= 4)
    {
    processNewCard ();
    cardCode = 0;
    count = 0; 
    }  // end of having 4 incoming bytes
#endif   // USE_ID12

  }  // end of processIncomingByte

