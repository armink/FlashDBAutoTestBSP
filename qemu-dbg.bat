@echo off
if exist sd.bin goto run
qemu-img create -f raw sd.bin 64M

:run
qemu-system-arm -M vexpress-a9 -kernel Debug/rtthread.elf -nographic -sd sd.bin -S -s
