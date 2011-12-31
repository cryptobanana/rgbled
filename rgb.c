/*
RGB Wave
Firmware
for use with ATtiny25
AS220
Mitch Altman
09-Mar-09 -- RBG instead of RGB
21-Dec-11 -- hacked for Dan's RGB LED modules

Distributed under Creative Commons 3.0 -- Attib & Share Alike
*/

/*
A very brief tutorial about regsiters:
----------------------------------------------------------------------
Microcontrollers have some special memory locations that are called "registers".
Registers control the hardware in the microcontroller.  For example, the DDRB register
for the ATtiny25 controls which of the pins on the microcontroller are inputs and outputs,
and the TCCR0B register controls some aspects of how the internal hardware Timer 0 will
function.  For the ATtiny25, registers are all 8-bits.

In the datasheet for the AVR microcontrollers, Atmel (who makes them) uses a somewhat
confusing convention for describing bits in an 8-bit register.  For example, TCCR0B is
the Timer/Counter Control Register B for Timer 0 (there is also a Register A, and there
is also a TCCR1 register, but since there is only one register for Timer 1, there is no
A and B).  Within TCCR0B there are 8 bits, defined like this:
   FOC0A   FOC0B   --   --   WGM02   CS02   CS01   CS00
There are three bits that start with "CS0":  CS02, CS01, CS00.  To refer to all three
of these in the same sentence, Atmel will refer to them as "CS02:0", which means CS0
bit 2 through CS0 bit 0.  Pretty messed up, but that's what we need to live with.
(And, by the way, these bits are "Clock Select" bits, which tell the microcontroller
how to select the clock, which is the heart-beat that controls the speed at which the
Timer 0 runs.)
The dashes mean that bit 4 and bit 5 are not used.
I won't define the other bits here.

You will see the above definitions in the firmware and comments, below.  For example,
in the comments in the main program, under the statement where we are setting up the
TCCR0B register for Timer 0, you will see:
     // CS02:00=001 for divide by 1 prescaler (this starts Timer0)
This means that CS0 bit 2 is 0, CS0 bit 1 is 0, and CS0 bit 0 is 1 (or stated another
way, the three CS0 bits are set to 001).

The datasheet describes in great depth all of the registers and functionality of the
ATtiny25 microcontroller.  I downloaded my copy of the datasheet from here:
http://www.atmel.com/dyn/resources/prod_documents/doc2586.pdf
But if that link is dead by the time you read this, just do a search for
     atmel attiny25 datasheet
and you'll come up with the latest version.
*/

/*
Parts list for this RGB Light project:
1   ATtiny25
1   CR2032 coin-cell battery
1   CR2032 battery holder
1   small slide switch (for on-off)
1   RGB LED (common anode)
2   47 ohm resistors
1   1k ohm resistor
1   0.1 uF capacitor
1   100 uF capacitor, 6v
1   6-pin header (if you want to re-program the ATtiny25)
*/


/*
The hardware for this project is very simple:
    ATtiny25 has 8 pins:
       pin 1  RST - connects to programming port pin 5
       pin 2  PB3 - connects to the output of the IR detector (pin 1 of the detector)
       pin 3  OC1B - Blue lead of RGB LED 
	   			(anode -- through a 47 ohm current limiting resistor)
       pin 4  ground 
       pin 5  OC0A -  Green lead of RGB LED 
	   			(anode -- also connects to programming port pin 4)
       pin 6  OC1A - Red lead of RGB LED 
	   			(anode -- through a 47 ohm current limiting resistor) (also connects to programming port pin 1)
       pin 7  PB2 - programming port pin 3
       pin 8  +3v 

	The 6-pin programming header needs to have its pin 2 connected to +3v, and
	its pin 6 connected to ground.

    This firmware requires that the clock frequency of the ATtiny
       is the default that it is shipped with:  8.0MHz
*/


#include <avr/io.h>             // this contains all the IO port definitions
#include <avr/interrupt.h>      // definitions for interrupts
#include <avr/sleep.h>          // definitions for power-down modes
#include <avr/pgmspace.h>       // definitions or keeping constants in program memory



