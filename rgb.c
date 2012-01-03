/*
RGB Wave Firmware
for use with ATtiny25
AS220
Mitch Altman
Dan Levin
09-Mar-09 -- RBG instead of RGB
21-Dec-11 -- hacked for Dan's RGB LED modules

Distributed under Creative Commons 3.0 -- Attib & Share Alike

Parts list for this RGB Light project:
1   ATtiny25
1   CR2032 coin-cell battery
1   CR2032 battery holder
1   small slide switch (for on-off)
1   RGB LED (common cathode)
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
       pin 2  PB3 - 
       pin 3  OC1B - Blue lead of RGB LED 
	   			(//anode -- through a 47 ohm current limiting resistor)
       pin 4  gro//und  (pin programming pin 6)
       pin 5  OC0A -  Green lead of RGB LED 
	   			(anode -- also connects to programming port pin 4)
       pin 6  OC1A - Red lead of RGB LED 
	   			(anode -- through a 47 ohm current limiting resistor) (also connects to programming port pin 1)
       pin 7  PB2 - programming port pin 3
       pin 8  +3v (programming pin 2) 

	Programmer:
		pin 1: pin6
		pin 2: +3v
		pin 3: pin7
		pin 4: pin5
		pin 5: pin1
		pin 6: ground	
    This firmware requires that the clock frequency of the ATtiny
       is the default that it is shipped with:  8.0MHz
*/


#include <avr/io.h>             // this contains all the IO port definitions
#include <avr/interrupt.h>      // definitions for interrupts
#include <avr/sleep.h>          // definitions for power-down modes
#include <avr/pgmspace.h>       // definitions or keeping constants in program memory
#define TIMESTEP 100

int Pause = 0;

ISR(PCINT0_vect) {
//	while (!(PORTB & 0b00001000)) {
//	  delay_ten_us(15000);
//	}
//	
//	switch(Pause) {
//		case 0:
//			Pause++;
//			break;
//		case 1:
//			Pause++;
//			break;
//		case 2:
//			Pause++;
//			break;
//		case 3:
//			Pause = 0;
//			break;
//	}
}

