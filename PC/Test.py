import serial
import pprint
from serial.tools import list_ports

ser = serial.Serial()
ser.baudrate = 19200
ser.port = "COM10"

pp = pprint.PrettyPrinter(indent=2)

x = list(list_ports.comports())

for y in x:
    pp.pprint(y.name)
    pp.pprint("Device: " + y.device)
    pp.pprint("Desc: " + y.description)
    pp.pprint("HWID: " + y.hwid)

ser.open()
ser.write(b'yoloo')
lul = ser.read(5)

pp.pprint("Returned: ")
pp.pprint(lul)

ser.close()