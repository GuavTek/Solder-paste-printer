import pprint
import serial
import threading
import sys
from serial.tools import list_ports


ser = serial.Serial()
pp = pprint.PrettyPrinter(indent=2)


class UserCom:
    new_command = False
    user_comflags = ''
    data = ''
    data_name = ''
    system_break = False
    system_pause = None
    user_commands = {
        "run": "%",
        "reset": "@",
        "run cycle": "~",
        "abort": "\x18",
        "settings": "?",
        "pause": "P",
        "start": "S",
        "load file": "F",
        "help": "H"
    }
    system_promt = "SYSTEM: {}"
    help_promt = ">> 1.System commands:\n" \
                 "[<user input>: <command sent to MCU>]\n\t" \
                 "run: %\n\t" \
                 "reset: @\n\t"\
                 "run cycle: ~\n\t"\
                 "abort: 0x18\n\t"\
                 "settings ?\n\t"\
                 "pause: P\n\t"\
                 "start: S\n\t"\
                 "load file: F\n\t"\
                 "help: H\n"\
                 ">> 2.Current settings:\n\t" \
                 "{}\n\t"\
                 "{}\n\t"\
                 "{}\n\t"\
                 "{}\n\t"\
                 "{}\n"\
                 ">> 3. Current file loaded:\n\t" \
                 "{}"

    s = ser.get_settings()

    def __call__(self, user_inp):
        user_message = self.user_commands.get(user_inp, "invalid input!")

        if user_inp in self.user_commands.keys():

            UserCom.new_command = True

            if user_message == "\x18":
                pp.pprint(self.system_promt.format(user_inp))
                while (ser.is_open != True):
                    ser.open()
                com_send = bytes(user_message, 'utf-8')
                ser.cancel_write()
                ser.write(com_send)
                UserCom.user_comflags = user_inp
                UserCom.reset()

            elif user_message == "?":
                pp.pprint('Current settings are: ')
                settings()

            elif user_message == "F":
                UserCom.data, UserCom.data_name = load_file()

            elif user_message == "P":
                UserCom.system_pause = True

            elif user_message == "S":
                UserCom.system_pause = False

            elif user_message == 'H':
                print(self.help_promt.format("Defined baudrate: " + str(self.s.get("baudrate")),
                                            "Max RX/TX bytesize: " + str(self.s.get("bytesize")),
                                            "RX/TX parity enabled: " + str(self.s.get("parity")),
                                            "RX/TX stopbits: " + str(self.s.get("stopbits")),
                                            "Define timeout(sec): " + str(self.s.get("timeout")),
                                            UserCom.data_name))

            else:
                while (ser.is_open != True):
                    ser.open()

                if user_inp == "reset":
                    UserCom.reset()
                    mcuCom.reset()
                    pp.pprint(self.system_promt.format(user_inp))

                UserCom.user_comflags = user_inp
                com_send = bytes(user_message, 'utf-8')
                ser.write(com_send)

    @classmethod
    def reset(cls):
        cls.data_name = ''
        cls.data = ''
        cls.user_comflags = ''
        cls.system_break = True
        cls.new_command = False

    @classmethod
    def add_commands(cls, key, value):
        cls.user_commands.update({str(key): str(value)})

    @classmethod
    def change_system_promt(cls, new_promt):
        cls.system_promt = new_promt + ": {}"

    @classmethod
    def clear_userflag(cls):
        cls.user_comflags = ''

    @classmethod
    def clear_new_com(cls):
        cls.new_command = False

