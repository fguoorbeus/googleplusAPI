################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../Tinyxml/tinystr.cpp \
../Tinyxml/tinyxml.cpp \
../Tinyxml/tinyxmlerror.cpp \
../Tinyxml/tinyxmlparser.cpp 

OBJS += \
./Tinyxml/tinystr.o \
./Tinyxml/tinyxml.o \
./Tinyxml/tinyxmlerror.o \
./Tinyxml/tinyxmlparser.o 

CPP_DEPS += \
./Tinyxml/tinystr.d \
./Tinyxml/tinyxml.d \
./Tinyxml/tinyxmlerror.d \
./Tinyxml/tinyxmlparser.d 


# Each subdirectory must supply rules for building sources it contributes
Tinyxml/%.o: ../Tinyxml/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: Cross G++ Compiler'
	g++ -O0 -g3 -Wall -c -fmessage-length=0 -std=gnu++0x -DHAVE_CONFIG_H -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


