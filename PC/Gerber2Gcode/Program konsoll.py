import RXTXlib
import sys
from KeyInput import int_keyboard
uart = RXTXlib

start_trans = False
uart.intSerialport()
Usercom = uart.UserCom
Mcucom = uart.mcuCom()
thread_index = 0
thread_dict = {}


while True:

    Mcucom(uart.RX_routine())
    if Usercom.user_comflags == "run" and not start_trans:
        thread_index += 1
        thread_dict[thread_index] = uart.Serial_routine(thread_id=str(thread_index), name=str(thread_index)+"TX_serial_routine", data=Usercom.data)
        thread_dict[thread_index].start()
        start_trans = True

    elif Usercom.user_comflags == 'abort':
        sys.exit()

    elif Usercom.system_break:
        if not thread_dict[thread_index].is_alive():
            Usercom.system_break = False
            start_trans = False

    elif start_trans:
        if not thread_dict[thread_index].is_alive():
            start_trans = False
            Mcucom.reset()
            Usercom.reset()
            print(Usercom.user_comflags)
            Usercom.system_break = False