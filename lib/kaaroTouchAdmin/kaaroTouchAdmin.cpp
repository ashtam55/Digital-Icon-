#ifndef _kaaroTouchAdmin_h_
#define _kaaroTouchAdmin_h_

#include<Arduino.h>

const int touchPin = 4; 
int touchValue;
static int TOUCH_PINS[4];
static int touch_pin_total;
static int pins_values[4];
static int pin_state;

class kaaroTouchAdmin {
public:

  

private:
//... set of custom actions as private static functions that we need....
public:
  static int getPinState(int pin_number);
  static int setTouchPinConfig(int *arr, int touch_pin_count);
  static void loop();
// Loop has special functions
//checks all pins and creates a 0010101
// we now match with patterns and execute private functions accordingly
};


int kaaroTouchAdmin::setTouchPinConfig(int *arr, int touch_pin_count)
{
    touch_pin_total = touch_pin_count;
        for(int i = 0; i < touch_pin_count; i++){
            TOUCH_PINS[i] = arr[i];
        pins_values[i] = touchRead(TOUCH_PINS[i]);
        Serial.print(pins_values[i]);
       Serial.print(",");
    
    }
    Serial.println("");
return 1;

}

int kaaroTouchAdmin::getPinState(int pin_number){
    pin_state = touchRead(TOUCH_PINS[pin_number - 1]);
    return pin_state;
}

void kaaroTouchAdmin::loop()
{   
    
    for(int i = 0; i < touch_pin_total; i++){
        pins_values[i] = touchRead(TOUCH_PINS[i]);
    //     Serial.print(pins_values[i]);
    //    Serial.print(",");
    
    }
    // Serial.println("");
}

#endif