#include <avr/interrupt.h>
#include <avr/sleep.h>

// Used pins
#define LED_PIN       LED_BUILTIN
#define TOUCH_PIN  2
#define DEBOUNCE_MS 200
#define DIMM_DEBOUNCE_MS 1000
#define DIMMING_STEP_MS 100

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
volatile commands previous_gear = LED_OFF;

volatile dim_direction dim = DOWN;
volatile long brightness = 0xFF;

void loop()
{
  Serial.println("-------------BOOT UP-----------");
  
  while(1){
    switch(current_gear)
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
  Serial.begin(9600);
  pinMode(led1,OUTPUT);
  pinMode(TOUCH_PIN, INPUT_PULLUP);
  delay(100);
  enable_touch_interrupt();
}

void switch_led_on(){
  digitalWrite(led1,HIGH);
  previous_gear = LED_ON;
  Serial.println("-------------DO NOTHING--------");
  current_gear = DO_NOTHING;
  enable_touch_interrupt();
}

void switch_led_off(){
  digitalWrite(led1,LOW);
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
  if (digitalRead(TOUCH_PIN) == 0){
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
  while(digitalRead(TOUCH_PIN) == 0) {
    if (dim == DOWN) {
      brightness = brightness - 5;
      if (brightness == 0) {
        dim = UP;
      }
    } else if (dim == UP){
        brightness = brightness + 5;
        if (brightness == 255){
        dim = DOWN;
      }
    }
  Serial.println(brightness);
  delay(DIMMING_STEP_MS);
  //TODO: send brightness
  }
  if (dim == UP){
    dim = DOWN;
  } else if (dim == DOWN) {
    dim = UP;
  }
  current_gear = DO_NOTHING;
  enable_touch_interrupt();
}

void sleep_now()         // here we put the arduino to sleep
{
    Serial.println("-------------SLEEP NOW---------");
    delay(100);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here
    sleep_enable();          
    sleep_mode();
    sleep_disable();         
    Serial.println("-------------WAKING UP---------");
}

void enable_touch_interrupt (){
  EIFR = bit (INTF0);
  attachInterrupt(digitalPinToInterrupt(TOUCH_PIN),touched,FALLING);
}
