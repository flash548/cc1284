import serial
import sys

s = serial.Serial('/dev/ttyUSB0', 115200)
lines = None
filterLines = None
try:
    outfile = open("test1.bas", 'w')
    with open("test.bas", "r") as f:
        lines = f.readlines()
    for line in lines:
        if line.strip().startswith("'"): continue
        else: outfile.write(line)
    outfile.close()
    print ("Waiting for byte...")
    s.readline()
    with open("test1.bas", 'r') as f:
        byte = f.read(1)
        while byte != "":
            print ("Sending byte...")
            s.write(byte)
            print ("Waiting for byte...")
            s.readline()
            byte = f.read(1)
            print (str(byte) + "\n")
        s.write('\0')
except:
    pass
finally:
    s.close()
