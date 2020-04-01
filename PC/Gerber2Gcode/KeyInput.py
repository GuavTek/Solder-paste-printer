import keyboard
import RXTXlib

User_commands = RXTXlib.UserCom()


def keyinp(inp):
    global keys

    if inp.name == 'esc':
        User_commands('abort')

    elif inp.name == "enter":
        keys = input()
        User_commands(keys)


int_keyboard = keyboard.on_release(keyinp)
