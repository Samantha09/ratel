################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../test/helper/Poker.cc \
../test/helper/PokerBasic.cc \
../test/helper/PokerHelper.cc \
../test/helper/PokerSell.cc \
../test/helper/test_checkPokerType.cc \
../test/helper/test_distribute.cc 

CC_DEPS += \
./test/helper/Poker.d \
./test/helper/PokerBasic.d \
./test/helper/PokerHelper.d \
./test/helper/PokerSell.d \
./test/helper/test_checkPokerType.d \
./test/helper/test_distribute.d 

OBJS += \
./test/helper/Poker.o \
./test/helper/PokerBasic.o \
./test/helper/PokerHelper.o \
./test/helper/PokerSell.o \
./test/helper/test_checkPokerType.o \
./test/helper/test_distribute.o 


# Each subdirectory must supply rules for building sources it contributes
test/helper/%.o: ../test/helper/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/san/apps/build/debug-install-cpp11/include -I"/home/san/eclipse-workspace/ratel/landlords-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


