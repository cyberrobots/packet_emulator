Packet Emulator (work in progress)

Project tries to emulate a very nasty behaviour where stochastic delay is introduced in every packet. 
Network degradations can be introdued such as loss and delay. 

Loss is introduced based on configuration file.

Delay is calculated per packet and follows a distribution pattern based described in input file.

Build:
make

Clean: 
make clean

Run:
sudo ./p_emu.bin -f test.txt



Helpers for Debug:

Create a virtual net iface:

sudo ip link add p_emu0 type dummy
sudo ifconfig p_emu0 up



