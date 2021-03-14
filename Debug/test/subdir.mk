################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CC_SRCS += \
../test/PokerBasic.cc \
../test/test1.cc \
../test/test2.cc \
../test/test3.cc \
../test/test4.cc \
../test/test5.cc \
../test/test_json.cc \
../test/test_serialize_pokers.cc \
../test/test_set_vector.cc 

CC_DEPS += \
./test/PokerBasic.d \
./test/test1.d \
./test/test2.d \
./test/test3.d \
./test/test4.d \
./test/test5.d \
./test/test_json.d \
./test/test_serialize_pokers.d \
./test/test_set_vector.d 

OBJS += \
./test/PokerBasic.o \
./test/test1.o \
./test/test2.o \
./test/test3.o \
./test/test4.o \
./test/test5.o \
./test/test_json.o \
./test/test_serialize_pokers.o \
./test/test_set_vector.o 


# Each subdirectory must supply rules for building sources it contributes
test/%.o: ../test/%.cc
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I/home/san/apps/build/debug-install-cpp11/include -I"/home/san/eclipse-workspace/ratel/landlords-common" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


