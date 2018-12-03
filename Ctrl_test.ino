#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Fonts/FreeSansBold12pt7b.h>
#include <Fonts/FreeSansBold9pt7b.h>
#include <Fonts/FreeSans12pt7b.h>
#include <Fonts/FreeSans9pt7b.h>
#include <Encoder.h>
#include <FastLED.h>
#include <Bounce2.h>
#include <Wire.h>

// Pin mapping
#define PIN_SPI1_SCK  13
#define PIN_SPI1_MOSI 11
#define PIN_SPI2_SCK  14
#define PIN_SPI2_MOSI 7
#define PIN_SPI2_CS   15
#define PIN_SPI2_DC   1
#define PIN_SPI2_RST  2
#define PIN_I2C_SCL   19
#define PIN_I2C_SDA   18
#define PIN_PWM_R     21
#define PIN_PWM_G     22
#define PIN_PWM_B     23
#define PIN_ENC_A     9
#define PIN_ENC_B     10
#define PIN_SW_BTN    8
#define PIN_CAN_TX    3
#define PIN_CAN_RX    4
#define PIN_JOY_UP    5
#define PIN_JOY_DN    6
#define PIN_JOY_RG    16
#define PIN_JOY_LF    17
#define PIN_JOY_CN    20
#define PIN_IO_EXT    0


// Encoder
Encoder myEnc(PIN_ENC_B, PIN_ENC_A);
long enc_pos_old  = -99999;
long enc_pos_curr = 0;
long encoder_value = 0;
long enc_max = 23;
long enc_min = 0;
long enc_scale = 1;

// LED ring
#define RING_MODES 6
#define RING_LENGTH 24
CRGB leds[RING_LENGTH];
CRGB __leds[RING_LENGTH];
unsigned char ring_origin = 9;
unsigned char ring_base = 1;
unsigned char ring_mode = 0;
unsigned char ring_hue = 0;
CRGB ring_color1, ring_color2, ring_cursor_color;
CRGB ring_behind_cursor = 0;

// Push button
Bounce enc_button = Bounce(PIN_SW_BTN, 10); // 10ms 

// 5-way Joystick
Bounce joy_up = Bounce(PIN_JOY_UP, 10); // 10ms
Bounce joy_down = Bounce(PIN_JOY_DN, 10); // 10ms 
Bounce joy_left = Bounce(PIN_JOY_LF, 10); // 10ms 
Bounce joy_right = Bounce(PIN_JOY_RG, 10); // 10ms 
Bounce joy_center = Bounce(PIN_JOY_CN, 10); // 10ms 

// OLED display
Adafruit_SSD1306 display(-1);
#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif


void setup()
{ 
  // Serial port
  Serial.begin(115200);
  Serial.print("Starting");

  // LED ring
  FastLED.addLeds<APA102, PIN_SPI1_MOSI, PIN_SPI1_SCK, BGR, DATA_RATE_MHZ(24)>(__leds, RING_LENGTH);
  FastLED.setBrightness(25);
//  FastLED.setBrightness(50);
  FastLED.setMaxPowerInVoltsAndMilliamps(5,200);
  SetMode(0);

  // Push button
  pinMode(PIN_SW_BTN, INPUT);

  // Joystick
  pinMode(PIN_JOY_UP, INPUT);
  pinMode(PIN_JOY_DN, INPUT);
  pinMode(PIN_JOY_LF, INPUT);
  pinMode(PIN_JOY_RG, INPUT);
  pinMode(PIN_JOY_CN, INPUT);

  // I2C
  Wire.setClock(1000000);
  
  // OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C, false);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.setFont(&FreeSans9pt7b);
//  display.setRotation(2);
  TextTest(0);
}


