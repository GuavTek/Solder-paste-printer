import pathgenerator
from serial import Serial
from threading import Thread
from serial.tools import list_ports
from sys import platform
from os import system, name

class SerialCom(Serial):

    def __init__(self):
        Serial.__init__(self)
        self.baudrate = 9600
        self.port = "null"
        self.timeout = 1

    def IntSerialport(self):
        # std. settings for uart
        clear()
        mcu_descdict = {
            'darwin': ['nEDBG CMSIS-DAP'],
            'win32': ['Seriell USB-enhet', 'Curiosity Virtual COM Port '],
            'win64': ['Seriell USB-enhet', 'Curiosity Virtual COM Port '],
        }
        # Check for connected devices
        x = list(list_ports.comports())

        # Print device info
        for y in x:
            print(y.name)
            print("Device: " + y.device)
            print("Desc: " + y.description)
            print("HWID: " + y.hwid)
            print('\n')

        system_os = platform
        mcu_desc = mcu_descdict.get(system_os)

        # connect to desired device
        for x in mcu_desc:
            if x in y.description:
                self.port = y.device
                s = self.get_settings()
                print("Defined baudrate: " + str(s.get("baudrate")))
                print("Max RX/TX bytesize: " + str(s.get("bytesize")))
                print("RX/TX parity enabled: " + str(s.get("parity")))
                print("RX/TX stopbits: " + str(s.get("stopbits")))
                print("Define timeout(sec): " + str(s.get("timeout")))
                print('\n')
                break

        if x not in y.description:
            print("device not connected")
            return

        # if device was found; communicate
        if self.port != "null":
            self.open()

            mcu_ready = []

            while not self.is_open:
                self.open()
            self.write(b'@')
            mcu_ready.append(self.read())

            if b'o' in mcu_ready:
                print('>> MCU Connected')
                print(">> To show system commands type: <help>")
            else:
                while b'o' not in mcu_ready:
                    x = self.read()

    def RX_routine(self):
        while not self.is_open:
            self.open()
        rx = self.read()
        return rx

    def TX_routine(self, data, mcucom, usercom):
        while not self.is_open:
            self.open()

        # prepearing a line to be sent
        while usercom.run_tx_flag:
            for data_line in data:

                while usercom.read_pause_flag():
                    if not usercom.read_runflag():
                        return
                if not usercom.read_runflag():
                    return

                elif data_line == '#':
                    usercom.runflag(False)
                    mcucom.last_line_flag = True

                elif data_line != ' ' or data_line != '' and usercom.read_runflag():
                    tx_data = bytes(data_line, 'utf-8')
                    self.write(tx_data)
                    mcucom.comflag_state(tx_data, usercom)
        print(">> Data transmission done")

    def Settings(self):
        # show std. settings

        s = self.get_settings()
        print("Defined baudrate: " + str(s.get("baudrate")))
        print("Max RX/TX bytesize: " + str(s.get("bytesize")))
        print("RX/TX parity enabled: " + str(s.get("parity")))
        print("RX/TX stopbits: " + str(s.get("stopbits")))
        print("Define timeout(sec): " + str(s.get("timeout")))
        print("\n")

        i = input("Do you want to change settings Y/N: ")

        while not (i == 'Y' or i == 'N'):
            print("Invalid input")
            i = input("Do you want to change settings Y/N: ")

        while i == "Y":
            def set(i):
                switcher = {
                    0: 'baudrate',
                    1: 'bytesize',
                    2: 'parity',
                    3: 'stopbits',
                    4: 'timeout',
                }
                return switcher.get(i, "invalid setting")

            for x in range(5):
                print(x, set(x), sep="-")

            i = input("What setting do you want to change? 0-4: ")
            set_value = set(int(i))

            while set_value == "invalid setting":
                print("invalid input!")
                i = input("What setting do you want to change? 0-4: ")

                set_value = set(int(i))

            i = input("Enter new value: ")

            new_value = i

            if new_value.isdigit():
                new_value = int(new_value)

            elif set_value == 'stopbits' or set_value == 'timeout':
                new_value = float(new_value)

            elif new_value == True or new_value == False:
                new_value = bool(new_value)

            else:
                new_value = new_value

            s[set_value] = new_value
            print(s)
            self.apply_settings(s)

            i = input("Change more settings Y/N: ")
            while not (i == "Y" or i == "N"):
                print("invalid input!")
                i = input("Change more settings Y/N: ")


