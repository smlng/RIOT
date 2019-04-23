include ../Makefile.tests_common

PARAM_GPIO  ?= GPIO_PIN\(0,14\)
CFLAGS += -DTFA_THW_PARAM_GPIO=$(PARAM_GPIO)

USEMODULE += tfa_thw
USEMODULE += fmt

include $(RIOTBASE)/Makefile.include