/*
The following Light Table consists of any number of rgbElements that will fit into the
2k flash ROM of the ATtiny25 microcontroller.
NOTE:  I measured the time, and each fadeTime and holdTime is actually
          550 microseconds
       instead of 400 microseconds, as calculated.

  The Light Sequences and the notions of fadeTime and holdTime
  are taken from Pete Griffiths, downloaded from:
  http://www.petesworld.demon.co.uk/homebrew/PIC/rgb/index.htm

  I modified it to fit my purposes.

  The sequence takes about 2 minutes.
  More precisely:
       adding all of the fadeTime values together:  121,000
       adding all of the holdTime values together:  134,000
       adding these together = 259,000.
  Since the time values are each 400 microseconds, 255,000 is 102.0 seconds,
    or, 1.70 minutes, which is 1 minute, 42 seconds.

  The Main function repeats the sequence several times.
*/


struct rgbElement {
  int fadeTime;       
  int holdTime;       
  unsigned char red;  
  unsigned char green;
  unsigned char blue; 
} const lightTab[] PROGMEM = {
  {     0,    500,   0,   0,   0 },
  {   500,    500, 255,   0,   0 },
  {   500,    500,   0, 255,   0 },
  {   500,    500,   0,   0, 255 },
  {   500,    500,   0, 255, 255 },
  {   500,    500, 255,   0, 255 },
  {   500,    500, 255, 255,   0 },
  {   500,   2500, 255, 255, 255 },
  {  7000,   2500, 255,   0,   0 },
  {  7000,   2500,   0, 255,   0 },
  {  7000,   2500,   0,   0, 255 },
  {  7000,   2500, 155,  64,   0 },
  {  7000,   2500,  64, 255,  64 },
  {  7000,   2500,   0,  64, 255 },
  {  7000,   2500,  64,   0,  64 },
  {  7000,   1500, 155,   0,   0 },
  {  7000,   1500,   0, 255,   0 },
  {  7000,   1500,   0,   0, 255 },
  {  7000,   1500, 140,   0, 240 },
  {  7000,   1500, 155, 155,   0 },
  {  7000,   1500, 155, 255, 255 },
  {  7000,   1500, 128, 128, 128 },
  {  7000,   1500,  48,  48,  58 },
  {  7000,   1500,   0,   0,   0 },
  {  2500,   2500, 155,   0,   0 },
  {  2500,   2500, 155, 255,   0 },
  {  2500,   2500,   0, 255,   0 },
  {  2500,   2500,   0, 255, 255 },
  {  2500,   2500,   0,   0, 255 },
  {  2500,   2500, 155,   0, 255 },
  {  2500,      0,   0,   0,   0 },
  {  2500,   2500, 155,   0,   0 },
  {  2500,   2500, 155, 255,   0 },
  {  2500,   2500,   0, 255,   0 },
  {  2500,   2500,   0, 255, 255 },
  {  2500,   2500,   0,   0, 255 },
  {  2500,   2500, 155,   0, 255 },
  {  2500,      0,   0,   0,   0 },
  {  2500,   2500, 154,  32,   0 },
  {  2500,   2500, 154, 128,   0 },
  {  2500,   2500, 154, 240,   0 },
  {  2500,   2500, 128, 240,   0 },
  {     0,   2500,   0,   0,   0 },
  {  2500,   2500,   0,  16, 255 },
  {  2500,   2500,   0, 128, 255 },
  {  2500,   2500,   0, 240, 128 },
  {  2500,   2500,  16,  16, 240 },
  {  2500,   2500, 140,  16, 240 },
  {  2500,   2500,  64,   0, 250 },
  {     0,   2500,  10,  10,  10 },
  {     0,   2500,   0,   0,   0 },
  {  2500,   2500, 140,   0, 240 },
  {  2500,   2500,  32,   0, 240 },
  {  2500,   2500, 128,   0, 128 },
  {  2500,   2500, 140,   0,  32 },
  {  2500,      0,   0,   0,  10 },
  {  2500,      0,   0,   0,   0 },
  {  1000,   1000,   0,   0,   0 },
  {  1000,   1000,  32,   0,   0 },
  {  1000,   1000,  64,   0,   0 },
  {     0,   1000,  96,   0,   0 },
  {  1000,      0, 128,   0,   0 },
  {  1000,      0, 160,  32,   0 },
  {  1000,      0, 192,  64,   0 },
  {  1000,      0, 124,  96,   0 },
  {     0,   1000, 155, 128,   0 },
  {  1000,   1000,   0, 160,   0 },
  {     0,   1000,   0, 192,   0 },
  {  1000,   1000,   0, 224,  32 },
  {  1000,      0,   0, 255,  64 },
  {  1000,      0,   0,   0,  96 },
  {  1000,      0,   0,   0, 128 },
  {  1000,      0,   0,   0, 160 },
  {  1000,      0,   0,   0, 192 },
  {  1000,      0,   0,   0, 224 },
  {  1000,   1000,   0,   0, 255 },
  {  1000,      0,   0,   0,   0 },
  {     0,   1000,   0,   0, 255 },
  {  1000,   1000,  32,   0,   0 },
  {  1000,   1000,  96,   0,   0 },
  {  1000,   1000, 160,   0,   0 },
  {  1000,      0, 255,   0,   0 },
  {  1000,   1000,   0,  96,   0 },
  {  1000,   1000,   0, 160,  32 },
  {  1000,   1000,   0, 224,  64 },
  {  1000,   1000,   0, 255,  96 },
  {  1000,   1000,   0,   0, 128 },
  {  1000,   1000,   0,   0, 160 },
  {  1000,   1000,   0,  32, 192 },
  {  1000,   1000,   0,  64, 224 },
  {  1000,   1000,   0,  96, 225 },
  {  1000,   1000,   0, 128,   0 },
  {  1000,   1000,   0, 160,   0 },
  {  1000,   1000,   0, 192,  32 },
  {  1000,   1000,   0, 224,  64 },
  {  1000,   1000,   0, 255,  96 },
  {  1000,   1000,   0,   0, 255 },
  {  1000,   1000,   0,   0,   0 },
  {     0,      0,   0,   0,   0 }
};



