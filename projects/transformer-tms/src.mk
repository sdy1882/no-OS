# Project sources
SRCS += $(PROJECT)/src/app/main.c \
	$(PROJECT)/src/app/transformer_tms.c

INCS += $(PROJECT)/src/app/parameters.h \
	$(PROJECT)/src/app/transformer_tms.h

# no-OS drivers includes
INCS += $(INCLUDE)/no_os_delay.h     \
	$(INCLUDE)/no_os_error.h     \
	$(INCLUDE)/no_os_gpio.h      \
	$(INCLUDE)/no_os_print_log.h \
	$(INCLUDE)/no_os_spi.h       \
	$(INCLUDE)/no_os_uart.h      \
	$(INCLUDE)/no_os_util.h      \
	$(INCLUDE)/no_os_units.h     \
	$(INCLUDE)/no_os_alloc.h

# ADE9430 driver
INCS += $(DRIVERS)/meter/ade9430/ade9430.h
SRCS += $(DRIVERS)/meter/ade9430/ade9430.c

# Platform specific includes
INCS += $(PLATFORM_DRIVERS)/$(PLATFORM)_delay.h     \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_gpio.h      \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_spi.h       \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_uart.h      \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_uart_stdio.h

# no-OS API drivers
SRCS += $(DRIVERS)/api/no_os_gpio.c  \
	$(DRIVERS)/api/no_os_spi.c   \
	$(DRIVERS)/api/no_os_uart.c  \
	$(NO-OS)/util/no_os_util.c   \
	$(NO-OS)/util/no_os_alloc.c

# Platform specific sources
SRCS += $(PLATFORM_DRIVERS)/$(PLATFORM)_delay.c     \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_gpio.c      \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_spi.c       \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_uart.c      \
	$(PLATFORM_DRIVERS)/$(PLATFORM)_uart_stdio.c