class mcuCom:

    mcu_comflags = []
    mcu_com_withnum = [b'N', b'L', b'E', b'H', b'O', b'F']
    mcu_com_num = {
        b'R' : 'RX',
        b'B' : 'Blockbuffer',
        b'W' : 'Blockbuffer Word Overflow'
    }

    line = 0
    line_flag = 0
    last_line_flag = 0
    wait_for_num = False
    console_txt = '{}. MCU system: {}'

    mcu_commands = {
        b'N': 'INSTRUCTION NR.{}: G-CODE COMMAND NOT RECOGNIZED!',
        b'E': 'UNEXPECTED EDGE ON {} AXIS!',
        b'O': '{}: BUFFER OVERFLOW! (WARNING!, UNEXECUTED LINES MAY BE OVERWRITTEN)',
        b'F': '{}: BUFFER FULL! WAITING FOR LINES STORED IN {} BUFFER TO BE EXECUTED',
        b'A': 'BUFFER READY TO RECEIVE DATA',
        b'S': 'STOP DETECTED!',
        b'H': 'INSTRUCTION LINE NR.{}: SHORT WORD.',
        b'B': 'DATA RECEIVED!',
        b'L': 'LINE NR.{}: EXECUTING',
        b'o': 'MCU is initalized',
        b'f': 'RX BUFFER IS FULL! (WARNING! UNREAD DATA MAY BE OVERWRITTEN',
        b'a': 'RX BUFFER IS READY TO RECEIVE DATA'
    }
    mcu_values = list(mcu_commands.values())

    def __init__(self):
        self.num_line = ''
        self.num_res = ''

    def __call__(self, inp):

        res_mcu_com = self.mcu_commands.get(inp, 'None')

        if inp in self.mcu_commands.keys() and not self.wait_for_num:
            if inp in self.mcu_com_withnum:
                self.wait_for_num = True
                self.num_line = res_mcu_com

            else:
                self.line += 1
                self.res_mcucom_append(res_mcu_com)
                pp.pprint(self.console_txt.format(self.line, res_mcu_com))

        elif self.wait_for_num:
            if 'LINE NR.{}: EXECUTING' in self.num_line:
                self.num_res += inp.decode('utf-8')
                if inp == b'\n':
                    self.line += 1
                    self.num_res = self.num_res.replace('\n', '')
                    pp.pprint(self.console_txt.format(self.line, self.num_line.format(self.num_res)))
                    self.res_mcucom_append(self.num_line)
                    self.num_res = ''
                    self.num_line = ''
                    self.wait_for_num = False

            else:
                self.line += 1
                self.num_res = self.mcu_com_num.get(inp, inp.decode('utf-8'))
                pp.pprint(self.console_txt.format(self.line, self.num_line.format(self.num_res)))
                self.res_mcucom_append(self.num_line)
                self.num_line = ''
                self.wait_for_num = False


    def comflag_state(self, data):
        if 'N' in data:
            self.line_flag += 1

        if data == '\n':
            while True:
                if self.system_break:
                    return

                elif self.system_pause:
                    while True:
                        if not self.system_pause:
                            break

                elif ('DATA RECEIVED!') in self.mcu_comflags:
                    clear_mcuflag('DATA RECEIVED!')
                    break

                elif self.line_flag >= 2 and self.line_flag > self.last_line_flag:
                    while self.last_line_flag != self.line_flag:
                        if self.system_break:
                            return

                        elif 'LINE NR.{}: EXECUTING' in self.mcu_comflags:
                            self.clear_mcuflag('LINE NR.{}: EXECUTING')
                            self.last_line_flag += 1

                elif 'RX BUFFER IS FULL! (WARNING! UNREAD DATA MAY BE OVERWRITTEN' in mcuCom.mcu_comflags:
                    self.clear_mcuflag('RX BUFFER IS FULL! (WARNING! UNREAD DATA MAY BE OVERWRITTEN')
                    while True:
                        if 'RX BUFFER IS READY TO RECEIVE DATA' in self.mcu_comflags:
                           self.clear_mcuflag('RX BUFFER IS READY TO RECEIVE DATA')
                            break

    @classmethod
    def res_mcucom_append(cls, mcu_command):
        cls.mcu_comflags.append(mcu_command)

    @classmethod
    def clear_mcuflag(cls, list):
        cls.mcu_comflags.remove(list)

    @classmethod
    def reset(cls):
        cls.num_line = False
        cls.line = 0
        cls.mcu_comflags.clear()
        cls.line_flag = 0
        cls.last_line_flag = 0