void SetMode(unsigned char mode)
{
  ring_mode = mode;
  switch(ring_mode)
  {
  case 0:
    enc_min = 0;
    enc_max = 23;
    ring_origin = 9;
    ring_cursor_color = CRGB::Black;
    // TODO: scaling, default, etc
    break;

  case 1:
    enc_min = 0;
    enc_max = 23;
    ring_origin = 9;
    ring_cursor_color = CRGB::Black;
    // TODO: scaling, default, etc
    break;
    
  case 2:
    enc_min = 0;
    enc_max = 20;
    ring_base = 3;
    ring_color1 = CRGB::Red;
    ring_color2 = CRGB::Green;
    ring_cursor_color = CRGB::White;
    ring_origin = 10 + (ring_base>>1);
    break;
    
  case 3:
    enc_min = 0;
    enc_max = 20;
    ring_base = 3;
    ring_color1 = CRGB::Red;
    ring_color2 = CRGB::Green;
    ring_cursor_color = CRGB::White;
    ring_origin = 10 + (ring_base>>1);
    break;
    
  case 4:
    enc_min = -50;
    enc_max = 50;
    ring_base = 5;
    ring_color1 = CRGB::Gray;
    ring_color2 = CRGB::Blue;
    ring_cursor_color = CRGB::Orange;
    ring_origin = 10 + (ring_base>>1);
    break;
    
  case 5:
    enc_min = -10;
    enc_max = 90;
//    enc_scale = 2;
    ring_base = 5;
    ring_color1 = CRGB::Yellow;
    ring_color2 = CRGB::Blue;
    ring_cursor_color = CRGB::Red;
    ring_origin = 10 + (ring_base>>1);
    break;
  }
  SetEncoder(0);
}

void SetLedRing()
{
  long led_index = 0;
  
  switch(ring_mode)
  {
  case 0:
    memset(leds, 0, sizeof(CRGB) * RING_LENGTH);
    fill_gradient(leds, 0, CHSV(ring_hue,255,200), RING_LENGTH, CHSV(ring_hue-1,255,200), FORWARD_HUES);
    ring_hue++;

    // Cursor
    ring_behind_cursor = leds[encoder_value];
    leds[encoder_value] = CRGB::Black; //ring_cursor_color;  // Black cursor
    break;

  case 1:
    fill_gradient(leds, 0, CHSV(ring_hue,255,200), RING_LENGTH, CHSV(ring_hue-1,255,200), FORWARD_HUES);
    ring_hue++;

    // Cursor
    ring_behind_cursor = leds[encoder_value];
    memset(leds, 0, sizeof(CRGB) * RING_LENGTH);
    leds[encoder_value] = ring_behind_cursor; //ring_cursor_color;  // Black cursor
    break;
    
  case 2:
    memset(leds, 0, sizeof(CRGB) * RING_LENGTH);
    fill_gradient_RGB (leds, 0, ring_color1, RING_LENGTH-ring_base -1, ring_color2);
    
    // Cursor
    ring_behind_cursor = leds[encoder_value];
    leds[encoder_value] = ring_cursor_color;
    break;

  case 3:
    memset(leds, 0, sizeof(CRGB) * RING_LENGTH);
//    fill_gradient_RGB (leds, 0, ring_color1, RING_LENGTH-ring_base -1, ring_color2);
    fill_gradient(leds, 0, CHSV(0,255,200), RING_LENGTH-ring_base -1, CHSV(95,255,200), FORWARD_HUES);
    
    // Cursor
    ring_behind_cursor = leds[encoder_value];
    leds[encoder_value] = ring_cursor_color;
    break;

  case 4:
    memset(leds, 0, sizeof(CRGB) * RING_LENGTH);
    fill_gradient_RGB(leds, 0, ring_color1, RING_LENGTH-ring_base -1, ring_color2);
    
    // Cursor
    // -50..50 -> 0..100 -> 0..18
    // (val-enc_min)/(enc_range) * led_range (division rounded to nearest)
    led_index = (((encoder_value-enc_min)*(RING_LENGTH-ring_base -1))  + ((enc_max-enc_min)/2)) / (enc_max-enc_min);
    ring_behind_cursor = leds[led_index];
    leds[led_index] = ring_cursor_color;
    break;

  case 5:
    memset(leds, 0, sizeof(CRGB) * RING_LENGTH);
    fill_gradient_RGB(leds, 0, ring_color1, RING_LENGTH-ring_base -1, ring_color2);
    
    // Cursor
    // 0..100 -> 0..18
    led_index = (((encoder_value-enc_min)*(RING_LENGTH-ring_base -1))  + ((enc_max-enc_min)/2)) / (enc_max-enc_min);
    ring_behind_cursor = leds[led_index];
    leds[led_index] = ring_cursor_color;
    break;
  }
  UpdateLedRing();
}

