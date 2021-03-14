################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../landlords-client/event/ClentEventListener.cc \
../landlords-client/event/eventFuncs.cc 

CC_DEPS += \
./landlords-client/event/ClentEventListener.d \
./landlords-client/event/eventFuncs.d 

OBJS += \
./landlords-client/event/ClentEventListener.o \
./landlords-client/event/eventFuncs.o 


# Each subdirectory must supply rules for building sources it contributes
landlords-client/event/%.o: ../landlords-client/event/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/san/apps/build/debug-install-cpp11/include -I"/home/san/eclipse-workspace/ratel/landlords-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


