################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Middlewares/littlefs/lfs.c \
../Middlewares/littlefs/lfs_util.c 

OBJS += \
./Middlewares/littlefs/lfs.o \
./Middlewares/littlefs/lfs_util.o 

C_DEPS += \
./Middlewares/littlefs/lfs.d \
./Middlewares/littlefs/lfs_util.d 


# Each subdirectory must supply rules for building sources it contributes
Middlewares/littlefs/%.o Middlewares/littlefs/%.su Middlewares/littlefs/%.cyclo: ../Middlewares/littlefs/%.c Middlewares/littlefs/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -DUSE_HAL_DRIVER -DSTM32F072xB -c -I../Core/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -I../USB_DEVICE/App -I../USB_DEVICE/Target -I../Middlewares/ST/STM32_USB_Device_Library/Core/Inc -I../Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc -I"/home/andrew/STM32CubeIDE/workspace_1.6.0/GPSRecorder/Middlewares/littlefs" -Os -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Middlewares-2f-littlefs

clean-Middlewares-2f-littlefs:
	-$(RM) ./Middlewares/littlefs/lfs.cyclo ./Middlewares/littlefs/lfs.d ./Middlewares/littlefs/lfs.o ./Middlewares/littlefs/lfs.su ./Middlewares/littlefs/lfs_util.cyclo ./Middlewares/littlefs/lfs_util.d ./Middlewares/littlefs/lfs_util.o ./Middlewares/littlefs/lfs_util.su

.PHONY: clean-Middlewares-2f-littlefs

