################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../landlords-common/entity/Poker.cc \
../landlords-common/entity/PokerSell.cc \
../landlords-common/entity/Room.cc 

CC_DEPS += \
./landlords-common/entity/Poker.d \
./landlords-common/entity/PokerSell.d \
./landlords-common/entity/Room.d 

OBJS += \
./landlords-common/entity/Poker.o \
./landlords-common/entity/PokerSell.o \
./landlords-common/entity/Room.o 


# Each subdirectory must supply rules for building sources it contributes
landlords-common/entity/%.o: ../landlords-common/entity/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/san/apps/build/debug-install-cpp11/include -I"/home/san/eclipse-workspace/ratel/landlords-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


