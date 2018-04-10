//
// Audio S-meter v0.2 by PA3FYM April 2018
//
// Voltage Controlled Oscillator (VCO)
//
// v0.2 April 2018 
// - Timer0 in CTC mode is now tone generator
// - 9 bit ADC resolution in conjunction with 256 prescaler gives an acceptable
//   'tone' vs ADC input response. Note: tone frequency is related to 1/x !
//
// v0.1 March 2018
// - used tone() routine, but gave unsatisfying results and
//   too much (overhead) c0de (remember, I love assembler more than C++ !) 
//
// c0de for ATTiny13, fuses: 9.6 MHz internal clock, BOD disabled
// use MCUdude's MicroCore ATTiny13 library for the Arduino IDE
// to compile this sketch, see: https://github.com/MCUdude/MicroCore
//
// connect buzzer or speaker to PB0 (pin5) of ATTiny13 and RSSI (input)
// voltage to one of the internal ADConverters (this sketch uses ADC3)
//

#define rssi  A3                    // ADC3 (PB3, pin2) is RSSI input pin (max 5V)
#define tone_on  0b00000001         // PB0 on (audio out pin)
#define tone_off 0b11111110         // PB0 off
#define de_lay 40                   // overload beep delay in ms
#define loop_delay round(de_lay/8)  // loop() delay
#define threshold 480               // ADC value to produce overload beeps

uint16_t ema,old_val;

void setup() {

 analogReference(0);  // <-- 'secret' to let the internal ADC's work, see:
     // https://arduinodiy.wordpress.com/2015/04/23/analogread-on-attiny13
                        

 TCCR0A = (1<<COM0A0)|(1<<WGM01); // CTC mode, timer value = OCR0A
 TCCR0B = (1<<CS02);              // prescaler = 256, see ATTiny13 datasheet
// TCCR0B = (1<<CS01) | (1<<CS00);  // prescaler = 64
// TCCR0B = (1<<CS02) | (1<<CS00);  // prescaler = 1024

// Note: OCR0A = (uint32_t) (F_CPU / (2 * freq * prescaler)) - 1;
}

void loop() {
                                            // read RSSI voltage and reduce noise by
      uint32_t val = analogRead(rssi) >>1;  // discarding LSB of 10 bits conversion
      
      ema += (val - ema) >>1; // exp moving average with alfa = 0.50 ( = 1 right shift)
      
      if (val > old_val+1 || val < old_val-1) val = ema; // reduce jitter (sounds very annoying!) 
      else val = old_val;
   
      if (ema > threshold/30) DDRB |= tone_on; // some threshold to produce a tone
      else DDRB &= tone_off;                   // below threshold stay quiet
      
      OCR0A = (uint32_t) (F_CPU/(val <<11)) - 1;  // F_CPU = 9.6 MHz
      old_val = val;                              // remember last RSSI ADC-value
 
      if (ema >= threshold) {    // generate overload beeps above threshold
                                 // (sign to increase receiver attenuation!)
      _delay_ms(de_lay);         // some delay
      DDRB &= tone_off;          // tone off
      _delay_ms(de_lay - loop_delay); // again some delay
      }
      _delay_ms(loop_delay);     // overall loop delay 
}
