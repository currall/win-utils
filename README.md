# win-utils
    windows utilities in C

## notify
- creates small, always-on-top window containing the text given in a command line argument
- building
    - gcc -o notify.exe notify.c window.c -lgdi32
- usage
    - notify message

## clock
- creates small, always-on-top window containing a clock
- building
    - gcc -o clock.exe clock.c window.c -lgdi32
- usage
    - clock.exe

## sudo
- runs any command as admin
- building
    - gcc -o sudo.exe sudo.c
- usage
    - sudo command