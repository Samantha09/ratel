################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../landlords-common/robot/RobotDecisionMakers.cc 

CC_DEPS += \
./landlords-common/robot/RobotDecisionMakers.d 

OBJS += \
./landlords-common/robot/RobotDecisionMakers.o 


# Each subdirectory must supply rules for building sources it contributes
landlords-common/robot/%.o: ../landlords-common/robot/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/san/apps/build/debug-install-cpp11/include -I"/home/san/eclipse-workspace/ratel/landlords-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