// This function delays the specified number of 10 microseconds
void delay_ten_us(unsigned long int us) {
  unsigned long int count;
  const unsigned long int DelayCount=6;  // this value was determined by trial and error

  while (us != 0) {
    // Toggling PB5 is done here to force the compiler to do this loop, rather than optimize it away
    //   NOTE:  Below you will see: "0b00100000".
    //   This is an 8-bit binary number with all bits set to 0 except for the the 6th one from the right.
    //   Since bits in binary numbers are labeled starting from 0, the bit in this binary number that is set to 1
    //     is called PB5, which is the one unsed PORTB pin, which is why we can use it here
    //     (to fool the optimizing compiler to not optimize away this delay loop).
    for (count=0; count <= DelayCount; count++) {PINB |= 0b00100000;};
    us--;
  }
}



// This function delays (1.56 microseconds * x) + 2 microseconds
//   (determined empirically)
//    e.g.  if x = 1, the delay is (1 * 1.56) + 2 = 5.1 microseconds
//          if x = 255, the delay is (255 * 1.56) + 2 = 399.8 microseconds
void delay_x_us(unsigned long int x) {
  unsigned long int count;
  const unsigned long int DelayCount=0;  // the shortest delay

  while (x != 0) {
    for (count=0; count <= DelayCount; count++) {PINB |= 0b00100000;};
    x--;
  }
}



