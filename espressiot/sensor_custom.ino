#ifndef SIMULATION_MODE
#ifdef TEMP_SENSOR_CUSTOM



#define THERMISTOR_PIN A0
#define THERMISTOR_R25 40000
#define THERMISTOR_BETA 3686
//#define THERMISTOR_SIERIES_RESISTOR 3300
//#define THERMISTOR_SIERIES_RESISTOR 3150
#define THERMISTOR_SIERIES_RESISTOR 3000
#define THERMISTOR_SAMPLES 5


// FROM https://www.circuitbasics.com/arduino-thermistor-temperature-sensor-tutorial/

float lastT = 0.0;

int Vor;
float R1 = 3300;
float Vo, logR2, logR2kOhm, R2, R2kOhm, T, Tc;
//float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
float alpha = 43000;
float beta = 3761.88;

// how many samples to take and average, more takes longer
// but is more 'smooth'
#define NUMSAMPLES 5


int thermistor_samples[THERMISTOR_SAMPLES];

float updateTempSensor() {
  uint8_t i;
  float average;

  // take N samples in a row, with a slight delay
  for (i=0; i< THERMISTOR_SAMPLES; i++) {
   thermistor_samples[i] = analogRead(THERMISTOR_PIN);
   delay(10);
  }
  // average all the samples out
  average = 0;
  for (i=0; i< NUMSAMPLES; i++) {
     average += thermistor_samples[i];
  }
  average /= NUMSAMPLES;

  //Serial.print("Average analog reading "); 
  //Serial.println(average);
  
  // convert the value to resistance
  average = 1023 / average - 1;
  average = THERMISTOR_SIERIES_RESISTOR / average;
  //Serial.print("Thermistor resistance "); 
  //Serial.println(average);

  //average = 4300;
  
  float steinhart;
  steinhart = average / THERMISTOR_R25;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= THERMISTOR_BETA;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (25 + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;                         // convert absolute temp to C
  
  //Serial.print("Temperature "); 
  //Serial.print(steinhart);
  //Serial.println(" *C");

  lastT = steinhart;
  return steinhart;
}

float getTemp() {
  return lastT;
}

void setupSensor() {
  pinMode(THERMISTOR_PIN, INPUT);
}

#endif
#endif
