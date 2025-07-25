# Makefile

CC = gcc

.PHONY: all notify clock sudo battery_tray clean

all: notify clock sudo battery_tray

notify: notify.c window.c
	$(CC) -o notify.exe notify.c window.c -lgdi32

battery: battery.c window.c
	$(CC) -o battery.exe battery.c window.c -lgdi32

clock: clock.c window.c
	$(CC) -o clock.exe clock.c window.c -lgdi32

sudo: sudo.c
	$(CC) -o sudo.exe sudo.c

battery_tray: battery_tray.c
	$(CC) battery_tray.c -o battery_tray -luser32 -lgdi32 -lshell32 -mwindows

clean:
	rm -f notify.exe clock.exe sudo.exe battery_tray
