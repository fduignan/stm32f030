arm-none-eabi-gcc -static -mthumb -g -mcpu=cortex-m0 *.cpp -T linker_script.ld -o main.elf -fno-rtti -fno-exceptions -nostartfiles
arm-none-eabi-objcopy -g -O binary main.elf main.bin
arm-none-eabi-objcopy -g -O ihex main.elf main.hex

