################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../landlords-common/backup/query.pb.cc 

CC_DEPS += \
./landlords-common/backup/query.pb.d 

OBJS += \
./landlords-common/backup/query.pb.o 


# Each subdirectory must supply rules for building sources it contributes
landlords-common/backup/%.o: ../landlords-common/backup/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/san/apps/build/debug-install-cpp11/include -I"/home/san/eclipse-workspace/ratel/landlords-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