class UserCom:
    new_command = False
    user_comflags = ''
    data = ''
    data_name = ''
    run_tx_flag = False
    system_pause = False

    user_commands = {
        "run": "%",
        "reset": "@",
        "run cycle": "~",
        "abort": "\x18",
        "settings": "?",
        "pause": "P",
        "start": "S",
        "load file": "F",
        "create new file": 'G',
        "real time command": "R",
        "help": "H",
        "log": "L",
        "delete log": "D",
        "exit" : 'E'
    }

    system_promt = "SYSTEM: {}"

    help_promt = ">> 1.System commands:\n\n" \
                 "[<user input>: <command sent to MCU>]\n\n\t" \
                 "run: %\n\t" \
                 "reset: @\n\t"\
                 "run cycle: ~\n\t"\
                 "abort: 0x18\n\t"\
                 "settings: ?\n\t"\
                 "pause: P\n\t"\
                 "start: S\n\t"\
                 "load file: F\n\t"\
                 "send command: C\n\t"\
                 "help: H\n\n"\
                 ">> 2.Current settings:\n\n\t" \
                 "{}\n\t"\
                 "{}\n\t"\
                 "{}\n\t"\
                 "{}\n\t"\
                 "{}\n\n"\
                 ">> 3. Current file loaded:\n\t" \
                 "{}"

    def __init__(self, serial_class=SerialCom):
        self.serial_class = serial_class

    def __call__(self, user_inp):
        user_message = self.user_commands.get(user_inp, "invalid input!")

        if user_inp in self.user_commands.keys():
            #Interal commands used in Python program
            if user_message == "?":
                clear()
                print('Current settings are: ')
                self.serial_class.Settings()

            elif user_message == 'E':
                self.user_comflags = 'exit'
                self.runflag(False)

            elif user_message == "F":
                clear()
                self.data, self.data_name = load_file()

            elif user_message == 'G':
                clear()
                create_gfile()

            elif user_message == 'H':
                clear()
                s = self.serial_class.get_settings()
                print(self.help_promt.format("Defined baudrate: " + str(s.get("baudrate")),
                                            "Max RX/TX bytesize: " + str(s.get("bytesize")),
                                            "RX/TX parity enabled: " + str(s.get("parity")),
                                            "RX/TX stopbits: " + str(s.get("stopbits")),
                                            "Define timeout(sec): " + str(s.get("timeout")),
                                            self.data_name))
            elif user_message == 'L':
                clear()
                for line in McuCom.resieved_message_log:
                    print(line)

            elif user_message == 'D':
                McuCom.clearlog()

            elif user_message == "P":
                self.pause()

            elif user_message == 'R':
                self.user_comflags = 'R'
                self.pause()

            elif user_message == "S":
                self.pause()

            #External commands sendt to MCU
            else:
                while not self.serial_class.is_open:
                    self.serial_class.open()

                if user_inp == 'run':
                    self.runflag(True)

                elif user_inp == "reset":
                    clear()
                    self.serial_class.cancel_write()
                    self.runflag(False)
                    print(self.system_promt.format(user_inp))

                elif user_message == "\x18":
                    self.serial_class.cancel_write()
                    self.runflag(False)
                    print(self.system_promt.format(user_inp))

                self.user_comflags = user_inp
                tx_send = bytes(user_message, 'utf-8')
                self.serial_class.write(tx_send)

    #Method for sending real time commands to mcu
    def real_time_mode(self, inp):
        self.serial_class.write(b'%')
        for send_command in inp:
            send_command = bytes(send_command, 'utf-8')
            self.serial_class.write(send_command)
        self.serial_class.write(b'\n')

    #Method for resetting variables
    def reset(self):
        self.new_command = False
        self.user_comflags = ''
        self.data = ''
        self.data_name = ''

    def clear_userflag(self):
        self.user_comflags = ''

    #classmethod for editing values used globaly
    @classmethod
    def pause(cls):
        if cls.system_pause:
            cls.system_pause = False
        else:
            cls.system_pause = True

    @classmethod
    def read_pause_flag(cls):
        return cls.system_pause

    @classmethod
    def runflag(cls, bool):
        cls.run_tx_flag = bool

    @classmethod
    def read_runflag(cls):
        return cls.run_tx_flag


