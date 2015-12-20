APPLICATION = driver_mpu6050
include ../Makefile.tests_common

FEATURES_REQUIRED = periph_i2c

USEMODULE += mpu6050
USEMODULE += xtimer

# define parameters for selected boards
ifneq (,$(filter msbiot,$(BOARD)))
  TEST_I2C       ?= MPU6050_I2C
  TEST_HW_ADDR   ?= MPU6050_HW_ADDR
endif

# set default device parameters in case they are undefined
TEST_I2C       ?= I2C_0
TEST_HW_ADDR   ?= MPU6050_HW_ADDR_HEX_68

# export parameters
CFLAGS += -DTEST_I2C=$(TEST_I2C)
CFLAGS += -DTEST_HW_ADDR=$(TEST_HW_ADDR)

include $(RIOTBASE)/Makefile.include