void sendrgbElement( int index ) {
  int FadeTime = pgm_read_word(&lightTab[index].fadeTime);
  int HoldTime = pgm_read_word(&lightTab[index].holdTime);

  unsigned char Red = 255 - pgm_read_byte(&lightTab[index].red);
  unsigned char Green = 255 - pgm_read_byte(&lightTab[index].green);
  unsigned char Blue = 255 - pgm_read_byte(&lightTab[index].blue);

  // get previous RGB brightness values from lightTab
  unsigned char redPrev = 0;  
  unsigned char greenPrev = 0;
  unsigned char bluePrev = 0; 

  if (index != 0) {
    redPrev = 255 - pgm_read_byte(&lightTab[index-1].red);
    greenPrev = 255 - pgm_read_byte(&lightTab[index-1].green);
    bluePrev = 255 - pgm_read_byte(&lightTab[index-1].blue);
  }

  // set color timing values
  //   everytime the fadeCounter reaches this timing value in the fade loop
  //   we will update the value for the color (default value of 0 for no updating)
  int redTime = 0;
  int greenTime = 0;
  int blueTime = 0;

  // set values of temp colors
  //   starting from the previous color values,
  //   these will change to the color values just gotten from rgbElement over fadeTime
  unsigned char redTemp = redPrev;
  unsigned char greenTemp = greenPrev;
  unsigned char blueTemp = bluePrev;

  // fade LEDs up or down, from previous values to current values
  int redDelta = Red - redPrev;                // total amount to fade red value (up or down) during fadeTime
  int greenDelta = Green - greenPrev;          // total amount to fade green value (up or down) during fadeTime
  int blueDelta = Blue - bluePrev;             // total amount to fade blue value (up or down) during fadeTime

  if (redDelta != 0) {
    redTime = (FadeTime / redDelta);           // increment Red value every time we reach this fade value in the fade loop
    if (redTime < 0) redTime = -redTime;       //    absolute value
    redTime = redTime + 1;                     // adjust for truncation of integer division
  }                                            //
  int redTimeInc = redTime;                    // increment Red value every time the fadeCounter increments this amount

  if (greenDelta != 0) {
    greenTime = (FadeTime / greenDelta);       // increment Green value every time we reach this fade value in the fade loop
    if (greenTime < 0) greenTime = -greenTime; //    absolute value
    greenTime = greenTime + 1;                 // adjust for truncation of integer division
  }                                            //
  int greenTimeInc = greenTime;                // increment Green value every time the fadeCounter increments this amount

  if (blueDelta != 0) {
    blueTime = (FadeTime / blueDelta);         // increment Blue value every time we reach this fade value in the fade loop
    if (blueTime < 0) blueTime = -blueTime;    //    absolute value
    blueTime = blueTime + 1;                   // adjust for truncation of integer division
  }                                            //
  int blueTimeInc = blueTime;                  // increment Blue value every time the fade value increments this amount

  // set color increment values
  //   the amount to increment color value each time we update it in the fade loop
  //   (default value of 1, to slowly increase brightness each time through the fade loop)
  unsigned char redInc = 1;
  unsigned char greenInc = 1;
  unsigned char blueInc = 1;
  // if we need to fade down the brightness, then make the increment values negative
  if (redDelta < 0) redInc = -1;
  if (greenDelta < 0) greenInc = -1;
  if (blueDelta < 0) blueInc = -1;

  // if FadeTime = 0, then just set the LEDs blinking at the RGB values (the fade loop will not be executed)
  if (FadeTime == 0) {
    OCR1A = Red;       // update PWM for Red LED on OC1A (pin 6)
    OCR1B = Blue;      // update PWM for Blue LED on OC1B (pin 3)
    OCR0A = Green;       // update PWM for Red LED on OC0A (pin 0)
  }

  // fade loop
  //   this loop will independently fade each LED up or down according to all of the above variables
  //   (it will take a length of time, FadeTime, to accomplish the task)
  //   this loop is not executed if FadeTime = 0 (since 1 is not <= 0, in the "for" loop)
  for (int fadeCounter=1; fadeCounter<=FadeTime; fadeCounter++) {
    if ( fadeCounter == redTime ) {
      redTemp = redTemp + redInc;                 // increment to next red value
      redTime = redTime + redTimeInc;             // we'll increment Red value again when FadeTime reaches new redTime
    }
    if ( fadeCounter == greenTime ) {
      greenTemp = greenTemp + greenInc;           // increment to next green value
      greenTime = greenTime + greenTimeInc;       // we'll increment Green value again when FadeTime reaches new greenTime
    }
    if ( fadeCounter == blueTime ) {
      blueTemp = blueTemp + blueInc;              // increment to next blue value
      blueTime = blueTime + blueTimeInc;          // we'll increment Blue value again when FadeTime reaches new blueTime
    }

    OCR1A = redTemp;
    OCR1B = blueTemp;
    OCR0A = greenTemp;

	// delay for a period of 1ms
    delay_ten_us(100);
  }

  // set all LEDs to new brightness values
  OCR1A = Red;
  OCR1B = Blue;
  OCR0A = Green;

  // hold all LEDs at current values
  for (int holdCounter=0; holdCounter<HoldTime; holdCounter++) {
	// delay for a period of 1ms
	delay_ten_us(100);
  }
}

