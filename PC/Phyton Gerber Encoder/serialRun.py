import pprint
import serial
import time

from serial.tools import list_ports

ser = serial.Serial()
pp = pprint.PrettyPrinter(indent=2)
mcu_ready = None
keyboard_input = None


def intSerialport():
    # std. settings for uart
    global mcu_ready

    baud = 9600
    ser.baudrate = baud
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
        Settings()

    else:
        print("device not connected")

    # if device was found; communicate
    if (ser.port != "null"):
        ser.open()
        ser.write(b'@')
        x = mcu_commands_read(serial_RX())
        while (str(x) != "MCU is initalized"):
            x = mcu_commands_read(serial_RX())
        ser.close()
    return

def serial_TX(tx_data):

    global mcu_ready
    i = False

    # prepearing a line to be sent
    if (mcu_ready == True):
        for x in tx_data:
            while (ser.is_open != True):
                ser.open()

            x = bytes(x, 'utf-8')

            if (x != b' '):
                ser.write(x)
                time.sleep(0.1)

                y = mcu_commands_read(serial_RX())

            while (y == str("BUFFER FULL! WAITING FOR LINES STORED IN BUFFER TO BE EXECUTED") and i == False):
                y = mcu_commands_read(serial_RX())
                if (y == str("BUFFER READY TO RECEIVE DATA")):
                    i = True

        mcu_ready = False
        ser.close()
        while(ser.is_open == True):
            pass

        return

def serial_RX():
    global command_write

    while(ser.is_open != True):
        ser.open()
    if(ser.is_open == True):
        command_write = True
        RX = ser.read()
        ser.close()
        while(ser.is_open == True):
            pass
        command_write = False
    return RX

def Settings():
# show std. settings
    global keyboard_input
    s = ser.get_settings()
    pp.pprint("Defined baudrate: " + str(s.get("baudrate")))
    pp.pprint("Max RX/TX bytesize: " + str(s.get("bytesize")))
    pp.pprint("RX/TX parity enabled: " + str(s.get("parity")))
    pp.pprint("RX/TX stopbits: " + str(s.get("stopbits")))
    pp.pprint("Define timeout(sec): " + str(s.get("timeout")))

    print("Do you want to change settings Y/N: ")
    keyboard_input = None
    while(keyboard_input == None):
        while(keyboard_input == None):
            pass

        if(not(keyboard_input == "Y" or keyboard_input == "N")):
            print("Ivalid input")
            keyboard_input = None


    while (keyboard_input == "Y"):
        keyboard_input = None
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
        print("What setting do you want to change?")
        while(keyboard_input == None):
            pass

        set_value = settings(int(keyboard_input))

        while (set_value == "invalid setting"):
            keyboard_input = None
            print("What setting do you want to change: ")
            while (keyboard_input == None):
                pass

            set_value = settings(int(keyboard_input))

        keyboard_input = None

        print("Enter new value: ")
        while (keyboard_input == None):
            pass
        new_value = keyboard_input
        if (new_value.isdigit() == True):
            new_value = int(new_value)
        elif ((set_value == 'stopbits') or (set_value == 'timeout')):
            new_value = float(new_value)
        elif (new_value == "True" or new_value == "False"):
            new_value = bool(new_value)
        else:
            new_value = new_value

        s[set_value] = new_value
        print(s)
        ser.apply_settings(s)

        keyboard_input = None
        print("Change more settings Y/N: ")
        while (keyboard_input == None):
            while (keyboard_input == None):
                pass

            if (not (keyboard_input == "Y" or keyboard_input == "N")):
                print("Invalid input! Please use capital letters!")
                keyboard_input = None
    return

def Keyboard_input(inp):
    global keyboard_input
    keyboard_input = inp

def commands(inp):
    global mcu_ready
    global keyboard_input

    x = user_com(inp)
    keyboard_input = 0

    if(x == "?"):
        print('Current settings are: ')
        Settings()

    elif(x == "%"):
        while (ser.is_open != True):
            ser.open()

        x = bytes(x, 'utf-8')
        ser.write(x)
        ser.close()

        while(ser.is_open == True):
            pass

        mcu_ready = True


    elif(x == "@"):
        while (ser.is_open != True):
            ser.open()

        x = bytes(x, 'utf-8')
        ser.write(x)
        ser.close()

        while (ser.is_open == True):
            pass


def user_com(i):
    user_commands = {
        "run"       :   "%",
        "reset"     :   "@",
        "run cycle" :   "~",
        "abort"     :   "27",
        "settings"  :   "?",
    }
    return user_commands.get(i, "INVALID COMMAND!")

def mcu_commands_read(i):
    x = mcu_com(i)
    if(x == None):
        pass
    else:
        pp.pprint(x)
        return x

def mcu_com(i):
    mcu_commands = {
        b'N': 'G-CODE COMMAND NOT DEFINED! ',
        b'E': 'UNEXPECTED EDGE!',
        b'O': 'BUFFER OVERFLOW! (WARNING, STORED COMMANDS CAN BE OVERWRITTEN)',
        b'F': 'BUFFER FULL! WAITING FOR LINES STORED IN BUFFER TO BE EXECUTED',
        b'L': 'BUFFER EMPTY',
        b'A': 'BUFFER READY TO RECEIVE DATA',
        b'S': 'STOP DETECTED!',
        b'B': 'DATA RECEIVED',
        b'o': 'MCU is initalized',
    }
    return mcu_commands.get(i, None)