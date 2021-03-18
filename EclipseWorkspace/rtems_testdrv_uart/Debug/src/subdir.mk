################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/rs422write.c 

OBJS += \
./src/rs422write.o 

C_DEPS += \
./src/rs422write.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross GCC Compiler'
	sparc-gaisler-rtems5-gcc -I/opt/rcc-1.3.0-gcc/sparc-gaisler-rtems5/gr712rc/lib/include -O0 -g3 -Wall -c -fmessage-length=0 -mcpu=leon3 -mfix-gr712rc -qbsp=gr712rc -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


