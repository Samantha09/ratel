################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../landlords-server/robot/RobotEventFuncs.cc \
../landlords-server/robot/RobotEventListener.cc 

CC_DEPS += \
./landlords-server/robot/RobotEventFuncs.d \
./landlords-server/robot/RobotEventListener.d 

OBJS += \
./landlords-server/robot/RobotEventFuncs.o \
./landlords-server/robot/RobotEventListener.o 


# Each subdirectory must supply rules for building sources it contributes
landlords-server/robot/%.o: ../landlords-server/robot/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/san/apps/build/debug-install-cpp11/include -I"/home/san/eclipse-workspace/ratel/landlords-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


