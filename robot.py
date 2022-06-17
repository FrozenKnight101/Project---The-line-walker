from datetime import datetime
import serial
import re
import json
import paho.mqtt.client as mqtt
import requests

#Referenceed from SIT310
class ReadLine:
    def __init__(self, s):
        self.buf = bytearray()
        self.s = s

    def readline(self):
        i = self.buf.find(b"\n")
        if i >= 0:
            r = self.buf[:i + 1]
            self.buf = self.buf[i + 1:]
            return r
        while True:
            i = max(1, min(2048, self.s.in_waiting))
            data = self.s.read(i)
            i = data.find(b"\n")
            if i >= 0:
                r = self.buf + data[:i + 1]
                self.buf[0:] = data[i + 1:]
                return r
            else:
                self.buf.extend(data)


if __name__ == '__main__':
    # Connect to serial port to send and receive data from arduino
    try:
        serialPort = serial.Serial(
            port='/dev/ttyACM0', \
            baudrate=9600, \
            parity=serial.PARITY_NONE, \
            stopbits=serial.STOPBITS_ONE, \
            bytesize=serial.EIGHTBITS, \
            timeout=0)
    except serial.SerialException as e:
        print("Serial Port Connection failed")
        exit()

    mqtt_client = mqtt.Client()
    mqtt_client.connect("broker.hivemq.com", 1883)

    RL = ReadLine(serialPort)
    
    input("Press any key to start robot...")
    cmd = "START\n"
    serialPort.write(cmd.encode(encoding='utf-8'))
    
    previous_state = 0

    while (True):
        # To handle errors that arise when data is not read or decoded properly
        try:
            line = RL.readline().decode(encoding="utf-8")
        except Exception as e:
            continue

        # Check if sensor data received is valid and in the right format using regex
        pattern = re.compile(r"\[(\d+\.\d+)%,(\d+\.\d+)C,(\d+)cm]", re.UNICODE)
        data = re.match(pattern, line)
        if data is None:
            continue

        humidity = float(data.group(1))
        temperature = float(data.group(2))
        frontDistance = int(data.group(3))

        # MQTT FOR TEMPERATURE AND HUMIDITY DATA
        # Getting the current date and time
        dt = datetime.now()

        sensorData = {
            "humidity": humidity,
            "temperature": temperature,
            "timeStamp": datetime.timestamp(dt)
        }

        # Publish Robot 1's sensor data to its MQTT topic
        mqtt_client.publish("robot_1/sensordata", json.dumps(sensorData))
        
        #--------------------------------------------------------------------------#

        # IFTTT FOR OBSTACLE AVOIDANCE WITH ULTRASONIC DATA
        if frontDistance <= 8:
            # Send notification if obstacle was newly detected
            if previous_state == 0:
                r = requests.post('https://maker.ifttt.com/trigger/obstacle_detected/with/key/bobbIpt-NkJFf_g6alEAZQ',
                        params={'value1': 'Path blocked - Obstacle detected in front of Robot 1'})
                previous_state = 1
                #print("Obstacle detected - Notification Sent")
        else:
            # Send notification if previously detected obstacle was removed
            if previous_state == 1:
                r = requests.post('https://maker.ifttt.com/trigger/obstacle_detected/with/key/bobbIpt-NkJFf_g6alEAZQ',
                        data={'value1': 'Path cleared - Obstacle in front of Robot 1 was removed'})
                previous_state = 0
                #print("Obstacle removed - Notification Sent")
                
