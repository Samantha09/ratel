################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../landlords-common/protobuf/codec.cc \
../landlords-common/protobuf/dispatcher_lite_test.cc \
../landlords-common/protobuf/dispatcher_test.cc \
../landlords-common/protobuf/query.pb.cc 

CC_DEPS += \
./landlords-common/protobuf/codec.d \
./landlords-common/protobuf/dispatcher_lite_test.d \
./landlords-common/protobuf/dispatcher_test.d \
./landlords-common/protobuf/query.pb.d 

OBJS += \
./landlords-common/protobuf/codec.o \
./landlords-common/protobuf/dispatcher_lite_test.o \
./landlords-common/protobuf/dispatcher_test.o \
./landlords-common/protobuf/query.pb.o 


# Each subdirectory must supply rules for building sources it contributes
landlords-common/protobuf/%.o: ../landlords-common/protobuf/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/san/apps/build/debug-install-cpp11/include -I"/home/san/eclipse-workspace/ratel/landlords-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