int initialize(void) {
  // disable the Watch Dog Timer (since we won't be using it, this will save battery power)
  MCUSR = 0b00000000;   // first step:   WDRF=0 (Watch Dog Reset Flag)
  WDTCR = 0b00011000;   // second step:  WDCE=1 and WDE=1 (Watch Dog Change Enable and Watch Dog Enable)
  WDTCR = 0b00000000;   // third step:   WDE=0
  // turn off power to the USI and ADC modules (since we won't be using it, this will save battery power)
  PRR = 0b00000011;
  // disable all Timer interrupts
  TIMSK = 0x00;         // setting a bit to 0 disables interrupts
  // set up the input and output pins (the ATtiny25 only has PORTB pins)
  DDRB = 0b00010111;    // setting a bit to 1 makes it an output, setting a bit to 0 makes it an input
                        //   PB5 (unused) is input
                        //   PB4 (Blue LED) is output
                        //   PB3 (IR detect) is input -- ignored for Dan
                        //   PB2 (Blue LED) is output
                        //   PB1 (Red LED) is output
                        //   PB0 (Green LED) is output
  PORTB = 0x00;         //   For Dan: inverse to account for common-cathode RGB LED



  // We will use Timer 1 to fade the Red and Blue LEDs up and down
  //
  // start up Timer1 in PWM Mode at 122Hz to drive Red LED on output OC1A and Blue LED on output OC1B:
  //   8-bit Timer1 OC1A (PB1, pin 6) and OC1B (PB4, pin 3) are set up as outputs to toggle in PWM mode
  //   Fclk = Clock = 8MHz
  //   Prescale = 256
  //   MAX = 255 (by setting OCR1C=255)
  //   OCR1A =  0 (as an initial value -- this value will increase to increase brightness of Red LED)
  //   OCR1B =  0 (as an initial value -- this value will increase to increase brightness of Blue LED)
  //   F = Fclk / (Prescale * (MAX+1) ) = 122Hz
  // There is nothing too important about driving the Red and Blue LEDs at 122Hz, it is somewhat arbitrary,
  //   but it is fast enough to make it seem that the Red and Blue LEDs are not flickering.
  // Later in the firmware, the OCR1A and OCR1B compare register values will change,
  //   but the period for Timer1 will always remain the same (with F = 122Hz, always) --
  //   with OCR1A = 0, the portion of the period with the Red LED on is a minimum
  //     so the Red LED is very dim,
  //   with OCR1A = 255, the portion of the period with the Red LED on is a maximum
  //     so the Red LED is very bright.
  //   with OCR1B = 0, the portion of the period with the Blue LED on is a minimum
  //     so the Blue LED is very dim,
  //   with OCR1B = 255, the portion of the period with the Blue LED on is a maximum
  //     so the Blue LED is very bright.
  //   the brightness of the Red LED can be any brightness between the min and max
  //     by varying the value of OCR1A between 0 and 255.
  //   the brightness of the Blue LED can be any brightness between the min and max
  //     by varying the value of OCR1B between 0 and 255.
  //
  // Please see the ATtiny25 datasheet for descriptions of these registers.
  GTCCR = 0b01110000;   // TSM=0 (we are not using synchronization mode)
                        // PWM1B=1 for PWM mode for compare register B
                        // COM1B1:0=11 for inverting PWM on OC1B (Blue LED output pin)
                        // FOC1B=0 (no Force Output Compare for compare register B)
                        // FOC1A=0 (no Force Output Compare for compare register A)
                        // PSR1=0 (do not reset the prescaler)
                        // PSR0=0 (do not reset the prescaler)
  TCCR1 = 0b01111001;   // CTC1=0 for PWM mode
                        // PWM1A=1 for PWM mode for compare register A
                        // COM1A1:0=11 for inverting PWM on OC1A (Red LED output pin)
                        // CS13:0=1001 for Prescale=256 (this starts Timer 1)
  OCR1C = 255;   // sets the MAX count for the PWM to 255 (to get PWM frequency of 122Hz)
  OCR1A = 0;  // start with minimum brightness for Red LED on OC1A (PB1, pin 6)
  OCR1B = 0;  // start with minimum brightness for Blue LED on OC1B (PB4, pin 3)

//  DDRB |= (1 << 0); // OC0A on PB0

  TCCR0A |= ((1 << COM0A1) | (1 << COM0A0) // COM0A1 - COM0A0 (Set OC0A on Compare Match, clear OC0A at TOP)
		 | (1 << WGM01) | (1 << WGM00)); // WGM01 - WGM00 (set fast PWM)
  TCCR0B |= (1 << CS01); // Start timer at Fcpu / 256
  OCR0A = 0; // initialize Output Compare Register A to 0

//   for (int i = 0 ; i < 255 ; i++ ) // For loop (Up counter 0 - 255)
//	    {
//			 OCR0A = i; // Update Output Compare Register (PWM 0 - 255)
//			  delay_ten_us(100);
//			   }


//  TCCR0A = 0b11000011;  // COM0A1:0=10 to toggle OC0A on Compare Match
//                        // COM0B1:0=00 to disconnect OC0B
//                        // bits 3:2 are unused
//                        // WGM01:00=11 
//  TCCR0B = 0b00001010;  // FOC0A=0 (no force compare)
//                        // F0C0B=0 (no force compare)
//                        // bits 5:4 are unused
//                        // WGM2=1 for CTC Mode (WGM01:00=10 in TCCR0A)?
//                        // CS02:00=001 for divide by 1 prescaler (this starts Timer0)
//  OCR0A = 0;
}

