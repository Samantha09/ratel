################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../landlords-common/helper/PokerHelper.cc 

CPP_SRCS += \
../landlords-common/helper/MapHelper.cpp 

CC_DEPS += \
./landlords-common/helper/PokerHelper.d 

OBJS += \
./landlords-common/helper/MapHelper.o \
./landlords-common/helper/PokerHelper.o 

CPP_DEPS += \
./landlords-common/helper/MapHelper.d 


# Each subdirectory must supply rules for building sources it contributes
landlords-common/helper/%.o: ../landlords-common/helper/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/san/apps/build/debug-install-cpp11/include -I"/home/san/eclipse-workspace/ratel/landlords-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '

landlords-common/helper/%.o: ../landlords-common/helper/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/san/apps/build/debug-install-cpp11/include -I"/home/san/eclipse-workspace/ratel/landlords-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


