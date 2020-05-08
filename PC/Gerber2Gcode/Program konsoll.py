import RXTXlib
import KeyInput
uart = RXTXlib

start_trans = False
uart.intSerialport()



Usercom = uart.UserCom()
Mcucom = uart.mcuCom()

KeyInput.user_class = Usercom

thread_index = 0
thread_dict = {}


while True:
    Mcucom(uart.RX_routine())
    if Usercom.user_comflags == "run" and not start_trans:
        thread_index += 1
        thread_dict[thread_index] = uart.Serial_routine(thread_id=str(thread_index), name=str(thread_index)+"TX_serial_routine", data=Usercom.data, mcu_class=Mcucom, user_class=Usercom)
        thread_dict[thread_index].start()
        start_trans = True

    elif Usercom.system_break:
        if not thread_dict[thread_index].is_alive():
            Mcucom.reset()
            Usercom.reset()
            start_trans = False

    elif start_trans:
        if not thread_dict[thread_index].is_alive():
            start_trans = False
            Mcucom.reset()
            Usercom.reset()
