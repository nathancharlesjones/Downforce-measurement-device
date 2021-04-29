import serial
import json

port = serial.Serial("/dev/ttyUSB0", baudrate=460800, timeout=3.0)
data = { "channel" : "TestChannel", "time" : 10101, "value" : 555 }
testline = json.dumps(data)
print(testline)
decoded_testline = json.loads(testline)
print(decoded_testline)

while True:
    line = port.readline()
    print(line)
    try:
        decoded_line = json.loads(line)
        print("channel: " + decoded_line['channel'] + "\n")
        print("time: " + str(decoded_line['time']) + "\n")
        print("value: " + str(decoded_line['value']) + "\n")
    except (json.decoder.JSONDecodeError):
        pass
