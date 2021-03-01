################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../landlords-server/RatelServer.cc \
../landlords-server/main.cc 

CC_DEPS += \
./landlords-server/RatelServer.d \
./landlords-server/main.d 

OBJS += \
./landlords-server/RatelServer.o \
./landlords-server/main.o 


# Each subdirectory must supply rules for building sources it contributes
landlords-server/%.o: ../landlords-server/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/san/apps/build/debug-install-cpp11/include -I"/home/san/eclipse-workspace/ratel/landlords-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


