import serialRun
import KeyboardInput
keyboard = KeyboardInput.Keyboard(serialRun.Keyboard_input)
serialRun.intSerialport()


data = open("datatest","r")

while 1:
    serialRun.commands(serialRun.keyboard_input)
    serialRun.serial_TX(data)
    serialRun.mcu_commands_read(serialRun.serial_RX())













