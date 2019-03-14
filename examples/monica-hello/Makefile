# name of your application
APPLICATION = monica-lgv-demo

BOARD_WHITELIST := native pba-d-01-kw2x
# If no BOARD is found in the environment, use this default:
BOARD ?= native
# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../..

# mandatory modules
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
USEMODULE += gnrc_ipv6_default
USEMODULE += gnrc_sock_udp
USEMODULE += gcoap
USEMODULE += xtimer
USEMODULE += fmt
# optional modules, e.g. for debugging
USEMODULE += gnrc_icmpv6_echo

# Add sensor modules, if supported
ifeq ($(BOARD),pba-d-01-kw2x)
	USEMODULE += hdc1000
	USEMODULE += tmp006
endif

CFLAGS += -DGCOAP_REQ_WAITING_MAX=8
CFLAGS += -DGCOAP_NON_TIMEOUT=1000000

# get rid of stack corruption and panics
ifneq ($(BOARD),native)
	CFLAGS += -DTHREAD_STACKSIZE_MAIN=2048
endif
# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1
DEVELHELP ?= 0

include $(RIOTBASE)/Makefile.include
