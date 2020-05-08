import keyboard
import RXTXlib

user_class = None

def keyinp(inp):
    global keys

    if inp.name == "esc":
        user_class("abort")
        return

    elif user_class.user_comflags == 'R':
        if inp.name == "enter":
            keys = input()
            if keys == "stop real time":
                print(">> Real time command mode disabled")
                user_class.user_comflags = ''
                user_class.system_pause = False
                return
            else:
                user_class.real_time_mode(keys)


    elif inp.name == "enter":
        keys = input()
        user_class(keys)
        if user_class.user_comflags == 'R':
            print('>> Enabled real time command mode')
        return

int_keyboard = keyboard.on_release(keyinp)
