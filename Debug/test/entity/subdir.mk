################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../test/entity/ClientSide.cc \
../test/entity/Poker.cc \
../test/entity/PokerBasic.cc \
../test/entity/PokerHelper.cc \
../test/entity/PokerSell.cc \
../test/entity/test_clientside.cc 

CC_DEPS += \
./test/entity/ClientSide.d \
./test/entity/Poker.d \
./test/entity/PokerBasic.d \
./test/entity/PokerHelper.d \
./test/entity/PokerSell.d \
./test/entity/test_clientside.d 

OBJS += \
./test/entity/ClientSide.o \
./test/entity/Poker.o \
./test/entity/PokerBasic.o \
./test/entity/PokerHelper.o \
./test/entity/PokerSell.o \
./test/entity/test_clientside.o 


# Each subdirectory must supply rules for building sources it contributes
test/entity/%.o: ../test/entity/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/san/apps/build/debug-install-cpp11/include -I"/home/san/eclipse-workspace/ratel/landlords-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


