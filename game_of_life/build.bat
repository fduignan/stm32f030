arm-none-eabi-gcc -g -O3 -static -mthumb  -mcpu=cortex-m0 *.cpp -T linker_script.ld -o main.elf -fno-rtti -fno-exceptions -nostartfiles -Xlinker --print-memory-usage -Xlinker -Map=output.map 
arm-none-eabi-objcopy -g -O binary main.elf main.bin
arm-none-eabi-objcopy -g    -O ihex main.elf main.hex

