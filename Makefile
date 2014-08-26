
# mfloat abi: softfp or hard
MFLOAT_ABI = softfp

# MCU name
MCU = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -march=armv7e-m -mtune=cortex-m4 -mfloat-abi=$(MFLOAT_ABI) -mlittle-endian

export MCU

# Output format. (can be srec, ihex, binary)
FORMAT = ihex

# Target file name (without extension).
TARGET = stm32f4_motion_player

# IJG libjpeg directory
JPEG_DIR = ./jpeg-7

# USB Lib 
USB_LIB_PATH = lib/STM32_USB_Device_Library
USB_CLASS_PATH = $(USB_LIB_PATH)/Class/msc
USB_CORE_PATH = $(USB_LIB_PATH)/Core
USB_OTG_DRIVER_PATH = lib/STM32_USB_OTG_Driver


# List C source files here. (C dependencies are automatically generated.)
SRC = main.c stm32f4xx_it.c fat.c sd.c dojpeg.c mpool.c lcd.c icon.c cfile.c pcf_font.c mjpeg.c aac.c mp3.c sound.c fft.c fx.c xpt2046.c settings.c xmodem.c usart.c usb_bsp.c usbd_desc.c usbd_usr.c usbd_storage_msd.c  delay.c \
$(JPEG_DIR)/jdapimin.c $(JPEG_DIR)/jerror.c $(JPEG_DIR)/jdatasrc.c $(JPEG_DIR)/wrppm.c $(JPEG_DIR)/jdapistd.c $(JPEG_DIR)/jmemmgr.c \
$(JPEG_DIR)/jdmarker.c $(JPEG_DIR)/jdinput.c $(JPEG_DIR)/jcomapi.c $(JPEG_DIR)/jdmaster.c $(JPEG_DIR)/jmemnobs.c $(JPEG_DIR)/jutils.c \
$(JPEG_DIR)/jquant1.c $(JPEG_DIR)/jquant2.c $(JPEG_DIR)/jddctmgr.c $(JPEG_DIR)/jdarith.c $(JPEG_DIR)/jdcoefct.c $(JPEG_DIR)/jdmainct.c \
$(JPEG_DIR)/jdcolor.c $(JPEG_DIR)/jdsample.c $(JPEG_DIR)/jdpostct.c $(JPEG_DIR)/jdhuff.c $(JPEG_DIR)/jdmerge.c $(JPEG_DIR)/jidctint.c \
$(JPEG_DIR)/jidctfst.c $(JPEG_DIR)/jaricom.c \
$(USB_CORE_PATH)/src/usbd_core.c $(USB_CORE_PATH)/src/usbd_req.c $(USB_CORE_PATH)/src/usbd_ioreq.c \
$(USB_CLASS_PATH)/src/usbd_msc_bot.c $(USB_CLASS_PATH)/src/usbd_msc_core.c $(USB_CLASS_PATH)/src/usbd_msc_data.c $(USB_CLASS_PATH)/src/usbd_msc_scsi.c \
$(USB_OTG_DRIVER_PATH)/src/usb_dcd.c $(USB_OTG_DRIVER_PATH)/src/usb_core.c $(USB_OTG_DRIVER_PATH)/src/usb_dcd_int.c



ASRC = images.s

# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
OPT = s

# Debugging format.
DEBUG = #stabs

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
EXTRAINCDIRS = lib/CMSIS/Include lib/STM32F4xx_StdPeriph_Driver/inc lib/CMSIS/ST/STM32F4xx/Include /usr/local/arm/arm-none-eabi/include $(USB_CLASS_PATH)/inc $(USB_CORE_PATH)/inc $(USB_OTG_DRIVER_PATH)/inc ./jpeg-7 ./aac ./mp3


# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD =

# Place -D or -U options here
CDEFS = -DARM -DARM_MATH_CM4 -D__FPU_PRESENT -DSTM32F4XX -DUSE_USB_OTG_FS -DUSE_STM324xG_EVAL -DUSE_DEFAULT_STDLIB -DUSE_STDPERIPH_DRIVER

export CDEFS

# Place -I options here
CINCS =


