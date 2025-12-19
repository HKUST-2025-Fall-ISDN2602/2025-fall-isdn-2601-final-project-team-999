

# Robot Arm — Xbox Controller to Servo Bridge

This project connects an Xbox-compatible controller to a multi‑servo robot arm via an Arduino-compatible board. The Python program reads joystick, trigger, and button inputs and streams them over serial to an Arduino sketch that maps the inputs to five servos.

**Repository:**
- **Controller reader:** [controller_reader.py](controller_reader.py)
- **Arduino sketch:** [Robot_arm/Robot_arm.ino](Robot_arm/Robot_arm.ino)

**Key features:**
- **Real-time control:** Reads Xbox controller axes, triggers and buttons and sends frames over serial.
- **Mapping to servos:** Joysticks control position axes; triggers and A/B buttons control additional servos.
- **Simple protocol:** Comma-separated ASCII frames: LeftX,LeftY,RightX,RightY,LT,RT,A,B,X,Y (terminated with newline).

**Hardware (example):**
- A microcontroller board with 5 PWM-capable servo outputs (the sketch attaches 5 Servo objects).
- One Xbox-compatible controller (USB or supported gamepad) connected to the computer running the Python script.
- 5 hobby servos mounted to the robot arm and wired to the pins configured in the sketch.

**Wiring / Pins** (as configured in the sketch):
- Servo 0 → pin 16
- Servo 1 → pin 5
- Servo 2 → pin 4
- Servo 3 → pin 0
- Servo 4 → pin 14

Adjust these pins in [Robot_arm/Robot_arm.ino](Robot_arm/Robot_arm.ino) if your board uses different PWM pins.

**Software requirements:**
- Python 3.x
- pygame (for joystick input)
- pyserial (for serial communication)
- Arduino core supporting the Servo library

Install Python deps, for example:

```bash
pip install pygame pyserial
```

**Configuration:**
- In [controller_reader.py](controller_reader.py) set `SERIAL_PORT` to the OS serial device for your board (e.g., `/dev/ttyACM0`, `/dev/tty.usbmodemXXX` on macOS, or `COM#` on Windows).
- Ensure `BAUD_RATE` matches the Arduino sketch (default 115200).
- Tweak `JOYSTICK_DEAD_ZONE` in the Python script and `threshold` / `multiple` in the Arduino sketch to adjust sensitivity and angle mapping.

**How it works (overview):**
- The Python script polls the connected Xbox controller using pygame, normalizes axes and triggers, and composes a comma-separated text frame.
- Each frame is written to the serial port and (optionally) echoes Arduino responses for debugging.
- The Arduino sketch reads each line, parses the 10 parameters, maps inputs to five internal counters, converts counters to angles (using a multiplier), and writes angles to the attached servos.

**Running the system:**

1. Upload [Robot_arm/Robot_arm.ino](Robot_arm/Robot_arm.ino) to your microcontroller and open the Serial Monitor at 115200 to confirm initialization.
2. On the computer, connect your Xbox controller and verify it is recognized by `pygame`.
3. Edit `SERIAL_PORT` in [controller_reader.py](controller_reader.py) to the correct device, then run:

```bash
python controller_reader.py
```

4. Move the joysticks, press triggers and buttons — the Arduino should receive frames and drive the servos accordingly.

**Control mapping (default):**
- Left joystick X → Servo 1 (left/right)
- Left joystick Y → Servo 2 (up/down)
- Right joystick X → Servo 3 (left/right)
- Triggers (LT / RT) → Servo 0 (decrease / increase)
- A / B buttons → Servo 4 (increase / decrease)

**Tips & troubleshooting:**
- If the controller is not detected on macOS, you may need a driver or to allow input devices in system settings.
- If servos jitter, increase the dead zone in `controller_reader.py` or increase `threshold` in the sketch.
- Make sure power supply to the servos can provide enough current; use a separate 5–6V power source for servos and common ground with the microcontroller.

**Notes & customization:**
- The `multiple` constant in the sketch scales the internal counter to degrees (angle = count * multiple). Adjust to change resolution and range.
- The initial `count` array determines starting angles; set center values if you want the arm to start centered at 90°.

If you want, I can: update this README in the repo (done), add a quick wiring diagram, or create a short demo script to test the serial protocol. Which would you like next?

[![Review Assignment Due Date](https://classroom.github.com/assets/deadline-readme-button-22041afd0340ce965d47ae6ef1cefeee28c7c493a6346c4f15d667ab976d596c.svg)](https://classroom.github.com/a/M9aW25uf)