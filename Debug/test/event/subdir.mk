################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../test/event/Base.cc \
../test/event/main.cc 

CC_DEPS += \
./test/event/Base.d \
./test/event/main.d 

OBJS += \
./test/event/Base.o \
./test/event/main.o 


# Each subdirectory must supply rules for building sources it contributes
test/event/%.o: ../test/event/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/san/apps/build/debug-install-cpp11/include -I"/home/san/eclipse-workspace/ratel/landlords-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


