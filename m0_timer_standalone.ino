// Sat 23 Jun 17:15:05 UTC 2018
// 0105-a0a-00-

// Fri Jun 30 20:18:10 UTC 2017
// 0103-a0a-00-

#include <Arduino.h>
#include "neo_pixel.h"

/******************************************************************************/
/**  The Arduino M0 pro has 0x4000 as bootloader offset                      **/
/******************************************************************************/

/* [ http://forum.arduino.cc/index.php?topic=332275.17 ] */

int scale = 1;
int buzzPin = 13 ; // A5


uint8_t sound = 1; // 0 = off  1 = on

void tickOff(void) { } 
void tickOn(void) { } 
void tick(void) { }

void darkenfive(void) { // turn off buzzPin
  digitalWrite(buzzPin, 0); // turn off
}

void darkenlight(void) {
  digitalWrite(13, 0); // turn off
  delay(2000);
}

void brightenfive(void) { // turn on pin five
  digitalWrite(buzzPin, 1); // turn on
}

void brightenlight(void) {
  digitalWrite(13, 1); // turn on
  delay(220);
}

void blinkenlight(void) {
  brightenlight();
  darkenlight();
}

void introduction() {
    blinkenlight(); Serial.print(".");
}


void setup() {
  Serial.begin(38400);     // Open serial communications:

  Serial.print("\033\133"); // ESC [
  Serial.print("\063\063"); // 33 - yellow fg
  Serial.print("m");        // for the stanza

  // LED PA_17 - mapped by Arduino as digital 13

  pinMode(buzzPin, 1); // OUTPUT
  pinMode(13, 1); // OUTPUT

  darkenlight();  // not needed - insurance.
  blinkenlight(); delay(44);
  darkenlight(); // turn off so to notice the gap
  delay(234);


  REG_GCLK_GENDIV = GCLK_GENDIV_DIV(0xfd) |       // Divide the 48MHz clock source by divisor 3: 48MHz/3=16MHz
                    GCLK_GENDIV_ID(4);            // Select Generic Clock (GCLK) 4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  REG_GCLK_GENCTRL = GCLK_GENCTRL_IDC |           // Set the duty cycle to 50/50 HIGH/LOW
                     GCLK_GENCTRL_GENEN |         // Enable GCLK4
                     GCLK_GENCTRL_SRC_DFLL48M |   // Set the 48MHz clock source
                     GCLK_GENCTRL_ID(4);          // Select GCLK4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |         // Enable GCLK4 to TC4 and TC5
                     GCLK_CLKCTRL_GEN_GCLK4 |     // Select GCLK4
                     GCLK_CLKCTRL_ID_TC4_TC5;     // Feed the GCLK4 to TC4 and TC5
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization
 
  REG_TC4_CTRLA |= TC_CTRLA_MODE_COUNT8;          // Set the counter to 8-bit mode
  while (TC4->COUNT8.STATUS.bit.SYNCBUSY);        // Wait for synchronization

  REG_TC4_COUNT8_CC0 = 0x02;                      // TC4 CC0 (arbitrary value)

  while (TC4->COUNT8.STATUS.bit.SYNCBUSY);        // Wait for synchronization
  REG_TC4_COUNT8_CC1 = 0x02;                      // TC4 CC1 (arbitrary value)

  while (TC4->COUNT8.STATUS.bit.SYNCBUSY);        // Wait for synchronization
  REG_TC4_COUNT8_PER = 0xec;                      // Set the PER (period) register to its maximum value
  while (TC4->COUNT8.STATUS.bit.SYNCBUSY);        // Wait for synchronization

  NVIC_SetPriority(TC4_IRQn, 0);    // Set the Nested Vector Interrupt Controller (NVIC) priority for TC4 to 0 (highest)
  NVIC_EnableIRQ(TC4_IRQn);         // Connect TC4 to Nested Vector Interrupt Controller (NVIC)

  REG_TC4_INTFLAG |= TC_INTFLAG_MC1 | TC_INTFLAG_MC0 | TC_INTFLAG_OVF;        // Clear the interrupt flags
  REG_TC4_INTENSET = TC_INTENSET_MC1 | TC_INTENSET_MC0 | TC_INTENSET_OVF;     // Enable TC4 interrupts
 
  // 1024 256 64 16:

      REG_TC4_CTRLA |= TC_CTRLA_PRESCALER_DIV1024 |   // Set prescaler to 1024

  //  REG_TC4_CTRLA |= TC_CTRLA_PRESCALER_DIV256  |   // Set prescaler to  256

  //  REG_TC4_CTRLA |= TC_CTRLA_PRESCALER_DIV64   |   // Set prescaler to   64, 16MHz/64 = 256kHz

  //  REG_TC4_CTRLA |= TC_CTRLA_PRESCALER_DIV16   |   // Set prescaler to   16
                       TC_CTRLA_ENABLE;               // Enable TC4

  while (TC4->COUNT8.STATUS.bit.SYNCBUSY);            // Wait for synchronization

  // introduction();

}



void led13Off(void) {
  digitalWrite(13, 0); // turn off
}


void led13On(void) {
  digitalWrite(13, 1); // turn on
}



// 1: scale  1, then 2, then -1: darken.
// 2: scale -1, then 0:          brighten.
// 3: scale  0, then 1, then -1: darken.
// 4: scale -1, then 0:          brighten.
void pinToggle(void) {
  scale = scale + 1 ;
  if (scale > 0) {
    scale = -1 ;    // reset
    darkenfive();   // turn off buzzPin
  } else {
    brightenfive(); // turn on buzzPin
  }
}


void loop() {
    Serial.print("."); // tell the serial port we're here
}

void TC4_Handler()             // Interrupt Service Routine (ISR) for timer TC4
{     
  if (TC4->COUNT8.INTFLAG.bit.OVF && TC4->COUNT8.INTENSET.bit.OVF)             
  {
    REG_TC4_INTFLAG = TC_INTFLAG_OVF;      // Clear the OVF interrupt flag
  }

  if (TC4->COUNT8.INTFLAG.bit.MC0 && TC4->COUNT8.INTENSET.bit.MC0)             
  {
   pinToggle();
   REG_TC4_INTFLAG = TC_INTFLAG_MC0;      // Clear the MC0 interrupt flag
  }

  if (TC4->COUNT8.INTFLAG.bit.MC1 && TC4->COUNT8.INTENSET.bit.MC1)           
  {
   REG_TC4_INTFLAG = TC_INTFLAG_MC1;      // Clear the MC1 interrupt flag
  }
}



// end.
