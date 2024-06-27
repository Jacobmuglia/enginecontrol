const int hallSensorPin = 8; // Hall sensor input pin
const int ignPin = 4;        // Ignition control output pin
const float RPM_LIMIT = 9000.0;
const int degreesAdvanced = 5;
const int smoothingWindowSize = 5; // Number of revolutions to average for smoothing

bool firstFrame = false;
unsigned long firstTimestamp = 0;
unsigned long secondTimestamp = 0;
const float revTimeToRPM = 60000000.0;  //(60seconds/min)*(1MM nanoseconds/second)
float RPM = 0;
float revTime = 0;
unsigned long revTimes[smoothingWindowSize] = {0};
unsigned long revTimeSum = 0;
unsigned long averageRevTime = 0;
unsigned long degreesConversion = 0;
unsigned long finalAdvanceTime = 0;
unsigned long fireTime = 0;
unsigned long lastFireTime = 0;

void setup() {
  pinMode(ignPin, OUTPUT);
  pinMode(hallSensorPin, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  int sensorState = digitalRead(hallSensorPin);

  // Updates every time propeller revolves
  if (sensorState == LOW) {
    if (firstFrame) {
      firstFrame = false;
      secondTimestamp = micros();
      revTime = secondTimestamp - firstTimestamp;

      if (revTime > 0) {
        RPM = (1.0 / revTime) * revTimeToRPM;
      }

      firstTimestamp = micros();
    }
  } else {
    firstFrame = true;
    digitalWrite(ignPin, HIGH); // Ensure ignition is not blocked by default
  }

  // Updates at system refresh rate 16MHz
  updateRevTimeArray(revTime); // Adds current RPM to RPM array, pops current last value
  calculateAverageRevTime(); // RevTimeArray / RevTimeArray.length
  calculateFinalAdvance(); // Returns absolute time ignition should be advanced
  fireTime = micros() + finalAdvanceTime; // Returns current time + absolute time ignition should be advanced
                                          // Not used right now
  controlIgnition(RPM);
} // end main loop()

void updateRevTimeArray(unsigned long newRevTime) {
  revTimeSum -= revTimes[0];
  for (int i = 0; i < smoothingWindowSize - 1; i++) {
    revTimes[i] = revTimes[i + 1];
  }
  revTimes[smoothingWindowSize - 1] = newRevTime;
  revTimeSum += newRevTime;
}

void calculateAverageRevTime() {
  averageRevTime = revTimeSum / smoothingWindowSize;
}

void calculateFinalAdvance() {
  degreesConversion = averageRevTime / 360;
  finalAdvanceTime = averageRevTime - (degreesConversion * degreesAdvanced);
}

void controlIgnition(float revolutions) {
  if (revolutions < 2000) {
    digitalWrite(ignPin, LOW);
    lastFireTime = micros();
    Serial.println("We are in the LESS than 2000 RPM range");
  } else if (revolutions >= 2000 && revolutions < RPM_LIMIT) {
    if (lastFireTime + finalAdvanceTime < micros()) {
      digitalWrite(ignPin, LOW);
      lastFireTime = micros();
      Serial.println("We are in the GREATER than 2000 RPM range");
    } else {
      digitalWrite(ignPin, HIGH);
    }
  }
}

