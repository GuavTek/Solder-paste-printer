import serial
import pprint
from serial.tools import list_ports

ser = serial.Serial()
ser.baudrate = 19200
ser.port = "null"

pp = pprint.PrettyPrinter(indent=2)

#Check for connected devices
x = list(list_ports.comports())

#Print device info
for y in x:
    pp.pprint(y.name)
    pp.pprint("Device: " + y.device)
    pp.pprint("Desc: " + y.description)
    pp.pprint("HWID: " + y.hwid)

    # connect to desired device
    if("Curiosity" in y.description):
        ser.port = y.device

# if device was found; communicate
if(ser.port != "null"):
    ser.open()
    ser.write(b'aabbb')
    lul = ser.read(5)
    ser.close()

pp.pprint("Returned: ")
pp.pprint(lul)

