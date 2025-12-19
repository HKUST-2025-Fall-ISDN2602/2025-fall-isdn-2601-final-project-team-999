#include "Servo.h"

// Servo objects (one per servo motor)
Servo servo0;
Servo servo1;
Servo servo2;
Servo servo3;
Servo servo4;

// Configuration parameters (adjust as needed for your hardware)
const int threshold = 8000;    // Dead zone threshold: Joystick (8000 to avoid jitter), Trigger (20000 = firm press required)
const int multiple = 3;        // Angle multiplier (count range 0~60 → angle 0~180°, centered at 90°)
const int debounceTime = 50;   // Button debounce time (milliseconds) to filter mechanical noise

// Servo pin assignments (D0 changed to D2 to avoid serial port conflict)
const int pin0 = 16;  // PWM pin for servo0
const int pin1 = 5;   // PWM pin for servo1
const int pin2 = 4;   // PWM pin for servo2
const int pin3 = 0;   // PWM pin for servo3 (original D0 → changed to D2 to prevent UART conflict)
const int pin4 = 14;  // PWM pin for servo4

// Global variables (initialized to avoid random values on startup)
int leftX = 0;                // Left joystick X-axis value (-32768 ~ 32767)
int leftY = 0;                // Left joystick Y-axis value (-32768 ~ 32767)
int rightX = 0;               // Right joystick X-axis value (-32768 ~ 32767)
int rightY = 0;               // Right joystick Y-axis value (-32768 ~ 32767)
uint16_t lt = 0;              // Left trigger value (0 ~ 65535)
uint16_t rt = 0;              // Right trigger value (0 ~ 65535)
bool aBtn = false;            // State of A button (true = pressed, false = released)
bool bBtn = false;            // State of B button (true = pressed, false = released)
int count[5] = {0, 0, 0, 0, 0};  // Angle counter (initial = center angle 90° for all servos)
unsigned long lastBtnTime = 0;    // Timestamp for button debounce timing

void setup() {
  Serial.begin(115200);  // Initialize serial communication (must match Python baud rate)
  
  // Attach servo objects to their respective PWM pins
  // (Pulse width range: 0-2500μs for full 0-180° rotation)
  servo0.attach(pin0, 0, 2500);
  servo1.attach(pin1, 0, 2500);
  servo2.attach(pin2, 0, 2500);
  servo3.attach(pin3, 0, 2500);
  servo4.attach(pin4, 0, 2500);
  
  // Set initial servo positions (centered at 90°: count[x]=30 → 30*1 + 60 = 90°)
  servo1.write(count[0] * multiple);
  servo2.write(count[1] * multiple);
  servo3.write(count[2] * multiple);
  servo0.write(count[3] * multiple);
  servo4.write(0 + count[4] * multiple); 
  
  delay(200);  // Wait for servos to reach initial position (mechanical settling time)
  Serial.println("Servos initialized successfully, waiting for serial data...");
}

