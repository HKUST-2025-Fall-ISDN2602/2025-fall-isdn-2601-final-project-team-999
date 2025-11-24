import pygame
import serial
import time

# -------------------------- Setting --------------------------
SERIAL_PORT = "COM3"  # Find your Arduino serial port (e.g., "COM3" on Windows or "/dev/ttyACM0" on Linux)
BAUD_RATE = 115200      # the braud rate must match the Arduino setting
JOYSTICK_DEAD_ZONE = 500  # controller joystick dead zone threshold
# --------------------------------------------------------------------------

# initialize the pygame library for joystick handling
pygame.init()
pygame.joystick.init()

# Connect to Arduino via serial
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
    time.sleep(2)  # Waiting for the serial connection to initialize
    print(f"Connected to:{SERIAL_PORT} successfully.")
except Exception as e:
    print(f"Connection failed, error {e}")
    print("Please check the SERIAL_PORT and ensure Arduino is connected.")
    exit()

# Detect and initialize the Xbox controller
joystick = None
for i in range(pygame.joystick.get_count()):
    joy = pygame.joystick.Joystick(i)
    joy.init()
    # Check if it's an Xbox controller
    if "Xbox" in joy.get_name() or "XBOX" in joy.get_name():
        joystick = joy
        print(f"Xbox controller found:{joy.get_name()}")
        break

if not joystick:
    print("No Xbox controller detected. Please connect one and try again.\(MacOs needs download driver\)")
    pygame.quit()
    exit()

# Main loop to read controller data and send to Arduino
print("Starting to read controller data and send to Arduino...(Ctrl+C to quit)")
try:
    while True:
        pygame.event.pump()  # 刷新手柄事件
        
        # 1. 读取摇杆数据（-1.0~1.0 → 转换为-32768~32767）
        left_x = int(joystick.get_axis(0) * 32767)  # 左摇杆X（左负右正）
        left_y = int(-joystick.get_axis(1) * 32767) # 左摇杆Y（上正下负，翻转符号）
        right_x = int(joystick.get_axis(2) * 32767) # 右摇杆X
        right_y = int(-joystick.get_axis(3) * 32767)# 右摇杆Y（翻转符号）
        
        # 摇杆死区处理（中心附近微小值归零）
        left_x = 0 if abs(left_x) < JOYSTICK_DEAD_ZONE else left_x
        left_y = 0 if abs(left_y) < JOYSTICK_DEAD_ZONE else left_y
        right_x = 0 if abs(right_x) < JOYSTICK_DEAD_ZONE else right_x
        right_y = 0 if abs(right_y) < JOYSTICK_DEAD_ZONE else right_y
        
        # 2. 读取扳机数据（0.0~1.0 → 转换为0~65535）
        lt = int((joystick.get_axis(4) + 1) / 2 * 65535)  # LT（默认-1~1，映射到0~65535）
        rt = int((joystick.get_axis(5) + 1) / 2 * 65535)  # RT
        
        # 3. 读取按键数据（A/B/X/Y → 1=按下，0=未按）
        a_btn = 1 if joystick.get_button(0) else 0  # A键（按键编号0，不同手柄可能一致）
        b_btn = 1 if joystick.get_button(1) else 0  # B键（编号1）
        x_btn = 1 if joystick.get_button(2) else 0  # X键（编号2）
        y_btn = 1 if joystick.get_button(3) else 0  # Y键（编号3）
        
        # 4. 格式化数据（逗号分隔，换行符结束，Arduino按此解析）
        data = f"{left_x},{left_y},{right_x},{right_y},{lt},{rt},{a_btn},{b_btn},{x_btn},{y_btn}\n"
        
        # 发送数据到Arduino
        ser.write(data.encode('utf-8'))
        
        # 打印调试信息（可选，注释后可提高效率）
        # print(f"发送：{data.strip()}")
        
        time.sleep(0.03)  # 控制发送频率（约30Hz，避免串口拥堵）
        
except KeyboardInterrupt:
    print("\n退出程序...")
finally:
    ser.close()
    pygame.quit()