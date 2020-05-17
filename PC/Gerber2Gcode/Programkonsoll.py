import rxtx
import KeyInput
from keyboard import on_release



def main():
    #Creating class objects and variables
    uart = rxtx.SerialCom()
    Usercom = rxtx.UserCom(serial_class=uart)
    Mcucom = rxtx.McuCom()
    KeyClass = KeyInput.KeyboardListener(Usercom)
    start_trans = False
    thread_index = 0
    thread_dict = {}

    INT_keyboard = on_release(KeyClass)
    uart.IntSerialport()

    while True:
        Mcucom(uart.RX_routine())

        # Statement for genarating threads on command.
        # Blocks as long the current thread is alive
        if Usercom.user_comflags == "run" and not start_trans:

            # thread_index used as implisit identity and object daclaration
            thread_index += 1
            thread_dict[thread_index] = rxtx.SerialThread(thread_id=str(thread_index),
                                                      name=str(thread_index) + "TX_serial_routine",
                                                      target=uart.TX_routine,
                                                      data=Usercom.data,
                                                      mcu_class=Mcucom, user_class=Usercom)
            thread_dict[thread_index].start()
            start_trans = True

        elif not Usercom.read_runflag() and start_trans:
            if not thread_dict[thread_index].is_alive():
                if Usercom.user_comflags == 'abort':
                    Usercom.reset()
                    start_trans = False
                elif Usercom.user_comflags == 'reset':
                    Usercom.reset()
                    Mcucom.reset()
                    start_trans = False
                elif 'Print done' in Mcucom.mcu_comflags:
                    Usercom.reset()
                    Mcucom.reset()
                    start_trans = False



if __name__ == '__main__':
    main()



