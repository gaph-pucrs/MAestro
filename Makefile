RED  =\033[0;31m
NC   =\033[0m # No Color

TARGET = kernel

CC = riscv64-elf-gcc
OBJDUMP = riscv64-elf-objdump
OBJCOPY = riscv64-elf-objcopy

SRCDIR = src
INCDIR = $(SRCDIR)/include
HALDIR = hal

LIBMEMPHISDIR = ../libmemphis/src
LIBUTILSDIR = ../libmutils/src
INCMEMPHIS = $(LIBMEMPHISDIR)/include
INCMUTILS = $(LIBUTILSDIR)/include

CFLAGS  = -march=rv32im -mabi=ilp32 -Os -fdata-sections -ffunction-sections -flto -Wall -std=c11 -I$(INCDIR) -I$(INCMEMPHIS) -I$(INCMUTILS) -I$(HALDIR)
LDFLAGS = --specs=nano.specs -march=rv32im -mabi=ilp32 -nostartfiles -Wl,--section-start=".init"=0,--section-start=".rodata=01000000",--gc-sections,-flto -L../libmutils -lmutils

CCSRC = $(wildcard $(SRCDIR)/*.c) $(wildcard $(HALDIR)/*.c)
CCOBJ = $(patsubst %.c, %.o, $(CCSRC))

ASSRC = $(wildcard $(HALDIR)/*.S)
ASOBJ = $(patsubst %.S,%.o, $(ASSRC))

all: i$(TARGET).bin d$(TARGET).bin $(TARGET).lst

d$(TARGET).bin: $(TARGET).elf
	@printf "${RED}Generating %s...${NC}\n" "$@"
	@$(OBJCOPY) $< $@ -O binary -j .rodata -j .data -j .sdata

i$(TARGET).bin: $(TARGET).elf
	@printf "${RED}Generating %s...${NC}\n" "$@"
	@$(OBJCOPY) $< $@ -O binary -j .init -j .text

$(TARGET).elf: $(CCOBJ) $(ASOBJ)
	@printf "${RED}Linking %s...${NC}\n" "$@"
	@$(CC) $^ -Wl,-Map=$(TARGET).map -N -o $@ $(LDFLAGS)

$(TARGET).lst: $(TARGET).elf
	@printf "${RED}Generating %s...${NC}\n" "$@"
	@$(OBJDUMP) -S $< > $@

$(SRCDIR)/%.o: $(SRCDIR)/%.c
	@printf "${RED}Compiling %s...${NC}\n" "$<"
	@$(CC) -c $< -o $@ $(CFLAGS)

$(HALDIR)/%.o: $(HALDIR)/%.S
	@printf "${RED}Assemblying %s...${NC}\n" "$<"
	@$(CC) -c $< -o $@ $(CFLAGS) -march=rv32im_zicsr -D__ASSEMBLY__

$(HALDIR)/%.o: $(HALDIR)/%.c
	@printf "${RED}Compiling %s...${NC}\n" "$<"
	@$(CC) -c $< -o $@ $(CFLAGS)

clean:
	@printf "Cleaning up\n"
	@rm -rf src/*.o
	@rm -rf hal/*.o
	@rm -rf *.bin
	@rm -rf *.map
	@rm -rf *.lst
	@rm -rf *.elf

.PHONY: clean
