################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../landlords-server/event/EventFuncs.cc \
../landlords-server/event/ServerContains.cc \
../landlords-server/event/ServerEventListener.cc 

CC_DEPS += \
./landlords-server/event/EventFuncs.d \
./landlords-server/event/ServerContains.d \
./landlords-server/event/ServerEventListener.d 

OBJS += \
./landlords-server/event/EventFuncs.o \
./landlords-server/event/ServerContains.o \
./landlords-server/event/ServerEventListener.o 


# Each subdirectory must supply rules for building sources it contributes
landlords-server/event/%.o: ../landlords-server/event/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/san/apps/build/debug-install-cpp11/include -I"/home/san/eclipse-workspace/ratel/landlords-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


