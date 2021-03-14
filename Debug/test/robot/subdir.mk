################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../test/robot/RobotDecisionMakers.cc \
../test/robot/main.cc 

CC_DEPS += \
./test/robot/RobotDecisionMakers.d \
./test/robot/main.d 

OBJS += \
./test/robot/RobotDecisionMakers.o \
./test/robot/main.o 


# Each subdirectory must supply rules for building sources it contributes
test/robot/%.o: ../test/robot/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/san/apps/build/debug-install-cpp11/include -I"/home/san/eclipse-workspace/ratel/landlords-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


