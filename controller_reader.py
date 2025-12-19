import pygame
import serial
import time

# -------------------------- Settings --------------------------
SERIAL_PORT = "COM6"  # Find your Arduino's serial port (e.g., "COM3" on Windows or "/dev/ttyACM0" on Linux)
BAUD_RATE = 115200    # Baud rate must match the setting on your Arduino
JOYSTICK_DEAD_ZONE = 500  # Threshold for joystick dead zone (small values near center are set to 0)
# --------------------------------------------------------------------------

# Initialize pygame library for joystick handling
pygame.init()
pygame.joystick.init()

# Connect to Arduino via serial port
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
    time.sleep(2)  # Wait for the serial connection to initialize
    print(f"Successfully connected to: {SERIAL_PORT}")
except Exception as e:
    print(f"Connection failed, error: {e}")
    print("Please check the SERIAL_PORT and ensure the Arduino is properly connected.")
    exit()

# Detect and initialize the Xbox controller
joystick = None
for i in range(pygame.joystick.get_count()):
    joy = pygame.joystick.Joystick(i)
    joy.init()
    # Check if the detected controller is an Xbox controller
    if "Xbox" in joy.get_name() or "XBOX" in joy.get_name():
        joystick = joy
        print(f"Xbox controller found: {joy.get_name()}")
        break

if not joystick:
    print("No Xbox controller detected. Please connect one and try again. (macOS may require driver installation)")
    pygame.quit()
    exit()

# Main loop to read controller data and send to Arduino
print("Starting to read controller data and send to Arduino... (Press Ctrl+C to quit)")
try:
    while True:
        pygame.event.pump()  # Update joystick events
        
        # 1. Read joystick data (-1.0~1.0 → converted to -32768~32767)
        left_x = int(joystick.get_axis(0) * 32767)   # Left joystick X-axis (negative = left, positive = right)
        left_y = int(-joystick.get_axis(1) * 32767)  # Left joystick Y-axis (inverted: positive = up, negative = down)
        right_x = int(joystick.get_axis(2) * 32767)  # Right joystick X-axis
        right_y = int(-joystick.get_axis(3) * 32767) # Right joystick Y-axis (inverted: positive = up, negative = down)
        
        # Apply dead zone handling (set small values near center to 0)
        left_x = 0 if abs(left_x) < JOYSTICK_DEAD_ZONE else left_x
        left_y = 0 if abs(left_y) < JOYSTICK_DEAD_ZONE else left_y
        right_x = 0 if abs(right_x) < JOYSTICK_DEAD_ZONE else right_x
        right_y = 0 if abs(right_y) < JOYSTICK_DEAD_ZONE else right_y
        
        # 2. Read trigger data (0.0~1.0 → converted to 0~65535)
        lt = int((joystick.get_axis(4) + 1) / 2 * 65535)  # Left trigger (default range -1~1, mapped to 0~65535)
        rt = int((joystick.get_axis(5) + 1) / 2 * 65535)  # Right trigger
        
        # 3. Read button data (A/B/X/Y → 1 = pressed, 0 = not pressed)
        a_btn = 1 if joystick.get_button(0) else 0  # A button (button ID 0, may be consistent across most controllers)
        b_btn = 1 if joystick.get_button(1) else 0  # B button (ID 1)
        x_btn = 1 if joystick.get_button(2) else 0  # X button (ID 2)
        y_btn = 1 if joystick.get_button(3) else 0  # Y button (ID 3)
        
        # 4. Format data (comma-separated, ends with newline for Arduino parsing)
        data = f"{left_x},{left_y},{right_x},{right_y},{lt},{rt},{a_btn},{b_btn},{x_btn},{y_btn}\n"
        
        # Send data to Arduino
        ser.write(data.encode('utf-8'))
        
        # Print debug information (optional - comment out to improve performance)
        print(f"Sent: {data.strip()}")
        response = ser.readline().decode().strip()
        if response:
            print("Arduino echo:", response)
        
        time.sleep(0.01)  # Control send frequency (approx 30Hz to avoid serial port congestion)
        
except KeyboardInterrupt:
    print("\nExiting program...")
finally:
    ser.close()       # Close serial connection
    pygame.quit()     # Shutdown pygame