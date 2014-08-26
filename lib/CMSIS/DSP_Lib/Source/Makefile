
# MCU name
#MCU = -mcpu=cortex-m4 -mthumb -mfpu=fpv4-sp-d16 -march=armv7e-m -mtune=cortex-m4 -mfloat-abi=softfp -mlittle-endian -mthumb-interwork

# Target file name (without extension).
TARGET = libdsp.a


# List C source files here. (C dependencies are automatically generated.)
SRC = BasicMathFunctions/arm_abs_f32.c \
BasicMathFunctions/arm_abs_q15.c \
BasicMathFunctions/arm_abs_q31.c \
BasicMathFunctions/arm_abs_q7.c \
BasicMathFunctions/arm_add_f32.c \
BasicMathFunctions/arm_add_q15.c \
BasicMathFunctions/arm_add_q31.c \
BasicMathFunctions/arm_add_q7.c \
BasicMathFunctions/arm_dot_prod_f32.c \
BasicMathFunctions/arm_dot_prod_q15.c \
BasicMathFunctions/arm_dot_prod_q31.c \
BasicMathFunctions/arm_dot_prod_q7.c \
BasicMathFunctions/arm_mult_f32.c \
BasicMathFunctions/arm_mult_q15.c \
BasicMathFunctions/arm_mult_q31.c \
BasicMathFunctions/arm_mult_q7.c \
BasicMathFunctions/arm_negate_f32.c \
BasicMathFunctions/arm_negate_q15.c \
BasicMathFunctions/arm_negate_q31.c \
BasicMathFunctions/arm_negate_q7.c \
BasicMathFunctions/arm_offset_f32.c \
BasicMathFunctions/arm_offset_q15.c \
BasicMathFunctions/arm_offset_q31.c \
BasicMathFunctions/arm_offset_q7.c \
BasicMathFunctions/arm_scale_f32.c \
BasicMathFunctions/arm_scale_q15.c \
BasicMathFunctions/arm_scale_q31.c \
BasicMathFunctions/arm_scale_q7.c \
BasicMathFunctions/arm_shift_q15.c \
BasicMathFunctions/arm_shift_q31.c \
BasicMathFunctions/arm_shift_q7.c \
BasicMathFunctions/arm_sub_f32.c \
BasicMathFunctions/arm_sub_q15.c \
BasicMathFunctions/arm_sub_q31.c \
BasicMathFunctions/arm_sub_q7.c \
CommonTables/arm_common_tables.c \
ComplexMathFunctions/arm_cmplx_conj_f32.c \
ComplexMathFunctions/arm_cmplx_conj_q15.c \
ComplexMathFunctions/arm_cmplx_conj_q31.c \
ComplexMathFunctions/arm_cmplx_dot_prod_f32.c \
ComplexMathFunctions/arm_cmplx_dot_prod_q15.c \
ComplexMathFunctions/arm_cmplx_dot_prod_q31.c \
ComplexMathFunctions/arm_cmplx_mag_f32.c \
ComplexMathFunctions/arm_cmplx_mag_q15.c \
ComplexMathFunctions/arm_cmplx_mag_q31.c \
ComplexMathFunctions/arm_cmplx_mag_squared_f32.c \
ComplexMathFunctions/arm_cmplx_mag_squared_q15.c \
ComplexMathFunctions/arm_cmplx_mag_squared_q31.c \
ComplexMathFunctions/arm_cmplx_mult_cmplx_f32.c \
ComplexMathFunctions/arm_cmplx_mult_cmplx_q15.c \
ComplexMathFunctions/arm_cmplx_mult_cmplx_q31.c \
ComplexMathFunctions/arm_cmplx_mult_real_f32.c \
ComplexMathFunctions/arm_cmplx_mult_real_q15.c \
ComplexMathFunctions/arm_cmplx_mult_real_q31.c \
ControllerFunctions/arm_pid_init_f32.c \
ControllerFunctions/arm_pid_init_q15.c \
ControllerFunctions/arm_pid_init_q31.c \
ControllerFunctions/arm_pid_reset_f32.c \
ControllerFunctions/arm_pid_reset_q15.c \
ControllerFunctions/arm_pid_reset_q31.c \
ControllerFunctions/arm_sin_cos_f32.c \
ControllerFunctions/arm_sin_cos_q31.c \
FastMathFunctions/arm_cos_f32.c \
FastMathFunctions/arm_cos_q15.c \
FastMathFunctions/arm_cos_q31.c \
FastMathFunctions/arm_sin_f32.c \
FastMathFunctions/arm_sin_q15.c \
FastMathFunctions/arm_sin_q31.c \
FastMathFunctions/arm_sqrt_q15.c \
FastMathFunctions/arm_sqrt_q31.c \
FilteringFunctions/arm_biquad_cascade_df1_32x64_init_q31.c \
FilteringFunctions/arm_biquad_cascade_df1_32x64_q31.c \
FilteringFunctions/arm_biquad_cascade_df1_f32.c \
FilteringFunctions/arm_biquad_cascade_df1_fast_q15.c \
FilteringFunctions/arm_biquad_cascade_df1_fast_q31.c \
FilteringFunctions/arm_biquad_cascade_df1_init_f32.c \
FilteringFunctions/arm_biquad_cascade_df1_init_q15.c \
FilteringFunctions/arm_biquad_cascade_df1_init_q31.c \
FilteringFunctions/arm_biquad_cascade_df1_q15.c \
FilteringFunctions/arm_biquad_cascade_df1_q31.c \
FilteringFunctions/arm_biquad_cascade_df2T_f32.c \
FilteringFunctions/arm_biquad_cascade_df2T_init_f32.c \
FilteringFunctions/arm_conv_f32.c \
FilteringFunctions/arm_conv_fast_q15.c \
FilteringFunctions/arm_conv_fast_q31.c \
FilteringFunctions/arm_conv_partial_f32.c \
FilteringFunctions/arm_conv_partial_fast_q15.c \
FilteringFunctions/arm_conv_partial_fast_q31.c \
FilteringFunctions/arm_conv_partial_q15.c \
FilteringFunctions/arm_conv_partial_q31.c \
FilteringFunctions/arm_conv_partial_q7.c \
FilteringFunctions/arm_conv_q15.c \
FilteringFunctions/arm_conv_q31.c \
FilteringFunctions/arm_conv_q7.c \
FilteringFunctions/arm_correlate_f32.c \
FilteringFunctions/arm_correlate_fast_q15.c \
FilteringFunctions/arm_correlate_fast_q31.c \
FilteringFunctions/arm_correlate_q15.c \
FilteringFunctions/arm_correlate_q31.c \
FilteringFunctions/arm_correlate_q7.c \
FilteringFunctions/arm_fir_decimate_f32.c \
FilteringFunctions/arm_fir_decimate_fast_q15.c \
FilteringFunctions/arm_fir_decimate_fast_q31.c \
FilteringFunctions/arm_fir_decimate_init_f32.c \
FilteringFunctions/arm_fir_decimate_init_q15.c \
FilteringFunctions/arm_fir_decimate_init_q31.c \
FilteringFunctions/arm_fir_decimate_q15.c \
FilteringFunctions/arm_fir_decimate_q31.c \
FilteringFunctions/arm_fir_f32.c \
FilteringFunctions/arm_fir_fast_q15.c \
FilteringFunctions/arm_fir_fast_q31.c \
FilteringFunctions/arm_fir_init_f32.c \
FilteringFunctions/arm_fir_init_q15.c \
FilteringFunctions/arm_fir_init_q31.c \
FilteringFunctions/arm_fir_init_q7.c \
FilteringFunctions/arm_fir_interpolate_f32.c \
FilteringFunctions/arm_fir_interpolate_init_f32.c \
FilteringFunctions/arm_fir_interpolate_init_q15.c \
FilteringFunctions/arm_fir_interpolate_init_q31.c \
FilteringFunctions/arm_fir_interpolate_q15.c \
FilteringFunctions/arm_fir_interpolate_q31.c \
FilteringFunctions/arm_fir_lattice_f32.c \
FilteringFunctions/arm_fir_lattice_init_f32.c \
FilteringFunctions/arm_fir_lattice_init_q15.c \
FilteringFunctions/arm_fir_lattice_init_q31.c \
FilteringFunctions/arm_fir_lattice_q15.c \
FilteringFunctions/arm_fir_lattice_q31.c \
FilteringFunctions/arm_fir_q15.c \
FilteringFunctions/arm_fir_q31.c \
FilteringFunctions/arm_fir_q7.c \
FilteringFunctions/arm_fir_sparse_f32.c \
FilteringFunctions/arm_fir_sparse_init_f32.c \
FilteringFunctions/arm_fir_sparse_init_q15.c \
FilteringFunctions/arm_fir_sparse_init_q31.c \
FilteringFunctions/arm_fir_sparse_init_q7.c \
FilteringFunctions/arm_fir_sparse_q15.c \
FilteringFunctions/arm_fir_sparse_q31.c \
FilteringFunctions/arm_fir_sparse_q7.c \
FilteringFunctions/arm_iir_lattice_f32.c \
FilteringFunctions/arm_iir_lattice_init_f32.c \
FilteringFunctions/arm_iir_lattice_init_q15.c \
FilteringFunctions/arm_iir_lattice_init_q31.c \
FilteringFunctions/arm_iir_lattice_q15.c \
FilteringFunctions/arm_iir_lattice_q31.c \
FilteringFunctions/arm_lms_f32.c \
FilteringFunctions/arm_lms_init_f32.c \
FilteringFunctions/arm_lms_init_q15.c \
FilteringFunctions/arm_lms_init_q31.c \
FilteringFunctions/arm_lms_norm_f32.c \
FilteringFunctions/arm_lms_norm_init_f32.c \
FilteringFunctions/arm_lms_norm_init_q15.c \
FilteringFunctions/arm_lms_norm_init_q31.c \
FilteringFunctions/arm_lms_norm_q15.c \
FilteringFunctions/arm_lms_norm_q31.c \
FilteringFunctions/arm_lms_q15.c \
FilteringFunctions/arm_lms_q31.c \
MatrixFunctions/arm_mat_add_f32.c \
MatrixFunctions/arm_mat_add_q15.c \
MatrixFunctions/arm_mat_add_q31.c \
MatrixFunctions/arm_mat_init_f32.c \
MatrixFunctions/arm_mat_init_q15.c \
MatrixFunctions/arm_mat_init_q31.c \
MatrixFunctions/arm_mat_inverse_f32.c \
MatrixFunctions/arm_mat_mult_f32.c \
MatrixFunctions/arm_mat_mult_fast_q15.c \
MatrixFunctions/arm_mat_mult_fast_q31.c \
MatrixFunctions/arm_mat_mult_q15.c \
MatrixFunctions/arm_mat_mult_q31.c \
MatrixFunctions/arm_mat_scale_f32.c \
MatrixFunctions/arm_mat_scale_q15.c \
MatrixFunctions/arm_mat_scale_q31.c \
MatrixFunctions/arm_mat_sub_f32.c \
MatrixFunctions/arm_mat_sub_q15.c \
MatrixFunctions/arm_mat_sub_q31.c \
MatrixFunctions/arm_mat_trans_f32.c \
MatrixFunctions/arm_mat_trans_q15.c \
MatrixFunctions/arm_mat_trans_q31.c \
StatisticsFunctions/arm_max_f32.c \
StatisticsFunctions/arm_max_q15.c \
StatisticsFunctions/arm_max_q31.c \
StatisticsFunctions/arm_max_q7.c \
StatisticsFunctions/arm_mean_f32.c \
StatisticsFunctions/arm_mean_q15.c \
StatisticsFunctions/arm_mean_q31.c \
StatisticsFunctions/arm_mean_q7.c \
StatisticsFunctions/arm_min_f32.c \
StatisticsFunctions/arm_min_q15.c \
StatisticsFunctions/arm_min_q31.c \
StatisticsFunctions/arm_min_q7.c \
StatisticsFunctions/arm_power_f32.c \
StatisticsFunctions/arm_power_q15.c \
StatisticsFunctions/arm_power_q31.c \
StatisticsFunctions/arm_power_q7.c \
StatisticsFunctions/arm_rms_f32.c \
StatisticsFunctions/arm_rms_q15.c \
StatisticsFunctions/arm_rms_q31.c \
StatisticsFunctions/arm_std_f32.c \
StatisticsFunctions/arm_std_q15.c \
StatisticsFunctions/arm_std_q31.c \
StatisticsFunctions/arm_var_f32.c \
StatisticsFunctions/arm_var_q15.c \
StatisticsFunctions/arm_var_q31.c \
SupportFunctions/arm_copy_f32.c \
SupportFunctions/arm_copy_q15.c \
SupportFunctions/arm_copy_q31.c \
SupportFunctions/arm_copy_q7.c \
SupportFunctions/arm_fill_f32.c \
SupportFunctions/arm_fill_q15.c \
SupportFunctions/arm_fill_q31.c \
SupportFunctions/arm_fill_q7.c \
SupportFunctions/arm_float_to_q15.c \
SupportFunctions/arm_float_to_q31.c \
SupportFunctions/arm_float_to_q7.c \
SupportFunctions/arm_q15_to_float.c \
SupportFunctions/arm_q15_to_q31.c \
SupportFunctions/arm_q15_to_q7.c \
SupportFunctions/arm_q31_to_float.c \
SupportFunctions/arm_q31_to_q15.c \
SupportFunctions/arm_q31_to_q7.c \
SupportFunctions/arm_q7_to_float.c \
SupportFunctions/arm_q7_to_q15.c \
SupportFunctions/arm_q7_to_q31.c \
TransformFunctions/arm_cfft_radix4_f32.c \
TransformFunctions/arm_cfft_radix4_init_f32.c \
TransformFunctions/arm_cfft_radix4_init_q15.c \
TransformFunctions/arm_cfft_radix4_init_q31.c \
TransformFunctions/arm_cfft_radix4_q15.c \
TransformFunctions/arm_cfft_radix4_q31.c \
TransformFunctions/arm_dct4_f32.c \
TransformFunctions/arm_dct4_init_f32.c \
TransformFunctions/arm_dct4_init_q15.c \
TransformFunctions/arm_dct4_init_q31.c \
TransformFunctions/arm_dct4_q15.c \
TransformFunctions/arm_dct4_q31.c \
TransformFunctions/arm_rfft_f32.c \
TransformFunctions/arm_rfft_init_f32.c \
TransformFunctions/arm_rfft_init_q15.c \
TransformFunctions/arm_rfft_init_q31.c \
TransformFunctions/arm_rfft_q15.c \
TransformFunctions/arm_rfft_q31.c

