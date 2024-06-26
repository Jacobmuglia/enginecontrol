const int hallSensorPin = 8; // Hall sensor input pin
const int ignPin = 4;        // Ignition control output pin
const float RPM_LIMIT = 9000.0;
const int degreesAdvanced = 15;
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

void setup() {
  pinMode(ignPin, OUTPUT);
  pinMode(hallSensorPin, INPUT_PULLUP);
  Serial.begin(9600);
}

void loop() {
  int sensorState = digitalRead(hallSensorPin);

  if (sensorState == LOW) {
    if (firstFrame) {
      firstFrame = false;
      secondTimestamp = micros();
      revTime = secondTimestamp - firstTimestamp;

      if (revTime > 0) {
        RPM = (1.0 / revTime) * revTimeToRPM;
      }

      firstTimestamp = micros();

      updateRevTimeArray(revTime);
      calculateAverageRevTime();
      calculateFinalAdvance();

      fireTime = micros() + finalAdvanceTime;

      Serial.print("Avg Last 5: ");
      Serial.println(averageRevTime);
      Serial.print("RPM: ");
      Serial.println(RPM);
      Serial.print("Final Advance: ");
      Serial.println(finalAdvanceTime);
      Serial.print("fireTime: ");
      Serial.println(fireTime);

      controlIgnition();
    } else {
      firstFrame = true;
      digitalWrite(ignPin, HIGH); // Ensure ignition is not blocked by default
    }
  }
}

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

void controlIgnition() {
  if (RPM < 2000) {
    digitalWrite(ignPin, LOW);
    Serial.println("We are in the LESS than 2000 RPM range");
  } else{
    Serial.println("We are in the GREATER than 2000 RPM range");
    if (micros() >= fireTime) {
      digitalWrite(ignPin, LOW);
      Serial.println("Setting ignPin LOW");
    } else {
      digitalWrite(ignPin, HIGH);
      Serial.println("Setting ignPin HIGH");
    }
  }
}