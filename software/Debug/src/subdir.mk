################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/filter.c \
../src/gps_recorder_device.c \
../src/gps_recorder_host.c \
../src/persistence.c 

C_DEPS += \
./src/filter.d \
./src/gps_recorder_device.d \
./src/gps_recorder_host.d \
./src/persistence.d 

OBJS += \
./src/filter.o \
./src/gps_recorder_device.o \
./src/gps_recorder_host.o \
./src/persistence.o 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I"/home/andrew/STM32CubeIDE/workspace_1.6.0/GPSRecorderHost/inc" -O0 -g3 -Wall -c -fmessage-length=0   `pkg-config --cflags gtk+-3.0` `pkg-config --cflags osmgpsmap-1.0` -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


clean: clean-src

clean-src:
	-$(RM) ./src/filter.d ./src/filter.o ./src/gps_recorder_device.d ./src/gps_recorder_device.o ./src/gps_recorder_host.d ./src/gps_recorder_host.o ./src/persistence.d ./src/persistence.o

.PHONY: clean-src

