# Makefile for libusb based examples
BTSTACK_ROOT ?= ../..

CORE += \
	btstack_chipset_da14581.c \
	btstack_link_key_db_fs.c \
	btstack_run_loop_posix.c \
	btstack_tlv_posix.c \
	btstack_uart_block_posix.c \
	hci_transport_h4.c \
	le_device_db_fs.c \
	main.c \
	wav_util.c 					\
	btstack_stdin_posix.c \
	hci_585.c \
	# hci_581_active_uart.c \

# examples
include ${BTSTACK_ROOT}/example/Makefile.inc

CFLAGS  += -g -Wall -Werror \
	-I$(BTSTACK_ROOT)/platform/posix \
	-I$(BTSTACK_ROOT)/chipset/da14581 \
	-I$(BTSTACK_ROOT)/platform/embedded \
	-I${BTSTACK_ROOT}/3rd-party/tinydir

VPATH += ${BTSTACK_ROOT}/platform/posix
VPATH += ${BTSTACK_ROOT}/chipset/da14581

ifeq ($(OS),Windows_NT)
LDFLAGS += -lws2_32
endif

# Command Line examples require porting to win32, so only build on other unix-ish hosts
ifneq ($(OS),Windows_NT)
EXAMPLES += ${EXAMPLES_CLI}
endif

# use pkg-config for portaudio
# CFLAGS  += $(shell pkg-config portaudio-2.0 --cflags) -DHAVE_PORTAUDIO
# LDFLAGS += $(shell pkg-config portaudio-2.0 --libs)
# hard coded flags for portaudio in /usr/local/lib
# CFLAGS  += -I/usr/local/include -DHAVE_PORTAUDIO
# LDFLAGS += -L/sw/lib -lportaudio -Wl,-framework,CoreAudio -Wl,-framework,AudioToolbox -Wl,-framework,AudioUnit -Wl,-framework,Carbon

all: ${EXAMPLES}