int teardown(void) {
  // Shut down everything and put the CPU to sleep
  cli();                 // disable microcontroller interrupts
  delay_ten_us(10000);   // wait .1 second
  TCCR0B &= 0b11111000;  // CS02:CS00=000 to stop Timer0 (turn off IR emitter)
  TCCR0A &= 0b00111111;  // COM0A1:0=00 to disconnect OC0A from PB0 (pin 5)
  TCCR1 &= 0b11110000;   // CS13:CS10=0000 to stop Timer1 (turn off Red and Blue LEDs)
  TCCR1 &= 0b11001111;   // COM1A1:0=00 to disconnect OC1A from PB1 (pin 6)
  GTCCR &= 0b11001111;   // COM1B1:0=00 to disconnect OC1B from PB4 (pin 3)
  DDRB = 0x00;           // make PORTB all inputs (saves battery power)
  PORTB = 0xFF;          // enable pull-up resistors on all PORTB input pins (saves battery power)
  MCUCR |= 0b00100000;   // SE=1 (bit 5)
  MCUCR |= 0b00010000;   // SM1:0=10 to enable Power Down Sleep Mode (bits 4, 3)
  sleep_cpu();           // put CPU into Power Down Sleep Mode
}

int main(void) {
  initialize();

  int index = 0;
  // Number of times to run through the light array
  for (int count=0; count<360; count++) {
    do {
      sendrgbElement(index);
      index++;
    } while (!((pgm_read_word(&lightTab[index].fadeTime) == 0) && (pgm_read_word(&lightTab[index].holdTime) == 0)));
    index = 0;
  }

  teardown();
}
