// "Pacifica_siren"
// create a simple, but beautiful/haunting reactive piece
// Written to run on "Sign of Sirens" by N. Izzi Long
// October 2020, !Bob
// Still for Dan.

// Primarily based on:
//  "Pacifica"
//  Gentle, blue-green ocean waves.
//  December 2019, Mark Kriegsman and Mary Corey March.
//  For Dan.


#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
FASTLED_USING_NAMESPACE

#define DATA_PIN            3
#define NUM_LEDS            256
#define MAX_POWER_MILLIAMPS 12000
#define LED_TYPE            WS2812B
#define COLOR_ORDER         RGB

//PIR Sensor
#define pirPin 2
int calibrationTime = 30;
long unsigned int lowIn;
long unsigned int pause = 5000;
boolean lockLow = true;
boolean takeLowTime;
int PIRValue = 0;
int lastPotentiometer = 0;
int brightness = 200;
int adjust_amount = 1;

int loop_count, cooldown_count;
boolean angry, angry_first;

//////////////////////////////////////////////////////////////////////////

CRGB leds[NUM_LEDS];

void setup() {
  Serial.begin(9600);  
  delay( 3000); // 3 second delay for boot recovery, and a moment of silence
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS)
        .setCorrection( TypicalLEDStrip );
  FastLED.setMaxPowerInVoltsAndMilliamps( 5, MAX_POWER_MILLIAMPS);

  pinMode(pirPin, INPUT);
  loop_count = 0;
  angry = false;
  angry_first = true;
}

void loop()
{
  EVERY_N_MILLISECONDS( 20) {
    PIRSensor();  
    pacifica_loop();
    if(lastPotentiometer != analogRead(A0))
    {
      lastPotentiometer = analogRead(A0);
      brightness = lastPotentiometer * (255.0/1023.0);
      FastLED.setBrightness(brightness);  
      Serial.println(brightness);      
    }    
    FastLED.show();
  }
}

void PIRSensor() {
   if(digitalRead(pirPin) == HIGH) {
      if(lockLow) {
         PIRValue = 1;
         lockLow = false;
      }
      takeLowTime = true;
   }
   if(digitalRead(pirPin) == LOW) {
      if(takeLowTime){
         lowIn = millis();takeLowTime = false;
      }
      if(!lockLow && millis() - lowIn > pause) {
         PIRValue = 0;
         lockLow = true;
      }
   }
}

//////////////////////////////////////////////////////////////////////////
//
// The code for this animation is more complicated than other examples, and 
// while it is "ready to run", and documented in general, it is probably not 
// the best starting point for learning.  Nevertheless, it does illustrate some
// useful techniques.
//
//////////////////////////////////////////////////////////////////////////
//
// In this animation, there are four "layers" of waves of light.  
//
// Each layer moves independently, and each is scaled separately.
//
// All four wave layers are added together on top of each other, and then 
// another filter is applied that adds "whitecaps" of brightness where the 
// waves line up with each other more.  Finally, another pass is taken
// over the led array to 'deepen' (dim) the blues and greens.
//
// The speed and scale and motion each layer varies slowly within independent 
// hand-chosen ranges, which is why the code has a lot of low-speed 'beatsin8' functions
// with a lot of oddly specific numeric ranges.
//
// These three custom blue-green color palettes were inspired by the colors found in
// the waters off the southern coast of California, https://goo.gl/maps/QQgd97jjHesHZVxQ7
//
CRGBPalette16 pacifica_palette_1 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x14554B, 0x28AA50 };
CRGBPalette16 pacifica_palette_2 = 
    { 0x000507, 0x000409, 0x00030B, 0x00030D, 0x000210, 0x000212, 0x000114, 0x000117, 
      0x000019, 0x00001C, 0x000026, 0x000031, 0x00003B, 0x000046, 0x0C5F52, 0x19BE5F };
CRGBPalette16 pacifica_palette_3 = 
    { 0x000208, 0x00030E, 0x000514, 0x00061A, 0x000820, 0x000927, 0x000B2D, 0x000C33, 
      0x000E39, 0x001040, 0x001450, 0x001860, 0x001C70, 0x002080, 0x1040BF, 0x2060FF };


