target remote localhost:3333
monitor reset
monitor halt
file build/out_std.elf
# file build/out_ram.elf
load
b main
c