class McuCom:

    mcu_comflags = []
    command_number = 0
    last_command_number = 0
    N_line_number = 0
    N_line_exe = False
    package_number = 0
    com_withnum_flag = False
    console_txt = '{}. MCU system: {}'
    num_res = ''
    last_line_flag = False

    mcu_com_withnum = [b'N', b'L', b'E', b'H', b'O', b'F', b'G']

    withnum_com = {
        b'R': 'RX',
        b'B': 'Blockbuffer',
        b'W': 'Blockbuffer Word Overflow',
        b'X': 'X',
        b'Y': 'Y'
    }

    mcu_commands = {
        b'N': 'INSTRUCTION NR.{}: G-CODE COMMAND NOT RECOGNIZED!',
        b'E': 'UNEXPECTED EDGE ON {} AXIS!',
        b'O': '{}: BUFFER OVERFLOW! (WARNING!, UNEXECUTED LINES MAY BE OVERWRITTEN)',
        b'F': '{}: BUFFER FULL! WAITING FOR LINES STORED IN BUFFER TO BE EXECUTED',
        b'A': 'BUFFER READY TO RECEIVE DATA',
        b'G': 'BUFFER EMPTY',
        b'S': 'STOP DETECTED!',
        b'H': 'INSTRUCTION LINE NR.{}: SHORT WORD.',
        b'B': 'DATA PACKAGE NR.{} RECEIVED!',
        b'L': 'LINE NR.{}: EXECUTING',
        b'o': 'MCU is initalized',
        b'f': 'RX BUFFER IS FULL! (WARNING! UNREAD DATA MAY BE OVERWRITTEN',
        b'a': 'RX BUFFER IS READY TO RECEIVE DATA'
    }
    mcu_values = list(mcu_commands.values())
    resieved_message_log = []

    def __call__(self, inp):
        res_mcu_com = self.mcu_commands.get(inp, 'None')

        if (inp in self.mcu_commands.keys()) and (not self.com_withnum_flag):

            if inp in self.mcu_com_withnum:
                self.com_withnum_flag = True
                self.num_line = res_mcu_com
            elif res_mcu_com == 'DATA PACKAGE NR.{} RECEIVED!':

                self.package_number += 1
                self.command_print(res_mcu_com.format(self.package_number))
                self.res_mcucom_append(res_mcu_com)

            else:

                self.command_print(res_mcu_com)
                self.res_mcucom_append(res_mcu_com)

        elif self.com_withnum_flag:
            if 'LINE NR.{}: EXECUTING' in self.num_line:
                self.num_res += inp.decode('utf-8')
                if inp == b'\n':
                    self.num_res = self.num_res.replace('\n', '')
                    line_print = self.num_line.format(self.num_res)
                    self.command_print(line_print)
                    self.res_mcucom_append(self.num_line)

                    if self.last_line_flag and self.num_res == str(self.N_line_number + 1):
                        print('>> Print done')
                        self.mcu_comflags.append('Print done')

                    self.num_res = ''
                    self.num_line = ''
                    self.com_withnum_flag = False
            else:
                self.num_res = self.withnum_com.get(inp)
                self.num_res = self.num_line.format(self.num_res)
                self.command_print(self.num_res)
                self.res_mcucom_append(self.num_line)
                self.num_line = ''
                self.num_res = ''
                self.com_withnum_flag = False

    def comflag_state(self, data, usercom):

        if b'N' in data:
            self.N_line_number += 1
            if self.N_line_number > 1:
                self.N_line_exe = True

        if data == b'\n':

            res_message = False

            while self.N_line_exe or not res_message:

                if not usercom.read_runflag():
                    self.N_line_exe = False
                    res_message = True
                    return

                elif self.N_line_exe:
                    if not usercom.read_runflag():
                        self.N_line_exe = False
                        res_message = True
                        return

                    elif 'LINE NR.{}: EXECUTING' in self.mcu_comflags:
                        self.clear_mcuflag('LINE NR.{}: EXECUTING')
                        self.N_line_exe = False
                        if not len(self.mcu_comflags):
                            res_message = True

                elif 'RX BUFFER IS FULL! (WARNING! UNREAD DATA MAY BE OVERWRITTEN' in self.mcu_comflags:
                    self.clear_mcuflag('RX BUFFER IS FULL! (WARNING! UNREAD DATA MAY BE OVERWRITTEN')
                    while True:
                        if 'RX BUFFER IS READY TO RECEIVE DATA' in self.mcu_comflags:
                            self.clear_mcuflag('RX BUFFER IS READY TO RECEIVE DATA')

                elif '{}: BUFFER OVERFLOW! (WARNING!, UNEXECUTED LINES MAY BE OVERWRITTEN)' in self.mcu_comflags:
                    self.clear_mcuflag('{}: BUFFER OVERFLOW! (WARNING!, UNEXECUTED LINES MAY BE OVERWRITTEN)')

                elif '{}: BUFFER FULL! WAITING FOR LINES STORED IN BUFFER TO BE EXECUTED' in self.mcu_comflags:
                    self.clear_mcuflag('{}: BUFFER FULL! WAITING FOR LINES STORED IN BUFFER TO BE EXECUTED')

                elif 'UNEXPECTED EDGE ON {} AXIS!' in self.mcu_comflags:
                    self.clear_mcuflag('UNEXPECTED EDGE ON {} AXIS!')

                elif 'BUFFER EMPTY' in self.mcu_comflags:
                    self.clear_mcuflag('BUFFER EMPTY')

                elif 'STOP DETECTED!' in self.mcu_comflags:
                    self.clear_mcuflag('STOP DETECTED!')
                    if not len(self.mcu_comflags):
                        res_message = True

                elif 'MCU is initalized' in self.mcu_comflags:
                    self.clear_mcuflag('MCU is initalized')

                elif 'DATA PACKAGE NR.{} RECEIVED!' in self.mcu_comflags:
                    self.clear_mcuflag('DATA PACKAGE NR.{} RECEIVED!')
                    if not len(self.mcu_comflags):
                        res_message = True


                elif 'INSTRUCTION LINE NR.{}' in self.mcu_comflags:
                    self.clear_mcuflag('INSTRUCTION LINE NR.{}')
                    if not len(self.mcu_comflags):
                        res_message = True


    def command_print(self, message):

        if self.command_number == self.last_command_number + 5:
            clear()
            self.last_command_number = self.command_number

        self.command_number += 1
        print(self.console_txt.format(self.command_number, message))
        self.appendlog(self.console_txt.format(self.command_number, message))

    def res_mcucom_append(self, mcu_command):
        self.mcu_comflags.append(mcu_command)

    def clear_mcuflag(self, list):
        self.mcu_comflags.remove(list)

    def reset(self):
        self.num_line = False
        self.last_line_flag = False
        self.command_number = 0
        self.last_command_number = 0
        self.package_number = 0
        self.mcu_comflags.clear()
        self.N_line_number = 0
        self.last_N_line_number = 0
        self.num_line = ''
        self.num_res = ''

    @classmethod
    def appendlog(cls, message):
        cls.resieved_message_log.append(message)

    @classmethod
    def clearlog(cls):
        cls.resieved_message_log.clear()


class SerialThread(Thread):
    def __init__(self, thread_id, name, target, data, mcu_class, user_class):
        Thread.__init__(self)
        self.thread_id = thread_id
        self.name = name
        self.data = data
        self.mcu_class = mcu_class
        self.user_class = user_class
        self.target = target

    def run(self):
        self.target(self.data, self.mcu_class, self.user_class)


def create_gfile():

    clear()
    edge_cut_file = input(">> Type in edge cut file name: ")
    paste_file = input(">> Type in paste file name: ")
    file_name = input(">> Type in file name: ")
    pathgenerator.create_G_file(edge_cut_file, paste_file, file_name)


def load_file():

    file_name = input("Type in file name: ")
    try:
        file_temp = open(file_name, 'r')
        filename = file_temp.name
        data = file_temp.read()
        data += '\n#'
        print(">> File " + file_temp.name + " loaded")
    except FileNotFoundError:
        print("File not found, pleease check if file exsist")
        return
    finally:
        file_temp.close()
        return data, filename


def clear():
    system('cls' if name=='nt' else 'clear')


