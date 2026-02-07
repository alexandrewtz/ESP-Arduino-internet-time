# ESP32 Internet Time to Arduino Mega

This project uses an ESP32 to connect to the internet, retrieve the current time from an NTP server, and send it via serial communication to an Arduino Mega board.

## Hardware Setup

### Connections
Connect the ESP32 to Arduino Mega as follows:

- **ESP32 TX** → **Arduino Mega RX1 (Pin 19)**
- **ESP32 GND** → **Arduino Mega GND**

### Required Hardware
- ESP32 board (configured for Adafruit Feather ESP32-S3)
- Arduino Mega 2560
- USB cables for programming both boards
- Jumper wires for serial connection

## Project Initialization

### Prerequisites
- [PlatformIO Core](https://platformio.org/install/cli) or [PlatformIO IDE](https://platformio.org/install/ide) installed
- Arduino IDE (for uploading Arduino Mega code)
- Python 3.6 or later (required by PlatformIO)

### Initialize the Project

1. **Navigate to the project directory:**
   ```bash
   cd ESP-Arduino-internet-time
   ```

2. **Install PlatformIO dependencies:**
   ```bash
   pio pkg install
   ```
   This will download the ESP32 platform and required libraries.

3. **Build the project to verify setup:**
   ```bash
   pio run
   ```
   This compiles the code and checks for any errors before uploading.

4. **List available devices (optional):**
   ```bash
   pio device list
   ```
   This shows all connected serial devices to identify your ESP32's port.

## Software Setup

### 1. Configure WiFi Credentials

WiFi credentials are managed through environment variables for security. Do NOT commit credentials to Git.

**Option 1: Using `.env` file (Recommended)**

1. Copy the example environment file:
   ```bash
   cp .env.example .env
   ```

2. Edit `.env` and add your WiFi credentials:
   ```bash
   WIFI_SSID=your_network_name
   WIFI_PASSWORD=your_network_password
   ```

3. PlatformIO will automatically load these variables when building.

**Option 2: Using system environment variables**

Set environment variables in your shell:

```bash
export WIFI_SSID="your_network_name"
export WIFI_PASSWORD="your_network_password"
pio run --target upload
```

**Option 3: Using PlatformIO CLI environment**

Pass credentials directly via command line:

```bash
WIFI_SSID=your_network_name WIFI_PASSWORD=your_network_password pio run --target upload
```

⚠️ **Security Note:** The `.env` file is in `.gitignore` and will NOT be committed to Git. Do not commit credentials to version control.

### 2. Timezone Configuration

**Default:** The project is configured for **CET/CEST** (Central European Time) with **automatic daylight saving time** handling.

- **CET (Winter):** UTC+1
- **CEST (Summer):** UTC+2
- **DST Period:** Last Sunday in March (2:00 AM) to last Sunday in October (3:00 AM)

**To change timezone** (optional), edit the timezone string in `src/main.cpp`:

```cpp
const char* timezone = "CET-1CEST,M3.5.0,M10.5.0/3";
```

**Other timezone examples:**
- **PST/PDT (US Pacific):** `"PST8PDT,M3.2.0,M11.1.0"`
- **EST/EDT (US Eastern):** `"EST5EDT,M3.2.0,M11.1.0"`
- **GMT (No DST):** `"GMT0"`
- **JST (Japan, No DST):** `"JST-9"`

### 3. Upload ESP32 Code

```bash
# Build and upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

### 4. Upload Arduino Mega Code

1. Open `arduino_mega_receiver.ino` in Arduino IDE
2. Select **Board: Arduino Mega 2560**
3. Select the correct COM port
4. Click **Upload**

## How It Works

### ESP32 (Transmitter)
1. Connects to WiFi network
2. Synchronizes time with NTP server (`pool.ntp.org`)
3. Sends time data every second via Serial (115200 baud)
4. Sends two formats:
   - Human-readable: `YYYY-MM-DD HH:MM:SS`
   - Parseable: `TIME:year,month,day,hour,minute,second`

### Arduino Mega (Receiver)
1. Listens on Serial1 (RX1/TX1 - pins 19/18)
2. Receives and parses time data from ESP32
3. Displays time on Serial Monitor (USB)
4. Stores current time in `currentTime` structure

## Time Data Format

The ESP32 sends time in two formats:

### Format 1: Human Readable
```
2026-02-07 14:30:45
```

### Format 2: Parseable (CSV)
```
TIME:2026,2,7,14,30,45
```

This format is easier to parse on the Arduino side.

## Customization

### Change Update Interval

In `src/main.cpp`, modify:
```cpp
const unsigned long TIME_UPDATE_INTERVAL = 1000; // milliseconds
```

### Change Baud Rate

Make sure both ESP32 and Arduino use the same baud rate:
- ESP32: `Serial.begin(115200);`
- Arduino: `Serial1.begin(115200);`

## Troubleshooting

### ESP32 won't connect to WiFi
- Verify WiFi credentials are correct
- Check if your network is 2.4GHz (ESP32 doesn't support 5GHz)
- Ensure WiFi network is in range

### Arduino not receiving data
- Check physical connections (TX→RX1, GND→GND)
- Verify both boards use 115200 baud rate
- Make sure ESP32 is successfully connected to WiFi
- Check Serial Monitor on Arduino (should use Serial1 for receiving)

### Time is incorrect
- Verify the timezone string is correct for your location in `src/main.cpp`
- DST is handled automatically for CET/CEST (default configuration)
- For other timezones, use appropriate POSIX timezone string (see Timezone Configuration section)
- Check if NTP server is accessible from your network

## Serial Monitor Output

### ESP32 Output
```
ESP32 Internet Time to Arduino
================================
Connecting to WiFi...
WiFi connected!
IP address: 192.168.1.100
Waiting for time synchronization...
Time synchronized!
Friday, February 07 2026 14:30:45
Timezone: CET (UTC+1)
2026-02-07 14:30:45
TIME:2026,2,7,14,30,45
```

### Arduino Mega Output
```
Arduino Mega - Time Receiver
============================
Waiting for time data from ESP32...
Time: 2026-02-07 14:30:45
Received time: 2026-02-07 14:30:45
```

## License

This project is provided as-is for educational purposes.
