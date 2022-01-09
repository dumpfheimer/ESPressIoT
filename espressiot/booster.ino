
// 1L Water takes 1Wh to be heated 1°C
// 1 double espresso = 17g

float boost_cold_water_temperature = 25;
float boost_coffee_g = 17;
float boost_ml = boost_coffee_g * 2.3;
float boost_energy_wh = boost_ml / 1000;
float boost_energy_ws = boost_energy_wh * 3600;
float boost_heater_w = 1000;
float boost_time = 25; // s

float boost_w_per_deg = boost_energy_ws / boost_time;
float boost_percent_per_deg = boost_w_per_deg / boost_heater_w * 100;

float boost_deg = gTargetTemp - boost_cold_water_temperature;
float boost_percent = boost_deg * boost_percent_per_deg;

uint16_t boost_loops_per_s = 2;

unsigned long boost_last_loop = 0;

bool boost_enabled = false;
bool boost_skip = false;
unsigned long boost_begin = 0;

#define BOOSTER_STACK_SIZE 30
float temp_log[BOOSTER_STACK_SIZE];
int last_booster_stack_pointer = BOOSTER_STACK_SIZE - 1;

void setupBooster() {
  Serial.println("Boost Deg: ");
  Serial.println(boost_deg);
  Serial.println("Boost ml: ");
  Serial.println(boost_ml);
  Serial.println("Boost Watt/C: ");
  Serial.println(boost_w_per_deg);
  Serial.println("Boost %/C: ");
  Serial.println(boost_percent_per_deg);
  Serial.println("Boost %: ");
  Serial.println(boost_percent);

  for (int i = 0; i < BOOSTER_STACK_SIZE; i++) temp_log[i] = 0;
  boost_last_loop = 0;
}

int advancePointer() {
  last_booster_stack_pointer = (last_booster_stack_pointer + 1) % BOOSTER_STACK_SIZE;
  return last_booster_stack_pointer;
}

float getHistoricalTemp(int nth) {
  // 0 = current
  int pointer = last_booster_stack_pointer;
  if (pointer < nth) pointer += BOOSTER_STACK_SIZE;
  return temp_log[pointer - nth];
}

float getHistoricalTempNsecAgo(int n) {
  return getHistoricalTemp(n * boost_loops_per_s);
}

void pushTemp(float temp) {
  temp_log[advancePointer()] = temp;
}

void logLastNTemps(int n) {
  for (int i = 0; i < n; i++) {
    Serial.print("Historical Temp ");
    Serial.print(i);
    Serial.print("=");
    Serial.println(getHistoricalTemp(i));
  }
}

void loopBoosterOncePerSecond(float currentTemp) {
  //Serial.println("booster loop");

  unsigned long boost_interval = (1000 / boost_loops_per_s);
  unsigned long m = millis() - boost_interval;
  while (boost_last_loop <= m) {
    boost_last_loop += boost_interval;
  }
  pushTemp(currentTemp);
  //logLastNTemps(10);
  
  boost_skip = false;

  if (!boost_enabled) {
    // check if temperature is falling rapidly (>= 0.2°C in <= 2s) 
    if (getHistoricalTempNsecAgo(3) >= currentTemp + 0.3) {
      // check if temperature is within extrcation temp
      if (currentTemp <= gTargetTemp + 1) {
        // looks like we should boost!
        boost_enabled = true;
        boost_begin = millis();
        Serial.println("Starting boost");
      }
    }
  } else {
    // check if boost should stop
    if (millis() - boost_begin >= boost_time * 1000) {
      // timed out
      Serial.println("Stopping boost due to timeout");
      boost_enabled = false;
    }
    // if its getting warmer, pause
    if (getHistoricalTemp(1) < currentTemp) {
      boost_skip = true;
      Serial.println("Skipping boost due to timeout");
    }
    
    if (currentTemp > gTargetTemp) {
      boost_skip = true;
      Serial.println("Skipping boost due to required temperature");
    }
  }
}

void loopBooster(float currentTemp) {
  if (millis() - (1000 / boost_loops_per_s) >= boost_last_loop) {
    loopBoosterOncePerSecond(currentTemp);
  }
  
  if (boost_enabled) {
    boost_deg = gTargetTemp - boost_cold_water_temperature;
    boost_percent = boost_deg * boost_percent_per_deg;
    float boost_output_power = boost_percent * 10;
    gOutputPwr += boost_output_power;
  }
}
