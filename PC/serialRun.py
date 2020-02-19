import pprint
import serial
from serial.tools import list_ports

ser = serial.Serial()
pp = pprint.PrettyPrinter(indent=2)
def intSerialport():

    #std. settings for uart
    baud = 9600
    ser.baudrate=baud
    ser.port = "null"
    ser.timeout = 1
    # Check for connected devices
    x = list(list_ports.comports())

    # Print device info
    for y in x:
        pp.pprint(y.name)
        pp.pprint("Device: " + y.device)
        pp.pprint("Desc: " + y.description)
        pp.pprint("HWID: " + y.hwid)

    # connect to desired device
    if ("nEDBG CMSIS-DAP" in y.description):
        ser.port = y.device
        #show std. settings
        s = ser.get_settings()
        pp.pprint("Defined baudrate: " + str(s.get("baudrate")))
        pp.pprint("Max RX/TX bytesize: " + str(s.get("bytesize")))
        pp.pprint("RX/TX parity enabled: " + str(s.get("parity")))
        pp.pprint("RX/TX stopbits: " + str(s.get("stopbits")))
        pp.pprint("Define timeout(sec): " + str(s.get("timeout")))

        i = input("Do you want to change settings, Y/N: ") # bug here
        while(i == "Y"):

            def settings(i):
                switcher = {
                    0: 'baudrate',
                    1: 'bytesize',
                    2: 'parity',
                    3: 'stopbits',
                    4: 'timeout',
                }
                return switcher.get(i, "invalid setting")

            for x in range(5):
                print(x, settings(x), sep="-")

            set_value = settings(int(input("What setting do you want to change: ")))

            while(set_value == "invalid setting"):
                set_value = settings(int(input("What setting do you want to change: ")))

            new_value = input("Enter new value: ")
            if(new_value.isdigit() == True):
                new_value = int(new_value)
            elif((set_value == 'stopbits') or (set_value == 'timeout')):
                new_value = float(new_value)
            elif(new_value == "True" or new_value == "False"):
                new_value = bool(new_value)
            else:
                new_value = new_value

            s[set_value] = new_value
            print(s)
            ser.apply_settings(s)
            i = input("Change more settings: ")
    else:
        print("device not connected")

    # if device was found; communicate
    if (ser.port != "null"):
        ser.open()
    return

def serial_TX(data):
    #prepearing a line to be sent
    if (ser.is_open == True):
        RX = ser.readline()
        #if((TX) == "Ready"):
        rx_data = data.readline()
        for x in rx_data:
            x = ord(x)
            if (x != 32):
                ser.write(x.to_bytes(1, 'big'))
    return


def serial_RX(RX):
    if (ser.is_open == True):
        RX = ser.readline()
    return RX

serial.EIGHTBITS