ASRC =



# Optimization level, can be [0, 1, 2, 3, s]. 
# 0 = turn off optimization. s = optimize for size.
OPT = s

# Debugging format.
# Native formats for AVR-GCC's -g are stabs [default], or dwarf-2.
# AVR (extended) COFF requires stabs, plus an avr-objcopy run.
DEBUG =

# List any extra directories to look for include files here.
#     Each directory must be seperated by a space.
EXTRAINCDIRS  = ../../Include

# Compiler flag to set the C Standard level.
# c89   - "ANSI" C
# gnu89 - c89 plus GCC extensions
# c99   - ISO C99 standard (not yet fully implemented)
# gnu99 - c99 plus GCC extensions
CSTANDARD =

# Place -D or -U options here
#CDEFS = -DBUILD=0x`date '+%Y%m%d'` -DARM_MATH_CM4 -D__FPU_PRESENT

# Place -I options here
CINCS =


# Compiler flags.
#  -g*:          generate debugging information
#  -O*:          optimization level
#  -f...:        tuning, see GCC manual and avr-libc documentation
#  -Wall...:     warning level
#  -Wa,...:      tell GCC to pass this to the assembler.
#    -adhlns...: create assembler listing
CFLAGS = -g$(DEBUG)
CFLAGS += $(CDEFS) $(CINCS)
CFLAGS += -O$(OPT)
#CFLAGS += -funsigned-char -funsigned-bitfields -fpack-struct -fshort-enums
#CFLAGS += -Wall -Wstrict-prototypes
#CFLAGS += -Wa,-adhlns=$(<:.c=.lst)
CFLAGS += $(patsubst %,-I%,$(EXTRAINCDIRS))
CFLAGS += $(CSTANDARD)



