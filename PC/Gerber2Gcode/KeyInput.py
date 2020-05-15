from typing import Callable

import keyboard
import rxtx

#function for capturing konsoll input
def keyinp(inp, user_class=rxtx.UserCom()):

    #abort TX
    if inp.name == "esc":
        user_class("abort")
        return

    #routine for handling real time commands
    elif user_class.user_comflags == 'R':
        if inp.name == "enter":
            key_input = input()

            if key_input == "stop real time":
                print(">> Real time command mode disabled")
                user_class.user_comflags = ''
                user_class.system_pause = False
                return
            else:
                #function call for sending real time commands to MCU
                user_class.real_time_mode(key_input)

    #All internal commands are read here, except real time coordiantes
    #Calls the class UserCom for handling input string
    elif inp.name == "enter":
        key_input = input()
        user_class(key_input)
        if user_class.user_comflags == 'R':
            print('>> Enabled real time command mode')
        return

INT_keyboard = keyboard.on_release(keyinp)