void pacifica_loop()
{
  loop_count++;

  if(PIRValue)
  {
    //be angry now
    be_angry();
    angry = true;
  }else{
    //if angry is set, this is our first loop
    if(angry)
    {
      //this decrements each loop and blends us back to our pattern, disable the angry flag until it trips again.
      cooldown_count = 255;
      angry = false;
    }

    if(cooldown_count > 0)
    {
      //when strip is too dim, the fade happens too fast.  Slow it down based on ranges of brightness.
      bool do_cooldown = false;
      
      if(brightness < 80)
      {
        if(cooldown_count % 12 == 1)
          do_cooldown = true;
      }

      if(brightness > 79 && brightness < 120)
      {
        if(cooldown_count % 8 == 1)
          do_cooldown = true;
      }      

      if(brightness > 119 && brightness < 190)
      {
        if(cooldown_count % 4 == 1)
          do_cooldown = true;
      }

      if(brightness > 189 && brightness < 211)
      {
        if(cooldown_count % 2 == 1)
          do_cooldown = true;
      }

      if(brightness > 210)
        do_cooldown = true;
      
      if(do_cooldown)
      {
        be_less_angry();
      }
      cooldown_count--;
    }else{
      //fade is done, go back to pacifica
      // Increment the four "color index start" counters, one for each wave layer.
      // Each is incremented at a different speed, and the speeds vary over time.
      static uint16_t sCIStart1, sCIStart2, sCIStart3, sCIStart4;
      static uint32_t sLastms = 0;
      uint32_t ms = GET_MILLIS();
      uint32_t deltams = ms - sLastms;
      sLastms = ms;
      uint16_t speedfactor1 = beatsin16(3, 179, 269);
      uint16_t speedfactor2 = beatsin16(4, 179, 269);
      uint32_t deltams1 = (deltams * speedfactor1) / 256;
      uint32_t deltams2 = (deltams * speedfactor2) / 256;
      uint32_t deltams21 = (deltams1 + deltams2) / 2;
      sCIStart1 += (deltams1 * beatsin88(1011,10,13));
      sCIStart2 -= (deltams21 * beatsin88(777,8,11));
      sCIStart3 -= (deltams1 * beatsin88(501,5,7));
      sCIStart4 -= (deltams2 * beatsin88(257,4,6));
  
      // Clear out the LED array to a dim background blue-green
      fill_solid( leds, NUM_LEDS, CRGB( 2, 6, 10));
  
      // Render each of four layers, with different scales and speeds, that vary over time
      pacifica_one_layer( pacifica_palette_1, sCIStart1, beatsin16( 3, 11 * 256, 14 * 256), beatsin8( 10, 70, 130), 0-beat16( 301) );
      pacifica_one_layer( pacifica_palette_2, sCIStart2, beatsin16( 4,  6 * 256,  9 * 256), beatsin8( 17, 40,  80), beat16( 401) );
      pacifica_one_layer( pacifica_palette_3, sCIStart3, 6 * 256, beatsin8( 9, 10,38), 0-beat16(503));
      pacifica_one_layer( pacifica_palette_3, sCIStart4, 5 * 256, beatsin8( 8, 10,28), beat16(601));
  
      // Add brighter 'whitecaps' where the waves lines up more
      pacifica_add_whitecaps();
  
      // Deepen the blues and greens a bit
      pacifica_deepen_colors();
    }
  }
}

// Add one layer of waves into the led array
void pacifica_one_layer( CRGBPalette16& p, uint16_t cistart, uint16_t wavescale, uint8_t bri, uint16_t ioff)
{
  uint16_t ci = cistart;
  uint16_t waveangle = ioff;
  uint16_t wavescale_half = (wavescale / 2) + 20;
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    waveangle += 250;
    uint16_t s16 = sin16( waveangle ) + 32768;
    uint16_t cs = scale16( s16 , wavescale_half ) + wavescale_half;
    ci += cs;
    uint16_t sindex16 = sin16( ci) + 32768;
    uint8_t sindex8 = scale16( sindex16, 240);
    CRGB c = ColorFromPalette( p, sindex8, bri, LINEARBLEND);
    leds[i] += c;
  }
}

void be_angry(){
  for(int i=0;i<NUM_LEDS;i++)
    leds[i] = CHSV(0,random(200,256),brightness);
}

void be_less_angry(){
  for(int i=0;i<NUM_LEDS;i++)
  {
    fadeTowardColor(leds[i], CRGB(2,6,10),adjust_amount);
  }
}

// Add extra 'white' to areas where the four layers of light have lined up brightly
void pacifica_add_whitecaps()
{
  uint8_t basethreshold = beatsin8( 9, 55, 65);
  uint8_t wave = beat8( 7 );
  
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    uint8_t threshold = scale8( sin8( wave), 20) + basethreshold;
    wave += 7;
    uint8_t l = leds[i].getAverageLight();
    if( l > threshold) {
      uint8_t overage = l - threshold;
      uint8_t overage2 = qadd8( overage, overage);
      leds[i] += CRGB( overage, overage2, qadd8( overage2, overage2));
    }
  }
}

// Deepen the blues and greens
void pacifica_deepen_colors()
{
  for( uint16_t i = 0; i < NUM_LEDS; i++) {
    leds[i].blue = scale8( leds[i].blue,  145); 
    leds[i].green= scale8( leds[i].green, 200); 
    leds[i] |= CRGB( 2, 5, 7);
  }
}

//next two functions taken from mrExplore's comment here:
//https://forum.arduino.cc/index.php?topic=569388.0

// Blend one CRGB color toward another CRGB color by a given amount.
// Blending is linear, and done in the RGB color space.
// This function modifies 'cur' in place.
CRGB fadeTowardColor( CRGB& cur, const CRGB& target, uint8_t amount)
{
  nblendU8TowardU8( cur.red,   target.red,   amount);
  nblendU8TowardU8( cur.green, target.green, amount);
  nblendU8TowardU8( cur.blue,  target.blue,  amount);
  return cur;
}

// Helper function that blends one uint8_t toward another by a given amount
void nblendU8TowardU8( uint8_t& cur, const uint8_t target, uint8_t amount)
{
  if( cur == target) return;
 
  if( cur < target ) {
    uint8_t delta = target - cur;
    delta = scale8_video( delta, amount);
    cur += delta;
  } else {
    uint8_t delta = cur - target;
    delta = scale8_video( delta, amount);
    cur -= delta;
  }
}