# Assembler flags.
#  -Wa,...:   tell GCC to pass this to the assembler.
#  -ahlms:    create listing
#  -gstabs:   have the assembler create line number information; note that
#             for use in COFF files, additional information about filenames
#             and function names needs to be present in the assembler source
#             files -- see avr-libc docs [FIXME: not yet described there]
#ASFLAGS = -Wa,-adhlns=$(<:.S=.lst),-gstabs 
ASFLAGS = -Wa,-gstabs



# Define programs and commands.
SHELL = sh
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy
OBJDUMP = arm-none-eabi-objdump
AR = arm-none-eabi-ar
SIZE = arm-none-eabi-size
NM = arm-none-eabi-nm
REMOVE = rm -f
COPY = cp
YACC = bison
LEX = flex


# Define all object files.
OBJ = $(SRC:.c=.o) $(ASRC:.S=.o) 

# Define all listing files.
LST = $(ASRC:.S=.lst) $(SRC:.c=.lst)


# Compiler flags to generate dependency files.
#GENDEPFLAGS = -Wp,-M,-MP,-MT,$(*F).o,-MF,.dep/$(@F).d


# Combine all necessary flags and optional flags.
# Add target processor to flags.
ALL_CFLAGS = $(MCU) -I. $(CFLAGS) $(GENDEPFLAGS)
#ALL_ASFLAGS = -mcpu=$(MCU) -I. -x assembler-with-cpp $(ASFLAGS)
ALL_ASFLAGS = $(MCU) -I. -x assembler-with-cpp $(ASFLAGS)