void loop() {
  // 1. Reset all input values (prevent stale data if no new serial frame is received)
  leftX = 0;
  leftY = 0;
  rightY = 0;
  rightX = 0;
  lt = 0;
  rt = 0;
  aBtn = false;
  bBtn = false;

  // Read and parse serial data (comma-separated string from Python)
  if (Serial.available() > 0) {
    String data = Serial.readStringUntil('\n');  // Read until newline (end of frame)
    data.trim();  // Remove leading/trailing whitespace/newlines
    
    // Parse 10 parameters: LeftX, LeftY, RightX, RightY, LT, RT, A, B, X, Y
    int params[10] = {0};  // Initialize parameter array to avoid garbage values
    int paramCount = 0;
    String temp = "";
    
    // Split string by commas and convert to integers
    for (int i = 0; i < data.length(); i++) {
      if (data[i] == ',') {
        if (paramCount < 10) {
          params[paramCount++] = temp.toInt();
          temp = "";
        }
      } else {
        temp += data[i];
      }
    }
    // Read the last parameter (after final comma)
    if (paramCount < 10) {
      params[paramCount++] = temp.toInt();
    }
    
    // Debug: Print raw received data
    Serial.print("Received data: ");
    Serial.println(data);
    
    // Update controller data (only when new data is received)
    leftX = params[0];
    leftY = params[1];
    rightX = params[2];
    rightY = params[3];
    lt = (uint16_t)params[4];  // Cast to unsigned 16-bit integer (0~65535)
    rt = (uint16_t)params[5];
    aBtn = (params[6] == 1);   // Ensure boolean state (1 = pressed)
    bBtn = (params[7] == 1);
  }

  // 2. Map Left Joystick X → servo1 (left/right movement, centered at 90°)
  if (abs(leftX) > threshold) {
    if (leftX < 0 && count[0] > 0) {        // Push left: decrease count → angle < 90°
      count[0]--;
    } else if (leftX > 0 && count[0] < 180/multiple) { // Push right: increase count → angle > 90°
      count[0]++;
    }
  }

  // 3. Map Left Joystick Y → servo2 (up/down movement, centered at 90°)
  if (abs(leftY) > threshold) {
    if (leftY < 0 && count[1] > 0) {        // Push up: decrease count → angle < 90°
      count[1]--;
    } else if (leftY > 0 && count[1] < 180/multiple) { // Push down: increase count → angle > 90°
      count[1]++;
    }
  }

  // 4. Map Right Joystick X → servo3 (left/right movement, centered at 90°)
  if (abs(rightX) > threshold) {
    if (rightX < 0 && count[2] > 0) {        // Push left: decrease count → angle < 90°
      count[2]--;
    } else if (rightX > 0 && count[2] < 180/multiple) { // Push right: increase count → angle > 90°
      count[2]++;
    }
  }

  // 5. Map Triggers → servo0 (LT = decrease angle, RT = increase angle)
  if (lt > 20000) {  // Left trigger pressed (threshold = 20000 = firm press)
    if (count[3] > 0) {
      count[3]--;
    }
  } 
  
  if (rt > 20000) {  // Right trigger pressed (threshold = 20000 = firm press)
    if (count[3] < 180/multiple) {
      count[3]++;
    }
  }

  // 6. Map A/B Buttons → servo4 (debounce handled implicitly by serial frame rate)
  if (aBtn && count[4] < 180/multiple) {  // A button: increase count → angle 0~180°
    count[4]++;
  }
  if (bBtn && count[4] > 0) {             // B button: decrease count → angle 0~180°
    count[4]--;
  }

  // 7. Set servo angles (constrain to 0~180° to prevent servo damage)
  int servo1Angle = constrain(count[0] * multiple, 0, 180);
  int servo2Angle = constrain(count[1] * multiple, 0, 180);
  int servo3Angle = constrain(count[2] * multiple, 0, 180);
  int servo0Angle = constrain(count[3] * multiple, 0, 180);
  int servo4Angle = constrain(count[4] * multiple, 0, 180);

  // Write calculated angles to servos
  servo1.write(servo1Angle);
  servo2.write(servo2Angle);
  servo3.write(servo3Angle);
  servo0.write(servo0Angle);
  servo4.write(servo4Angle);

  // (Optional) Debug: Echo angle data over serial (uncomment to monitor)
  // Serial.print("Count: [");
  // for (int i = 0; i < 5; i++) {
  //   Serial.print(count[i]);
  //   if (i < 4) Serial.print(",");
  // }
  // Serial.print("] → Angles: ");
  // Serial.print(servo1Angle); Serial.print(",");
  // Serial.print(servo2Angle); Serial.print(",");
  // Serial.print(servo3Angle); Serial.print(",");
  // Serial.print(servo0Angle); Serial.print(",");
  // Serial.println(servo4Angle);

  delay(3);  // Small delay to stabilize servo movement (adjust for responsiveness)
}