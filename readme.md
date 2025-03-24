# Forest Monitoring - Baseline Experiment Code

This repository contains the baseline code used in my forest monitoring project to benchmark energy consumption and transmission performance using LoRa-based sensor nodes.

## 🔬 Baseline Test Conditions

- First, the system will be tested with **1 supercapacitor (5.5V, 15F)**
- Then, it will be tested again with **2 supercapacitors in parallel**
- This comparison will help evaluate the effect of capacitor capacity on node uptime and energy behavior

## 📡 Transmitter (Sensor Node)
- Hardware: Feather M0 LoRa, DHT22, voltage divider
- Sends temperature, humidity, voltage, and uptime every 15 seconds
- Built-in LED blinks after each transmission for visual confirmation
- Powered only by capacitor(s), no USB or solar panel during test

## 📥 Receiver (Gateway)
- Hardware: Feather M0 LoRa
- Receives packets over LoRa and prints data via Serial (for Raspberry Pi or PC)
- Displays RSSI and SNR for each packet
- Blinks LED after successful packet reception

## 🐍 Python Data Logger (CSV)
- Runs on Raspberry Pi or macOS
- Reads JSON + RSSI/SNR lines from Serial
- Logs each complete packet to a **timestamped CSV file**
- Flushes data in real-time so you can open the CSV live
- Starts a new CSV file on every script run

## 🗂️ File Structure

