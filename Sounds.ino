/*
RFID event / entry management system

Author: Nick Gammon
Date:   15 April 2015

 Copyright 2015 Nick Gammon.

Sound management (if speaker configured).
 
*/

// generic acknowledgement beep
void ack ()
  {
  if (SPEAKER == NOT_USED)
    return;
    
  tone1.play(NOTE_C7, 100);  
  }  // end of ack
  
void waiting ()
  {
  if (SPEAKER == NOT_USED)
    return;

  tone1.play(NOTE_A6, 20);  
  }  // end of waiting

// generic cancel mode beep
void cancel ()
  {
  if (SPEAKER == NOT_USED)
    return;
  
  tone1.play(NOTE_C7, 100);
  delay (200);
  tone1.play(NOTE_B6, 100);
  }  // end of cancel

// generic failure beep
void fail ()
  {
  if (SPEAKER == NOT_USED)
    return;

  tone1.play(NOTE_E2, 200);
  delay (300);
  tone1.play(NOTE_DS2, 200);
  state = STATE_EVENT_ACTIVE;
  }   // end of fail
  
void done ()
  {
  if (SPEAKER == NOT_USED)
    return;

  for (int i = 0; i < 3; i++)
    {
    tone1.play(NOTE_G6, 50);
    delay (100);
    tone1.play(NOTE_E6, 50);
    delay (100);
    tone1.play(NOTE_C7, 50);
    delay (100);
    }
  }  // end of done
  
void alreadyThere ()
  {
  if (SPEAKER == NOT_USED)
    return;

  for (int i = 0; i < 3; i++)
    {
    tone1.play(NOTE_G7, 50);
    delay (100);
    tone1.play(NOTE_E7, 50);
    delay (100);
    }
  }  // end of alreadyThere
  
void nowInside ()
  {
  if (SPEAKER == NOT_USED)
    return;

  tone1.play(NOTE_E5, 200);
  delay (300);
  tone1.play(NOTE_E5, 200);
  }  // end of nowInside
  
void passOutTune ()
  {
  if (SPEAKER == NOT_USED)
    return;

  tone1.play(NOTE_C6, 100);
  delay (200);
  tone1.play(NOTE_E6, 100);
  delay (200);
  tone1.play(NOTE_G6, 100);
  delay (200);
  }  // end of passOutTune
  
void deleteWarning ()
  {
  if (SPEAKER == NOT_USED)
    return;

  tone1.play(NOTE_C6, 100);
  delay (200);
  tone1.play(NOTE_C6, 100);
  delay (200);
  tone1.play(NOTE_C6, 100);
  delay (200);
  tone1.play(NOTE_F5, 100);
  delay (200);
  }  // end of deleteWarning

void corruptWarning ()
  {
  if (SPEAKER == NOT_USED)
    return;

  tone1.play(NOTE_A4, 20);  
  }  // end of corruptWarning
  
void badDisk ()
  {
  if (SPEAKER == NOT_USED)
    return;

  tone1.play(NOTE_E2, 200);
  delay (300);
  tone1.play(NOTE_DS2, 200);
  delay (300);
  tone1.play(NOTE_D2, 200);
    
  }  // end of badDisk
