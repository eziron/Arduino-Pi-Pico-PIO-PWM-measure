#include "PwmIn.h"

PwmIn PWM_test;

void setup() {
  Serial.begin(115200);

  PWM_test.attach(2);
}

void loop() {
  if (PWM_test.available()) {
    
    PWM_test.update();

    Serial.print(PWM_test.get_period());
    Serial.print(" - ");
    Serial.print(PWM_test.get_pulsewidth());
    Serial.print(" - ");
    Serial.println(PWM_test.get_dutycycle());
  }
}
