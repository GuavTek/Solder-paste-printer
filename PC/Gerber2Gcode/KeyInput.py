import keyboard

keys = None

def keyinp():
    global keys
    keys = None
    keys = keyboard.record()

keyboard.add_hotkey('$', keyinp)