# Compiler flags.
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
CFLAGS = -g$(DEBUG)
CFLAGS += $(CDEFS) $(CINCS)
CFLAGS += -O$(OPT)
#CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
CFLAGS += #-Wimplicit-function-declaration#-fno-strict-aliasing #-Wall -Wstrict-prototypes
#CFLAGS += -Wa,-adhlns=$(<:.c=.lst)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CFLAGS += $(CSTANDARD)


STARTUP = lib/CMSIS/ST/STM32F4xx/Source/Templates/startup_stm32f4xx.o


# Assembler flags.
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -ahlms:    create listing
#  -gstabs:   have the assembler create line number information
#ASFLAGS = -Wa,-adhlns=$(<:.S=.lst),-gstabs 
ASFLAGS = #-Wa,-gstabs



#Additional libraries.

# Minimalistic printf version
PRINTF_LIB_MIN = -Wl,-u,vfprintf -lprintf_min

# Floating point printf version (requires MATH_LIB = -lm below)
PRINTF_LIB_FLOAT = -Wl,-u,vfprintf -lprintf_flt

PRINTF_LIB = 

# Minimalistic scanf version
SCANF_LIB_MIN = -Wl,-u,vfscanf -lscanf_min

# Floating point + %[ scanf version (requires MATH_LIB = -lm below)
SCANF_LIB_FLOAT = -Wl,-u,vfscanf -lscanf_flt

SCANF_LIB = 

MATH_LIB = #-lm

# Linker flags.
#  -Wl,...:     tell GCC to pass this to linker.
#    -Map:      create map file
#    --cref:    add cross reference to  map file
LDFLAGS = -T STM32F407VG_FLASH.ld
#LDFLAGS += -Wl,-Map=$(TARGET).map,--cref
LDFLAGS += -Map=$(TARGET).map --cref --gc-section
LDFLAGS += $(PRINTF_LIB) $(SCANF_LIB) $(MATH_LIB) $(GCC_LIB) $(patsubst %,-L%,$(DIRLIB)) -lcm4 -lstd -ldsp -lc -lgcc -laac -lmp3 -lnosys

# ---------------------------------------------------------------------------

# Define directories, if needed.
DIRINC = .
DIRLIB = ./lib/STM32F4xx_StdPeriph_Driver ./lib/CMSIS/ST/STM32F4xx ./lib/CMSIS/DSP_Lib/Source $(libc_path) $(libgcc_path) ./aac ./mp3

# Define programs and commands.
SHELL = sh
CC = arm-none-eabi-gcc
LD = arm-none-eabi-ld
AS = arm-none-eabi-as
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
SIZE = arm-none-eabi-size
NM = arm-none-eabi-nm
REMOVE = rm -f
COPY = cp
YACC = bison
LEX = flex

find_cortex-m4-text :=
find_armv7e-m-text :=

ifeq "$(MFLOAT_ABI)" "hard"
	find_cortex-m4-text := thumb/cortex-m4/float-abi-hard/fpuv4-sp-d16
	find_armv7e-m-text := armv7e-m/fpu
else
	find_cortex-m4-text := thumb/cortex-m4
	find_armv7e-m-text := armv7e-m/softfp
endif

multilib := $(shell $(CC) -print-multi-lib)
find_cortex-m4 := $(findstring $(find_cortex-m4-text),$(multilib))
find_armv7e-m := $(findstring $(find_armv7e-m-text),$(multilib))
libgcc-file-name := $(shell $(CC) -print-libgcc-file-name)
libc-file-name := $(shell $(CC) -print-file-name=libc.a)
libgcc_path :=
libc_path :=
location :=


# Define all object files.
OBJ = $(SRC:.c=.o) $(ASRC:.s=.o) $(BINARY:.bin=.o)

# Define all listing files.
LST = $(ASRC:.s=.lst) $(SRC:.c=.lst)

# Compiler flags to generate dependency files.
#GENDEPFLAGS = -Wp,-M,-MP,-MT,$(*F).o,-MF,.dep/$(@F).d


# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = $(MCU) -I. $(CFLAGS) $(GENDEPFLAGS)
ALL_ASFLAGS = $(MCU) -I. -I./binary -x assembler-with-cpp $(ASFLAGS)



