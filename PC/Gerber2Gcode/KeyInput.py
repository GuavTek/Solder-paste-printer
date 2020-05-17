class KeyboardListener:

    def __init__(self, user_com):
        self.user_com = user_com

    def __call__(self, inp):
        # abort TX
        if inp.name == "esc":
            self.user_com("abort")
            return

        # routine for handling real time commands
        elif self.user_com.user_comflags == 'R':
            if inp.name == "enter":
                key_input = input()

                if key_input == "stop real time":
                    print(">> Real time command mode disabled")
                    self.user_com.user_comflags = ''
                    self.user_com.pause()
                    return
                else:
                    # function call for sending real time commands to MCU
                    self.user_com.real_time_mode(key_input)

        # All internal commands are read here, except real time coordiantes
        # Calls the class UserCom for handling the input string
        elif inp.name == "enter":
            key_input = input()
            self.user_com(key_input)
            if self.user_com.user_comflags == 'R':
                print('>> Enabled real time command mode')
