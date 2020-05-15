import rxtx
import KeyInput
from keyboard import on_release

uart = rxtx
uart.intSerialport()
Usercom = uart.UserCom()
Mcucom = uart.McuCom()
KeyClass = KeyInput.KeyboardListener(Usercom)
start_trans = False
thread_index = 0
thread_dict = {}

INT_keyboard = on_release(KeyClass)

while True:
    Mcucom(uart.RX_routine())
    if Usercom.user_comflags == "run" and not start_trans:
        thread_index += 1
        thread_dict[thread_index] = uart.SerialThread(thread_id=str(thread_index), name=str(thread_index) + "TX_serial_routine", data=Usercom.data, mcu_class=Mcucom, user_class=Usercom)
        thread_dict[thread_index].start()
        start_trans = True

    elif not Usercom.run_tx_flag and start_trans:
        if not thread_dict[thread_index].is_alive():
            if Usercom.user_comflags == 'abort':
                Usercom.reset()
            elif Usercom.user_comflags == 'reset':
                Usercom.reset()
                Mcucom.reset()
            else:
                print('>> Data transmit done')
                Usercom.reset()
                start_trans = False
