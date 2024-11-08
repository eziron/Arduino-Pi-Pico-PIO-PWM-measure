#include "PwmIn.h"

#define PWM_channels 7

PwmIn PWM_IN[PWM_channels];
int PWM_PIN[PWM_channels] = { 2, 15, 14, 13, 18, 17, 16 };

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Iniciando");

  for (int i = 0; i < PWM_channels; i++) {
    if (PWM_IN[i].attach(PWM_PIN[i])) {
      Serial.print("PWM ");
      Serial.print(i);
      Serial.println(": True");
    } else {
      Serial.print("PWM ");
      Serial.print(i);
      Serial.println(": False");
    }
  }
  delay(1000);
}

void loop() {
  bool all_available = true;
  for (int i = 0; i < PWM_channels; i++) {
    if (!PWM_IN[i].available()) {
      all_available = false;
    }
  }

  if (all_available) {

    for (int i = 0; i < PWM_channels; i++) {
      PWM_IN[i].update();

      Serial.print(PWM_IN[i].get_pulsewidth());
      Serial.print(" - ");
      Serial.print(PWM_IN[i].get_dutycycle());

      if (i < PWM_channels - 1)
        Serial.print(" -/- ");
    }
    Serial.println("");
  }
}