# Default target.
all:prepair build gccversion sizeafter

prepair:
	$(if $(find_cortex-m4),$(eval location = $(find_cortex-m4)),$(if $(find_armv7e-m),$(eval location = $(find_armv7e-m)),$(error "cannot detect library location. please specify libc_path & libgcc_path.")))
	$(eval libgcc_path = $(subst libgcc.a,$(location),$(libgcc-file-name)))
	$(eval libc_path = $(subst libc.a,$(location),$(libc-file-name)))


build: cm4lib stdlib dsplib libaac libmp3 elf bin hex lss sym 

elf: $(TARGET).elf
bin: $(TARGET).bin
hex: $(TARGET).hex
lss: $(TARGET).lss 
sym: $(TARGET).sym


cm4lib:
	$(MAKE) -C ./lib/CMSIS/ST/STM32F4xx

stdlib:	lib
	$(MAKE) -C ./lib/STM32F4xx_StdPeriph_Driver

dsplib:
	$(MAKE) -C ./lib/CMSIS/DSP_Lib/Source

libaac:
	$(MAKE) -C ./aac

libmp3:	
	$(MAKE) -C ./mp3	

jpeg:	
	$(MAKE) -C ./jpeg-7
		

# Display size of file.
HEXSIZE = $(SIZE) --target=$(FORMAT) $(TARGET).hex
ELFSIZE = $(SIZE) -A -x $(TARGET).elf
sizebefore:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_BEFORE); $(ELFSIZE); echo; fi

sizeafter:
	@if [ -f $(TARGET).elf ]; then echo; echo $(MSG_SIZE_AFTER); $(ELFSIZE); echo; fi



# Display compiler version information.
gccversion : 
	$(CC) --version

# Create binary file from ELF
%.bin: %.elf
	$(OBJCOPY) -O binary $< $@

# Create final output files (.hex, .eep) from ELF output file.
%.hex: %.elf
	$(OBJCOPY) -O $(FORMAT) $< $@


# Create extended listing file from ELF output file.
%.lss: %.elf
	$(OBJDUMP) -h -D $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	$(NM) -n $< > $@



# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(OBJ)
%.elf : $(OBJ)
	$(LD) $(STARTUP) $(OBJ) -o $@ $(LDFLAGS) 


# Compile: create object files from C source files.
%.o : %.c
	@echo
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(ALL_CFLAGS) $< -o $@ 


# Compile: create assembler files from C source files.
%.s : %.c
	$(CC) -S $(ALL_CFLAGS) $< -o $@


# Assemble: create object files from assembler source files.
%.o : %.s
	@echo
	@echo $(MSG_ASSEMBLING) $<
	$(CC) -c $(ALL_ASFLAGS) $< -o $@

distclean: clean libclean

# Target: clean project.
clean:
	$(REMOVE) $(TARGET).hex
	$(REMOVE) $(TARGET).elf
	$(REMOVE) $(TARGET).bin
	$(REMOVE) $(TARGET).map
	$(REMOVE) $(TARGET).obj
	$(REMOVE) $(TARGET).a90
	$(REMOVE) $(TARGET).sym
	$(REMOVE) $(TARGET).lnk
	$(REMOVE) $(TARGET).lss
	$(REMOVE) $(TARGET).map
	$(REMOVE) $(OBJ)
	$(REMOVE) $(LST)
	$(REMOVE) $(SRC:.c=.s)
	$(REMOVE) $(SRC:.c=.d)
	$(REMOVE) .dep/*

libclean:
	$(MAKE) -C ./lib/CMSIS/ST/STM32F4xx clean
	$(MAKE) -C ./lib/STM32F4xx_StdPeriph_Driver clean
	$(MAKE) -C ./lib/CMSIS/DSP_Lib/Source clean
	$(MAKE) -C ./aac clean
	$(MAKE) -C ./mp3 clean
	$(MAKE) -C ./jpeg-7 clean


# Include the dependency files.
#-include $(shell mkdir .dep 2>/dev/null) $(wildcard .dep/*)


# Listing of phony targets.
.PHONY : all sizebefore sizeafter gccversion \
build elf hex eep lss sym \
clean clean_list program

