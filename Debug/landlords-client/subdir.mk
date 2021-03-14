################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../landlords-client/SimpleClient.cc \
../landlords-client/client.cc \
../landlords-client/clinet_test.cc \
../landlords-client/main.cc 

CC_DEPS += \
./landlords-client/SimpleClient.d \
./landlords-client/client.d \
./landlords-client/clinet_test.d \
./landlords-client/main.d 

OBJS += \
./landlords-client/SimpleClient.o \
./landlords-client/client.o \
./landlords-client/clinet_test.o \
./landlords-client/main.o 


# Each subdirectory must supply rules for building sources it contributes
landlords-client/%.o: ../landlords-client/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/san/apps/build/debug-install-cpp11/include -I"/home/san/eclipse-workspace/ratel/landlords-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


