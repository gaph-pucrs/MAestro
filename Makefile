RED  =\033[0;31m
NC   =\033[0m # No Color

TARGET = kernel

CC = riscv64-elf-gcc
OBJDUMP = riscv64-elf-objdump
OBJCOPY = riscv64-elf-objcopy

SRCDIR = src
INCDIR = $(SRCDIR)/include
HEADERS = $(wildcard $(INCDIR)/*.h)

HALDIR = hal
HDRHAL = $(wildcard $(HALDIR)/*.h)

DIRMEMPHIS = ../libmemphis
INCMEMPHIS = $(DIRMEMPHIS)/src/include
HDRMEMPHIS = $(wildcard $(DIRMEMPHIS)/*.h) $(wildcard $(DIRMEMPHIS)/**/*.h)

DIRMUTILS = ../libmutils
INCMUTILS = $(DIRMUTILS)/src/include
HDRMUTILS = $(wildcard $(DIRMUTILS)/*.h) $(wildcard $(DIRMUTILS)/**/*.h)
LIBMUTILS = $(DIRMUTILS)/libmutils.a

CFLAGS  = -march=rv32imac_zicntr_zicsr_zihpm -mabi=ilp32 -Os -fdata-sections -ffunction-sections -flto -Wall -std=c23 -I$(INCDIR) -I$(HALDIR) -I$(INCMEMPHIS) -I$(INCMUTILS)
LDFLAGS = --specs=nano.specs -T maestro.ld -march=rv32imac_zicntr_zicsr_zihpm -mabi=ilp32 -nostartfiles -Wl,--gc-sections,-flto -L$(DIRMUTILS) -lmutils

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

$(TARGET).elf: $(CCOBJ) $(ASOBJ) $(LIBMUTILS)
	@printf "${RED}Linking %s...${NC}\n" "$@"
	@$(CC) $(CCOBJ) $(ASOBJ) -Wl,-Map=$(TARGET).map -N -o $@ $(LDFLAGS)

$(TARGET).lst: $(TARGET).elf
	@printf "${RED}Generating %s...${NC}\n" "$@"
	@$(OBJDUMP) -S $< > $@

$(SRCDIR)/%.o: $(SRCDIR)/%.c $(HEADERS) $(HDRMEMPHIS) $(HDRMUTILS) $(HDRHAL)
	@printf "${RED}Compiling %s...${NC}\n" "$<"
	@$(CC) -c $< -o $@ $(CFLAGS)

$(HALDIR)/%.o: $(HALDIR)/%.S $(HDRHAL)
	@printf "${RED}Assemblying %s...${NC}\n" "$<"
	@$(CC) -c $< -o $@ $(CFLAGS) -march=rv32imac_zicsr -D__ASSEMBLY__

$(HALDIR)/%.o: $(HALDIR)/%.c $(HEADERS) $(HDRMEMPHIS) $(HDRMUTILS) $(HDRHAL)
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
