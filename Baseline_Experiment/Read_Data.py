import serial
import csv
import json
from datetime import datetime

# Set your correct serial port here (use ls /dev/tty.* on Mac or dmesg on Linux)
ser = serial.Serial('/dev/tty.usbmodem1301', 9600, timeout=1)

# Generate a unique filename with timestamp
timestamp_now = datetime.now().strftime('%Y-%m-%d_%H-%M-%S')
filename = f'baseline_data_{timestamp_now}.csv'

# Open CSV file and write header
csv_file = open(filename, mode='w', newline='')
csv_writer = csv.writer(csv_file)
csv_writer.writerow([
    'Timestamp', 'Uptime(s)', 'Voltage(V)', 'Temp(C)', 
    'Humidity(%)', 'PacketID', 'RSSI (dBm)', 'SNR (dB)'
])

print(f"🚀 Logging data to CSV: {filename}\n")

current_data = {}
rssi = None
snr = None

try:
    while True:
        # Read and decode the line from serial safely
        line = ser.readline().decode('utf-8', errors='ignore').strip()
        if not line:
            continue

        print(f"RAW >> {line}")  # Optional: live debug

        if line.startswith("{") and line.endswith("}"):
            try:
                current_data = json.loads(line)
            except json.JSONDecodeError:
                continue

        elif line.startswith("RSSI:"):
            try:
                rssi = int(line.split(":")[1].strip())
            except:
                rssi = None

        elif line.startswith("SNR:"):
            try:
                snr = float(line.split(":")[1].strip())
            except:
                snr = None

            # Log once we have complete data
            if current_data:
                timestamp = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                csv_writer.writerow([
                    timestamp,
                    current_data.get('uptime'),
                    current_data.get('voltage'),
                    current_data.get('temp'),
                    current_data.get('humidity'),
                    current_data.get('id'),
                    rssi,
                    snr
                ])
                csv_file.flush()  # Ensure it's written immediately

                print(f"🟢 {timestamp} | ID {current_data.get('id')} | V={current_data.get('voltage')}V | RSSI={rssi} | SNR={snr}\n")

                # Reset values for next round
                current_data = {}
                rssi = None
                snr = None

except KeyboardInterrupt:
    print("🛑 Logging stopped by user.")
finally:
    csv_file.close()
    ser.close()
