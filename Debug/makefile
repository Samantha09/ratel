################################################################################
# Automatically-generated file. Do not edit!
################################################################################

-include ../makefile.init

RM := rm -rf

# All of the sources participating in the build are defined here
-include sources.mk
-include test/robot/subdir.mk
-include test/helper/serialization/subdir.mk
-include test/helper/subdir.mk
-include test/event/subdir.mk
-include test/entity/subdir.mk
-include test/boostt/subdir.mk
-include test/subdir.mk
-include landlords-server/robot/subdir.mk
-include landlords-server/event/subdir.mk
-include landlords-server/subdir.mk
-include landlords-common/robot/subdir.mk
-include landlords-common/protobuf/subdir.mk
-include landlords-common/helper/subdir.mk
-include landlords-common/enums/subdir.mk
-include landlords-common/entity/subdir.mk
-include landlords-common/backup/subdir.mk
-include landlords-common/back/subdir.mk
-include landlords-client/event/subdir.mk
-include landlords-client/subdir.mk
-include subdir.mk
-include objects.mk

ifneq ($(MAKECMDGOALS),clean)
ifneq ($(strip $(CC_DEPS)),)
-include $(CC_DEPS)
endif
ifneq ($(strip $(C++_DEPS)),)
-include $(C++_DEPS)
endif
ifneq ($(strip $(C_UPPER_DEPS)),)
-include $(C_UPPER_DEPS)
endif
ifneq ($(strip $(CXX_DEPS)),)
-include $(CXX_DEPS)
endif
ifneq ($(strip $(CPP_DEPS)),)
-include $(CPP_DEPS)
endif
ifneq ($(strip $(C_DEPS)),)
-include $(C_DEPS)
endif
endif

-include ../makefile.defs

# Add inputs and outputs from these tool invocations to the build variables 

# All Target
all: ratel

# Tool invocations
ratel: $(OBJS) $(USER_OBJS)
	@echo 'Building target: $@'
	@echo 'Invoking: GCC C++ Linker'
	g++  -o "ratel" $(OBJS) $(USER_OBJS) $(LIBS)
	@echo 'Finished building target: $@'
	@echo ' '

# Other Targets
clean:
	-$(RM) $(CC_DEPS)$(C++_DEPS)$(EXECUTABLES)$(C_UPPER_DEPS)$(CXX_DEPS)$(OBJS)$(CPP_DEPS)$(C_DEPS) ratel
	-@echo ' '

.PHONY: all clean dependents

-include ../makefile.targets
