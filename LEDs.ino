/*
RFID event / entry management system

Author: Nick Gammon
Date:   15 April 2015

 Copyright 2015 Nick Gammon.

 LED management.
 
*/

void flashLED (const byte which)
  {
  flashesToGo [which] = NUMBER_OF_FLASHES;
  }  // end of flashLED

void flashAllLEDs ()
  {
  for (int i = 0; i < 2; i++)
    {
    for (int j = 0; j < ARRAY_SIZE (LED_pins); j++)
      if (LED_pins [j] != NOT_USED)
        digitalWrite (LED_pins [j], HIGH);
    delay (200);
    for (int j = 0; j < ARRAY_SIZE (LED_pins); j++)
      if (LED_pins [j] != NOT_USED)
        digitalWrite (LED_pins [j], LOW);
    delay (200);
    }  // end of for
  }  // end of flashAllLEDs

void allLEDsOff ()
  {
  for (int j = 0; j < ARRAY_SIZE (LED_pins); j++)
    if (LED_pins [j] != NOT_USED)
      digitalWrite (LED_pins [j], LOW);
  }  // end of allLEDsOff

  
// LED management
void manageLEDs ()
  {
static unsigned long lastFlashTime;

  // turn off all "state" LEDs
  for (int i = STATE_EVENT_ACTIVE; i < STATE_LAST_STATE; i++)
    if (LED_pins [i] != NOT_USED)
      digitalWrite (LED_pins [i], LOW);

  // turn on LED corresponding to current state
  if (LED_pins [state] != NOT_USED)
    digitalWrite (LED_pins [state], HIGH);
  
  unsigned long now = millis ();
  
  // if time is up, toggle appropriate LEDs
  if (now - lastFlashTime >= FLASH_TIME)
    {
    for (byte i = LED_ACTION_ACCEPTED; i < LED_LAST_LED; i++)
      {
      if (LED_pins [i] != NOT_USED)
        {
        if (digitalRead (LED_pins [i]) == HIGH)
          {
          digitalWrite (LED_pins [i], LOW);
          flashesToGo [i]--; 
          }
        else        
          {
          if (flashesToGo [i] > 0)
            digitalWrite (LED_pins [i], HIGH);
          }
        }
      }  // end of for
    lastFlashTime = now;
    }  // end of if time to do it 
          
  }  // end of manageLEDs
  