/*
The following Light Table consists of any number of rgbElements that will fit into the
2k flash ROM of the ATtiny25 microcontroller.

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
  // Times in ms
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
  {  2500,      0,   0,   0,   0 },
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
  {  2500,      0,   0,   0,   0 },
  {  2500,   2500,   0,  16, 255 },
  {  2500,   2500,   0, 128, 255 },
  {  2500,   2500,   0, 240, 128 },
  {  2500,   2500,  16,  16, 240 },
  {  2500,   2500, 140,  16, 240 },
  {  2500,   2500,  64,   0, 250 },
  {  2500,   2500,  10,  10,  10 },
  {  2500,      0,   0,   0,   0 },
  {  2500,   2500, 140,   0, 240 },
  {  2500,   2500,  32,   0, 240 },
  {  2500,   2500, 128,   0, 128 },
  {  2500,   2500, 140,   0,  32 },
  {  2500,      0,   0,   0,  10 },
  {  2500,      0,   0,   0,   0 },
  {  1000,   1000,   0,   0,   0 },
  {  1000,   1000,  32,   0,   0 },
  {  1000,   1000,  64,   0,   0 },
  {  1000,      0,  96,   0,   0 },
  {  1000,      0, 128,   0,   0 },
  {  1000,      0, 160,  32,   0 },
  {  1000,      0, 192,  64,   0 },
  {  1000,      0, 124,  96,   0 },
  {  1000,   1000, 155, 128,   0 },
  {  1000,   1000,   0, 160,   0 },
  {  1000,   1000,   0, 192,   0 },
  {  1000,   1000,   0, 224,  32 },
  {  1000,      0,   0, 255,  64 },
  {  1000,      0,   0,   0,  96 },
  {  1000,      0,   0,   0, 128 },
  {  1000,      0,   0,   0, 160 },
  {  1000,      0,   0,   0, 192 },
  {  1000,      0,   0,   0, 224 },
  {  1000,   1000,   0,   0, 255 },
  {  1000,      0,   0,   0,   0 },
  {  1000,   1000,   0,   0, 255 },
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
  const unsigned long int DelayCount = 7;  // this value was determined by trial and error

  while (us > 0) {
    // Toggling PB5 is done here to force the compiler to do this loop, rather than optimize it away
    //     PB5 is the one unsed PORTB pin, which is why we can use it here
    for (count=0; count < DelayCount; count++) {PINB |= 0b00100000;};
    us--;
  }
}


//// This function delays (1.56 microseconds * x) + 2 microseconds
////   (determined empirically)
////    e.g.  if x = 1, the delay is (1 * 1.56) + 2 = 5.1 microseconds
////          if x = 255, the delay is (255 * 1.56) + 2 = 399.8 microseconds
//void delay_x_us(unsigned long int x) {
//  unsigned long int count;
//  const unsigned long int DelayCount=0;  // the shortest delay
//
//  while (x != 0) {
//    for (count=0; count <= DelayCount; count++) {PINB |= 0b00100000;};
//    x--;
//  }
//}


void sendrgbElement( int index ) {
  int FadeTime = pgm_read_word(&lightTab[index].fadeTime);
  int HoldTime = pgm_read_word(&lightTab[index].holdTime);

//  unsigned char Red = 255 - pgm_read_byte(&lightTab[index].red);
  unsigned char Red =  pgm_read_byte(&lightTab[index].red);
  unsigned char Green = pgm_read_byte(&lightTab[index].green);
  unsigned char Blue = pgm_read_byte(&lightTab[index].blue);

  // get previous RGB brightness values from lightTab
  unsigned char redPrev = 0;
  unsigned char greenPrev = 0;
  unsigned char bluePrev = 0;

  if (index != 0) {
//    redPrev = 255 - pgm_read_byte(&lightTab[index-1].red);
    redPrev =  pgm_read_byte(&lightTab[index-1].red);
    greenPrev = pgm_read_byte(&lightTab[index-1].green);
    bluePrev = pgm_read_byte(&lightTab[index-1].blue);
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

  // UGLY
  // if FadeTime = 0, then just set the LEDs blinking at the RGB values (the fade loop will not be executed)
  if (FadeTime == 0) {
    OCR1A = Red;       // update PWM for Red LED on OC1A (pin 6)
    OCR1B = Blue;      // update PWM for Blue LED on OC1B (pin 3)
    OCR0A = Green;       // update PWM for Red LED on OC0A (pin 0)
  }

  // fade loop
  //   this loop will independently fade each LED up or down according to all of the above variables
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
    delay_ten_us(TIMESTEP);

	while (Pause == 2) {
		delay_ten_us(TIMESTEP);
	}

  }

  // set all LEDs to new brightness values
  OCR1A = Red;
  OCR1B = Blue;
  OCR0A = Green;

  // hold all LEDs at current values
  for (int holdCounter=0; holdCounter<HoldTime; holdCounter++) {
	// delay for a period of 1ms
	delay_ten_us(TIMESTEP);

	while (Pause == 2) {
		delay_ten_us(TIMESTEP);
	}

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
  DDRB = 0b00010011;    // setting a bit to 1 makes it an output, setting a bit to 0 makes it an input
                        //   PB5 (unused)
                        //   PB4 Blue LED is output
                        //   PB3 Pause interrupt is input
                        //   PB2 (unused)
                        //   PB1 Red LED is output
                        //   PB0 Green LED is output
						//
  PORTB |= (1 << PB3); 	// Turns on the pullup for PB3

  GIMSK = 0b00100000;   // PCIE=1 to enable Pin Change Interrupts
  PCMSK = 0b00001000;   // PCINT3 bit = 1 to enable Pin Change Interrupts for PB3
  sei();                // enable microcontroller interrupts

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
  //   Driving the Red and Blue LEDs at 122Hz is somewhat arbitrary, but it is
  //   fast enough to make it seem that the Red and Blue LEDs are not
  //   flickering. Later in the firmware, the OCR1A and OCR1B compare register values will change,
  //   but the period for Timer1 will always remain the same (with F = 122Hz, always) --
  //
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

  // sets the MAX count for the PWM to 255 (to get PWM frequency of 122Hz)
  OCR1C = 255;
  OCR1A = 0;
  OCR1B = 0;


  // We will use Timer 0 to fade the Green LED up and down
  TCCR0A |= ((1 << COM0A1) | (1 << COM0A0) // COM0A1 - COM0A0 (Set OC0A on Compare Match, clear OC0A at TOP)
		 | (1 << WGM01) | (1 << WGM00)); // WGM01 - WGM00 (set fast PWM)
  TCCR0B |= (1 << CS01); // Start timer at Fcpu / 256

  OCR0A = 0; // initialize Output Compare Register A to 0
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
  // Run 50 times ~6 minutes = 5 hours then stop
  for (int count=0; count<50; count++) {
    do {
      sendrgbElement(index);
      index++;
    } while (!((pgm_read_word(&lightTab[index].fadeTime) == 0) && (pgm_read_word(&lightTab[index].holdTime) == 0)));
    index = 0;
  }

  teardown();
}