void UpdateLedRing()
{

  memcpy(__leds, &(leds[RING_LENGTH-ring_origin]), sizeof(CRGB) * ring_origin);
  memcpy(&(__leds[ring_origin]), leds, sizeof(CRGB) * (RING_LENGTH-ring_origin));
  FastLED.show();
}

void SetAnalogLed(CRGB rgb)
{
  analogWrite(PIN_PWM_R, 256 - rgb.r);
  analogWrite(PIN_PWM_G, 256 - rgb.g);
  analogWrite(PIN_PWM_B, 256 - rgb.b);
}

void HandleJoystick()
{
  if(joy_up.update() && joy_up.fell())
  {
    Serial.println("JOYSTICK: up");
  }
  if(joy_down.update() && joy_down.fell())
  {
    Serial.println("JOYSTICK: down");
  }
  if(joy_left.update() && joy_left.fell())
  {
    Serial.println("JOYSTICK: left");
  }
  if(joy_right.update() && joy_right.fell())
  {
    Serial.println("JOYSTICK: right");
  }
  if(joy_center.update() && joy_center.fell())
  {
    Serial.println("JOYSTICK: center - change ring mode");
    if(ring_mode+1 >= RING_MODES)
      SetMode(0);
    else
      SetMode(++ring_mode);
  }
}

void HandleButton()
{
  if(enc_button.update() && enc_button.fallingEdge())
  {
    Serial.println("Button pressed");
    myEnc.write(0); // Reset encoder
  }
}

void HandleEncoder()
{
  enc_pos_curr = myEnc.read(); // * enc_scale;

  if((enc_pos_curr != enc_pos_old))
  {
    if(enc_pos_curr > (enc_max << 2))
    {
      enc_pos_curr = (enc_max << 2);
      myEnc.write(enc_pos_curr);
    }
    if(enc_pos_curr < (enc_min << 2))
    {
      enc_pos_curr = (enc_min << 2);
      myEnc.write(enc_pos_curr);
    }
    enc_pos_old = enc_pos_curr;

    // Get rounded value
    if(enc_pos_curr >= 0)
      encoder_value = (enc_pos_curr + 2) / 4;
    else
      encoder_value = (enc_pos_curr - 2) / 4;

    // Scale
//    encoder_value *= enc_scale;
    
    Serial.print("ENCODER: ");
//    Serial.println(enc_pos);
    Serial.println(encoder_value);
  }
}

void SetEncoder(long value)
{
  // TODO: scaling
  if((value >= enc_min) && (value <= enc_max))
  {
    myEnc.write(value*4);
  }
}

void loop()
{
  HandleEncoder();
  
  // LED ring 
  SetLedRing();

  // Analog RGB LED
//  SetAnalogLed(CHSV(ring_hue, 255, 255));
  SetAnalogLed(ring_behind_cursor);

  // Push button
  HandleButton();

  // Joystick
  HandleJoystick();

  // Display
  TextTest(encoder_value);
  
//  FastLED.delay(20);
  delay(20);
}

void TextTest(int val)
{
  display.clearDisplay();
  display.drawRect(0, 0, display.width(), display.height(), WHITE);
//  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(4,22);
  display.print("Encoder: ");
  display.println(val);
//  display.setTextColor(BLACK, WHITE); // 'inverted' text
//  display.println(3.141592);
//  display.setTextSize(2);
//  display.setTextColor(WHITE);
//  display.print("0x"); display.println(0xDEADBEEF, HEX);
  display.display();

}


