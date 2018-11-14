
#include <avr/interrupt.h>
#include <avr/power.h>
#include <avr/sleep.h>

// Used pins
#define LED_PIN       LED_BUILTIN
#define TOUCH_PIN  2
#define DEBOUNCE_MS 200
#define DIMM_DEBOUNCE_MS 1000
#define DIMMING_STEP_MS 100
#define LED_DIM   9
#define MIN_BRIGHTNESS  0
#define MAX_BRIGHTNESS 255
#define NUM_LEDS  1
#define LED_DATA 12
#define LED_CLK 13

enum commands {
  DO_NOTHING = 0,
  LED_OFF = 1,
  LED_ON = 2,
  BUTTON_PRESSED = 3,
  DIMM = 4
};

enum dim_direction {
  UP = 1,
  DOWN = 2
};

const int led1 = LED_PIN;
volatile commands current_gear = DO_NOTHING;
volatile commands previous_gear = LED_ON;

volatile dim_direction dim = DOWN;
volatile byte brightness = 0xFF;

void loop() {
  Serial.println("-------------BOOT UP-----------");

  while (1) {
    switch (current_gear)
    {
      case DO_NOTHING:
        sleep_now();
        break;
      case LED_ON:
        switch_led_on();
        break;
      case LED_OFF:
        switch_led_off();
        break;
      case BUTTON_PRESSED:
        checking_button();
        break;
      case DIMM:
        dimming_led();
        break;
    }
  }
}
void setup()
{
  LED_strip_init();
  Serial.begin(9600);
  pinMode(led1, OUTPUT);
  pinMode(TOUCH_PIN, INPUT_PULLUP);
  delay(100);
  enable_touch_interrupt();
}

void LED_strip_init() {
  pinMode(LED_DATA, OUTPUT);
  pinMode(LED_CLK, OUTPUT);
  send_RGB(MAX_BRIGHTNESS);  
}

void send_RGB (byte bright){
  for (byte b = 0; b < 4; b++) {
    sendByte(0x00);
  }
  sendByte(0xFF);
  sendByte(bright);
  sendByte(bright);
  sendByte(bright);
  for (byte b = 0; b < 4; b++) {
    sendByte(0x00);
  }
  delay(200);
}

void switch_led_on() {
  digitalWrite(led1, HIGH);
  writeLED(brightness);
  previous_gear = LED_ON;
  Serial.println("-------------DO NOTHING--------");
  current_gear = DO_NOTHING;
  enable_touch_interrupt();
}

void switch_led_off() {
  digitalWrite(led1, LOW);
  writeLED(MIN_BRIGHTNESS);
  previous_gear = LED_OFF;
  Serial.println("-------------DO NOTHING--------");
  current_gear = DO_NOTHING;
  enable_touch_interrupt();
}

void touched()
{
  detachInterrupt(digitalPinToInterrupt(TOUCH_PIN));
  Serial.println("-------------TOUCH-------------");
  current_gear = BUTTON_PRESSED;
}

void checking_button() {
  Serial.println("-------------CHECKING----------");
  delay(DEBOUNCE_MS);
  if (digitalRead(TOUCH_PIN) == 0) {
    delay(DIMM_DEBOUNCE_MS);
    if (digitalRead(TOUCH_PIN) == 0) {
      current_gear = DIMM;
    } else {
      if (previous_gear == LED_ON) {
        Serial.println("-------------LED OFF-----------");
        current_gear = LED_OFF;
      } else {
        Serial.println("-------------LED ON------------");
        current_gear = LED_ON;
      }
    }
  } else {
    Serial.println("-------------DO NOTHING--------");
    current_gear = DO_NOTHING;
    enable_touch_interrupt();
  }
}

void dimming_led () {
  Serial.println("Dimming");
  if (previous_gear == LED_ON) {
    while (digitalRead(TOUCH_PIN) == 0) {
      if (dim == DOWN) {
        brightness = brightness - 5;
        if (brightness == MIN_BRIGHTNESS) {
          dim = UP;
        }
      } else if (dim == UP) {
        brightness = brightness + 5;
        if (brightness == MAX_BRIGHTNESS) {
          dim = DOWN;
        }
      }
      Serial.println(brightness);
      writeLED(brightness);
      delay(DIMMING_STEP_MS);
    }
    if (dim == UP) {
      dim = DOWN;
    } else if (dim == DOWN) {
      dim = UP;
    }
  }
  current_gear = DO_NOTHING;
  enable_touch_interrupt();
}

void writeLED(long bright) {
  Serial.println(bright);
  analogWrite(LED_DIM, bright);
  send_RGB(bright);
}

void sleep_now()         // here we put the arduino to sleep
{
  Serial.println("-------------SLEEP NOW---------");
  delay(100);
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here
  sleep_enable();
  power_timer2_enable();
  sleep_mode();
  sleep_disable();
  Serial.println("-------------WAKING UP---------");
}

void sendByte(byte b)
{
  // Send one bit at a time, starting with the MSB
  for (byte i = 0; i < 8; i++)
  {
    // If MSB is 1, write one and clock it, else write 0 and clock
    if ((b & 0x80) != 0)
      digitalWrite(LED_DATA, HIGH);
    else
      digitalWrite(LED_DATA, LOW);
    clk();

    // Advance to the next bit to send
    b <<= 1;
  }
}

void clk(void)
{
  digitalWrite(LED_CLK, LOW);
  delayMicroseconds(20);
  digitalWrite(LED_CLK, HIGH);
  delayMicroseconds(20);
}

void enable_touch_interrupt () {
  EIFR = bit (INTF0);
  attachInterrupt(digitalPinToInterrupt(TOUCH_PIN), touched, FALLING);
}
