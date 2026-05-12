#include "config.h"
#include "DHTesp.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

struct SystemData {
  float temperature;
  float humidity;

  int adcValue;
  int potPercent;

  int lightValue;
  bool nightMode;

  String systemState;
  String fanSpeed;
};

DHTesp dhtSensor;
SystemData systemData;

SemaphoreHandle_t dataMutex;

// ======================================================
// MACHINE STATE VARIABLES
// ======================================================

enum MachineState {
  MACHINE_OFF,
  WELCOME_SCREEN,
  AUTO_RUNNING
};

volatile MachineState machineState = MACHINE_OFF;

// ======================================================
// EMERGENCY OVERRIDE VARIABLES
// ======================================================

volatile bool emergencyOverride = false;
volatile unsigned long lastInterruptTime = 0;

// ======================================================
// Turn everything off until button is pressed
// ======================================================

void allOutputsOff() {
  digitalWrite(GREEN_PIN, LOW);
  digitalWrite(BLUE_PIN, LOW);
  digitalWrite(RED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}

// ======================================================
// Button check function
// ======================================================

bool startButtonPressed() {
  static bool lastState = HIGH;
  static unsigned long lastDebounce = 0;

  bool currentState = digitalRead(START_BUTTON_PIN);

  if (lastState == HIGH && currentState == LOW && millis() - lastDebounce > 300) {
    lastDebounce = millis();
    lastState = currentState;
    return true;
  }

  lastState = currentState;
  return false;
}

// ======================================================
// Add welcome animation
// ======================================================

void showWelcomeAnimation() {
  display.clearDisplay();

  display.setCursor(2, -6);
  display.println(" __________________");
  display.setCursor(0, 0);
  display.println("+-------------------+");

  display.setTextSize(2);
  display.setCursor(60, 2);
  display.println("o");
  display.setCursor(50, 12);
  display.println("o");
  display.setCursor(60, 16);
  display.println("*");
  display.setCursor(70, 12);
  display.println("o");
  display.setCursor(60, 22);
  display.println("o");

  display.setTextSize(2);
  display.setCursor(34, 35);
  display.println("G");

  display.setTextSize(1);
  display.setCursor(46, 38);
  display.println("reenhouse");
  display.setTextSize(0);
  display.setCursor(46, 46);
  display.println("Controller");
  
  display.setTextSize(1);
  display.setCursor(0, 55);
  display.println("+-------------------+");
  display.setCursor(2, 55);
  display.println(" __________________");

  display.display();
  delay(2000);

  display.clearDisplay();
  // =========================================
  // LOADING ANIMATION
  // =========================================

  display.clearDisplay();

  display.setTextSize(1);
  display.setCursor(18, 40);
  display.print("Starting...");

  // Loading bar outline
  display.drawRect(10, 50, 100, 8, WHITE);

  display.display();

  delay(500);

  // =========================================
  // 0%
  // =========================================

  // Clear flower area
  display.fillRect(45, 0, 45, 35, BLACK);

  // Flower frame 0
  display.setTextSize(2);

  display.setCursor(60, 14);
  display.print("*");


  // Percentage
  display.fillRect(85, 39, 40, 10, BLACK);

  display.setTextSize(1);
  display.setCursor(92, 40);
  display.print("0%");

  display.display();

  delay(500);

  // =========================================
  // 25%
  // =========================================

  // Clear flower area
  display.fillRect(45, 0, 45, 35, BLACK);

  // Flower frame 1
  display.setTextSize(2);

  display.setCursor(60, 10);
  display.print("o");

  display.setCursor(60, 10);
  display.print("o");

  display.setCursor(60, 14);
  display.print("*");

  display.setCursor(60, 10);
  display.print("o");

  display.setCursor(60, 10);
  display.print("o");

  // Loading bar
  display.fillRect(10, 50, 25, 8, WHITE);

  // Percentage
  display.fillRect(85, 39, 40, 10, BLACK);

  display.setTextSize(1);
  display.setCursor(88, 40);
  display.print("25%");

  display.display();

  delay(500);

  // =========================================
  // 50%
  // =========================================

  // Clear flower area
  display.fillRect(45, 0, 45, 35, BLACK);

  // Flower frame 2
  display.setTextSize(2);

  display.setCursor(60, 7);
  display.print("o");

  display.setCursor(57, 10);
  display.print("o");

  display.setCursor(60, 14);
  display.print("*");

  display.setCursor(63, 10);
  display.print("o");

  display.setCursor(60, 13);
  display.print("o");

  // Loading bar
  display.fillRect(10, 50, 50, 8, WHITE);

  // Percentage
  display.fillRect(85, 39, 40, 10, BLACK);

  display.setTextSize(1);
  display.setCursor(88, 40);
  display.print("50%");

  display.display();

  delay(500);

  // =========================================
  // 75%
  // =========================================

  // Clear flower area
  display.fillRect(45, 0, 45, 35, BLACK);

  // Flower frame 3
  display.setTextSize(2);

  display.setTextSize(2);

  display.setCursor(60, 3);
  display.print("o");

  display.setCursor(53, 10);
  display.print("o");

  display.setCursor(60, 14);
  display.print("*");

  display.setCursor(67, 10);
  display.print("o");

  display.setCursor(60, 17);
  display.print("o");

  // Loading bar
  display.fillRect(10, 50, 75, 8, WHITE);

  // Percentage
  display.fillRect(85, 39, 40, 10, BLACK);

  display.setTextSize(1);
  display.setCursor(88, 40);
  display.print("75%");

  display.display();

  delay(500);

  // =========================================
  // 100%
  // =========================================

  // Clear flower area
  display.fillRect(45, 0, 45, 35, BLACK);

  // Final flower 4
  display.setTextSize(2);

  display.setTextSize(2);

  display.setCursor(60, 0);
  display.print("o");

  display.setCursor(50, 10);
  display.print("o");

  display.setCursor(60, 14);
  display.print("*");

  display.setCursor(70, 10);
  display.print("o");

  display.setCursor(60, 20);
  display.print("o");

  // Loading bar
  display.fillRect(10, 50, 100, 8, WHITE);

  // Percentage
  display.fillRect(85, 39, 40, 10, BLACK);

  display.setTextSize(1);
  display.setCursor(84, 40);
  display.print("100%");

  display.display();

  delay(1000);

  // =========================================

}

// ======================================================
// MACHINE TASK
// ======================================================

void machineTask(void *parameter) {
  while (true) {

    if (machineState == MACHINE_OFF) {
      allOutputsOff();

        display.clearDisplay();
        display.setCursor(0, -6);
        display.println(" ___________________ ");
        display.setCursor(28, 15);
        display.println("<System OFF>");
        display.setCursor(10, 30);
        display.println("Press GREEN Button");
        display.setCursor(40, 40);
        display.println("to start");
        display.setCursor(0, 55);
        display.println("_____________________");

      if (startButtonPressed()) {
        machineState = WELCOME_SCREEN;
      }
    }

    else if (machineState == WELCOME_SCREEN) {
      showWelcomeAnimation();
      machineState = AUTO_RUNNING;
    }

    else if (machineState == AUTO_RUNNING) {
      if (startButtonPressed()) {
        machineState = MACHINE_OFF;
      }
    }

    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

void IRAM_ATTR emergencyISR() {

  unsigned long currentTime = millis();

  if (currentTime - lastInterruptTime > 300) {
    emergencyOverride = !emergencyOverride;
    lastInterruptTime = currentTime;
  }
}

void sensorTask(void *parameter) {

  while (true) {

    TempAndHumidity data = dhtSensor.getTempAndHumidity();

    if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
      systemData.temperature = data.temperature;
      systemData.humidity = data.humidity;
      xSemaphoreGive(dataMutex);
    }

    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void adcTask(void *parameter) {

  while (true) {

    int total = 0;
    int sampleCount = 10;

    for (int i = 0; i < sampleCount; i++) {
      total += analogRead(POT_PIN);
      vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    int rawValue = total / sampleCount;
    int percentValue = map(rawValue, 0, 4095, 0, 100);

    if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
      systemData.adcValue = rawValue;
      systemData.potPercent = percentValue;
      xSemaphoreGive(dataMutex);
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void lightTask(void *parameter) {

  while (true) {

    int lightValue = analogRead(LDR_PIN);
    bool isNight = lightValue < 1500;

    if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
      systemData.lightValue = lightValue;
      systemData.nightMode = isNight;
      xSemaphoreGive(dataMutex);
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void controlTask(void *parameter) {

  while (true) {

    if (machineState == MACHINE_OFF || machineState == WELCOME_SCREEN) {
      allOutputsOff();
      vTaskDelay(200 / portTICK_PERIOD_MS);
      continue;
    }

    float temp;
    bool nightMode;

    if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
      temp = systemData.temperature;
      nightMode = systemData.nightMode;
      xSemaphoreGive(dataMutex);
    }

    if (emergencyOverride) {

      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(BLUE_PIN, LOW);
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(BUZZER_PIN, HIGH);

      if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
        systemData.systemState = "EMERGENCY";
        systemData.fanSpeed = "FAIL-SAFE";
        xSemaphoreGive(dataMutex);
      }

      vTaskDelay(500 / portTICK_PERIOD_MS);
      continue;
    }

    if (isnan(temp) || temp < -10 || temp >= 80) {

      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(BLUE_PIN, LOW);
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(BUZZER_PIN, HIGH);

      if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
        systemData.systemState = "SENSOR ERROR";
        systemData.fanSpeed = "SHUTDOWN";
        xSemaphoreGive(dataMutex);
      }

      vTaskDelay(500 / portTICK_PERIOD_MS);
      continue;
    }

    if (temp < 28) {

      digitalWrite(GREEN_PIN, HIGH);
      digitalWrite(BLUE_PIN, LOW);
      digitalWrite(RED_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);

      if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
        systemData.systemState = "SAFE";

        if (nightMode) {
          systemData.fanSpeed = "ECO";
        } else {
          systemData.fanSpeed = "LOW";
        }

        xSemaphoreGive(dataMutex);
      }
    }

    else if (temp >= 28 && temp < 35) {

      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(BLUE_PIN, HIGH);
      digitalWrite(RED_PIN, LOW);
      digitalWrite(BUZZER_PIN, LOW);

      if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
        systemData.systemState = "WARNING";

        if (nightMode) {
          systemData.fanSpeed = "ECO-MED";
        } else {
          systemData.fanSpeed = "MEDIUM";
        }

        xSemaphoreGive(dataMutex);
      }
    }

    else {

      digitalWrite(GREEN_PIN, LOW);
      digitalWrite(BLUE_PIN, LOW);
      digitalWrite(RED_PIN, HIGH);
      digitalWrite(BUZZER_PIN, HIGH);

      if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {
        systemData.systemState = "DANGER";
        systemData.fanSpeed = "HIGH";
        xSemaphoreGive(dataMutex);
      }
    }

    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void loggingTask(void *parameter) {

  while (true) {

    if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {

      Serial.print("Temp: ");
      Serial.print(systemData.temperature);

      Serial.print(" C | Humidity: ");
      Serial.print(systemData.humidity);

      Serial.print(" % | ADC: ");
      Serial.print(systemData.adcValue);

      Serial.print(" | Pot: ");
      Serial.print(systemData.potPercent);

      Serial.print(" % | Light: ");
      Serial.print(systemData.lightValue);

      Serial.print(" | Mode: ");
      Serial.print(systemData.nightMode ? "Night" : "Day");

      Serial.print(" | State: ");
      Serial.print(systemData.systemState);

      Serial.print(" | Fan: ");
      Serial.println(systemData.fanSpeed);

      xSemaphoreGive(dataMutex);
    }

    vTaskDelay(2000 / portTICK_PERIOD_MS);
  }
}

void displayTask(void *parameter) {

  while (true) {

    if (machineState == AUTO_RUNNING) {

      if (xSemaphoreTake(dataMutex, portMAX_DELAY)) {

        display.clearDisplay();

        display.setCursor(0, 0);

        display.setCursor(0, -6);
        display.println("_____________________");

        display.setCursor(11, -3);
        display.println(" _______________ ");
        display.setCursor(12, -3);
        display.println(" _______________ ");
        display.setCursor(18, 5);
        display.println("Greenhouse HVAC");
        display.setCursor(11, 7);
        display.println(" _______________ ");
        display.setCursor(12, 7);
        display.println(" _______________ ");

        display.print("Temp : ");
        display.print(systemData.temperature, 1);
        display.println(" C");

        display.print("Hum  : ");
        display.print(systemData.humidity, 1);
        display.println(" %");

        display.print("State: ");
        display.println(systemData.systemState);

        display.print("Fan  : ");
        display.println(systemData.fanSpeed);

        display.print("Mode : ");
        display.println(systemData.nightMode ? "Night" : "Day");

        display.setCursor(0, 55);
        display.println("_____________________");

        display.display();

        xSemaphoreGive(dataMutex);
      }
    }

    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void setup() {

  Serial.begin(115200);

  analogReadResolution(12);

  pinMode(POT_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);

  pinMode(BUZZER_PIN, OUTPUT);

  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(START_BUTTON_PIN, INPUT_PULLUP);

  attachInterrupt(
    digitalPinToInterrupt(BUTTON_PIN),
    emergencyISR,
    FALLING
  );

  dhtSensor.setup(DHT_PIN, DHTesp::DHT22);

  dataMutex = xSemaphoreCreateMutex();

  systemData.temperature = 0;
  systemData.humidity = 0;

  systemData.adcValue = 0;
  systemData.potPercent = 0;

  systemData.lightValue = 0;
  systemData.nightMode = false;

  systemData.systemState = "OFF";
  systemData.fanSpeed = "OFF";

  Wire.begin(21, 22);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED failed");
    for (;;);
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);

  allOutputsOff();

  Serial.println("Industrial IoT Smart Greenhouse HVAC Controller");
  Serial.println("FreeRTOS multitasking system started");

  xTaskCreate(sensorTask, "Sensor Task", 4096, NULL, 2, NULL);
  xTaskCreate(adcTask, "ADC Task", 2048, NULL, 2, NULL);
  xTaskCreate(lightTask, "Light Task", 2048, NULL, 1, NULL);
  xTaskCreate(controlTask, "Control Task", 2048, NULL, 3, NULL);
  xTaskCreate(loggingTask, "Logging Task", 4096, NULL, 1, NULL);
  xTaskCreate(displayTask, "Display Task", 4096, NULL, 1, NULL);
  xTaskCreate(machineTask, "Machine Task", 4096, NULL, 4, NULL);
}

void loop() {

}
