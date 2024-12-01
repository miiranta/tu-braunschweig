# name of your application
APPLICATION = project2
PROJECT_BASE = $(CURDIR)

# If no BOARD is found in the environment, use this default:
BOARD ?= native

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(PROJECT_BASE)/../RIOT

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

USEMODULE += udp
USEMODULE += enums
USEMODULE += msp
USEMODULE += triggerTimer
USEMODULE += motor
USEMODULE += pathGenerator
USEMODULE += ultrassonic

DIRS += modules/udp
DIRS += modules/enums
DIRS += modules/msp
DIRS += modules/triggerTimer
DIRS += modules/motor
DIRS += modules/pathGenerator
DIRS += modules/ultrassonic

USERMODULE += shell
USERMODULE += shell_commands
USERMODULE += ps
USEMODULE += gnrc_netdev_default
USEMODULE += auto_init_gnrc_netif
USEMODULE += gnrc_ipv6_default
USEMODULE += gnrc_icmpv6_echo
USEMODULE += gnrc_sock_udp
USEMODULE += ztimer
USEMODULE += periph_adc

include $(RIOTBASE)/Makefile.include
