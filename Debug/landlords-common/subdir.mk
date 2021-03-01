################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../landlords-common/Poker.cc 

CC_DEPS += \
./landlords-common/Poker.d 

OBJS += \
./landlords-common/Poker.o 


# Each subdirectory must supply rules for building sources it contributes
landlords-common/%.o: ../landlords-common/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/san/apps/build/debug-install-cpp11/include -I"/home/san/eclipse-workspace/ratel/landlords-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