# Default target.
all: $(TARGET)


$(TARGET): $(OBJ)
	$(AR) rcu $(TARGET) $(OBJ)


# Eye candy.
# AVR Studio 3.x does not check make's exit code but relies on
# the following magic strings to be generated by the compile job.




# Display compiler version information.
gccversion : 
	$(CC) --version


# Create final output files (.hex, .eep) from ELF output file.
%.hex: %.elf
	@echo
	@echo $(MSG_FLASH) $@
	$(OBJCOPY) -O $(FORMAT) -R .eeprom $< $@

%.eep: %.elf
	@echo
	@echo $(MSG_EEPROM) $@
	-$(OBJCOPY) -j .eeprom --set-section-flags=.eeprom="alloc,load" \
	--change-section-lma .eeprom=0 -O $(FORMAT) $< $@

# Create extended listing file from ELF output file.
%.lss: %.elf
	@echo
	@echo $(MSG_EXTENDED_LISTING) $@
	$(OBJDUMP) -h -S $< > $@

# Create a symbol table from ELF output file.
%.sym: %.elf
	@echo
	@echo $(MSG_SYMBOL_TABLE) $@
	$(NM) -n $< > $@



# Link: create ELF output file from object files.
.SECONDARY : $(TARGET).elf
.PRECIOUS : $(OBJ)
%.elf: $(OBJ)
	@echo
	@echo $(MSG_LINKING) $@
	$(CC) $(ALL_CFLAGS) $(OBJ) --output $@ $(LDFLAGS)


# Compile: create object files from C source files.
%.o : %.c
	@echo
	@echo $(MSG_COMPILING) $<
	$(CC) -c $(ALL_CFLAGS) $< -o $@ 


# Compile: create assembler files from C source files.
%.s : %.c
	$(CC) -S $(ALL_CFLAGS) $< -o $@


# Assemble: create object files from assembler source files.
%.o : %.S
	@echo
	@echo $(MSG_ASSEMBLING) $<
	$(CC) -c $(ALL_ASFLAGS) $< -o $@



# Target: clean project.
clean:
	$(REMOVE) $(TARGET)
	$(REMOVE) $(OBJ)
	$(REMOVE) $(LST)
	$(REMOVE) .dep/*


# Listing of phony targets.
.PHONY : all sizebefore sizeafter gccversion \
build elf hex eep lss sym coff extcoff \
clean clean_list program

