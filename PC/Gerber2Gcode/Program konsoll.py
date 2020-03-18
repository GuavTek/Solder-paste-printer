import RXTXlib

uart = RXTXlib

data = open("datatest","r")
RXTXlib.intSerialport()

while 1:
    uart.commands(uart.keyboard_input)
    uart.serial_TX(data)
    uart.mcu_commands_read(uart.serial_RX())