class Serial_routine(threading.Thread):

    def __init__(self, thread_id, name, data):
        threading.Thread.__init__(self)
        self.thread_id = thread_id
        self.name = name
        self.data = data

    def run(self):
        TX_routine(self.data)



def intSerialport():
    # std. settings for uart

    baud = 9600
    ser.baudrate = baud
    ser.port = "null"
    ser.timeout = 1
    mcu_descdict = {


        'darwin': ['nEDBG CMSIS-DAP'],
        'win32':  ['Seriell USB-enhet', 'Curiosity Virtual COM Port '],
        'win64':  ['Seriell USB-enhet', 'Curiosity Virtual COM Port '],

    }
    # Check for connected devices
    x = list(list_ports.comports())

    # Print device info
    for y in x:
        pp.pprint(y.name)
        pp.pprint("Device: " + y.device)
        pp.pprint("Desc: " + y.description)
        pp.pprint("HWID: " + y.hwid)
        print('\n')

    system_os = sys.platform
    mcu_desc = mcu_descdict.get(system_os)

    # connect to desired device
    for x in mcu_desc:
        if x in y.description:
            ser.port = y.device
            s = ser.get_settings()
            pp.pprint("Defined baudrate: " + str(s.get("baudrate")))
            pp.pprint("Max RX/TX bytesize: " + str(s.get("bytesize")))
            pp.pprint("RX/TX parity enabled: " + str(s.get("parity")))
            pp.pprint("RX/TX stopbits: " + str(s.get("stopbits")))
            pp.pprint("Define timeout(sec): " + str(s.get("timeout")))
            print('\n')


    if x not in y.description:
        print("device not connected")
        return

    print(">> To show system commands type: <help>")
    # if device was found; communicate
    if ser.port != "null":
        ser.open()
        while not ser.is_open:
            if ser.is_open:
                break

        ser.write(b'@')
        init = mcuCom()
        init(ser.read())
        while 'MCU is initalized' not in init.mcu_comflags:
            ser.write(b'@')
            init(ser.read())
    return


def RX_routine():
    while not ser.is_open:
        ser.open()
    RX = ser.read()
    return RX


def TX_routine(data):
    MCU_commands = mcuCom()
    User_commands = UserCom()

    # prepearing a line to be sent
    while True:
        while (ser.is_open == False):
            ser.open()
        for x in data:

            TX = bytes(x, 'utf-8')
            if (TX != b' ' or TX != ''):
                ser.write(TX)
                if User_commands.system_break:
                    return
                MCU_commands.comflag_state(x)
        ser.write(b'\n')
        break


def load_file():

    file_name = input("Type in file name: ")
    try:
        file_temp = open(file_name, 'r')
        filename = file_temp.name
        data = file_temp.read()
        print(">> File " + file_temp.name + " loaded")
    except FileNotFoundError:
        print("File not found, pleease check if file exsist")
    finally:
        file_temp.close()
        return data, filename


def settings():
    # show std. settings

    s = ser.get_settings()
    pp.pprint("Defined baudrate: " + str(s.get("baudrate")))
    pp.pprint("Max RX/TX bytesize: " + str(s.get("bytesize")))
    pp.pprint("RX/TX parity enabled: " + str(s.get("parity")))
    pp.pprint("RX/TX stopbits: " + str(s.get("stopbits")))
    pp.pprint("Define timeout(sec): " + str(s.get("timeout")))
    pp.pprint("\n")

    i = input("Do you want to change settings Y/N: ")

    while (not (i == 'Y' or i == 'N')):
        print("Invalid input")
        i = input("Do you want to change settings Y/N: ")

    while (i == "Y"):

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

        i = input("What setting do you want to change? 0-4: ")
        set_value = settings(int(i))

        while (set_value == "invalid setting"):
            print("invalid input!")
            i = input("What setting do you want to change? 0-4: ")

            set_value = settings(int(i))

        i = input("Enter new value: ")

        new_value = i
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

        i = input("Change more settings Y/N: ")
        while not (i == "Y" or i == "N"):
            print("invalid input!")
            i = input("Change more settings Y/N: ")
    return
