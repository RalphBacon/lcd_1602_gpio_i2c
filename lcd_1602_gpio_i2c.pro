
TEMPLATE = app
TARGET = lcd_1602_gpio_i2c
INCLUDEPATH += $(HOME)/pico/pico-sdk/src/common/pico_base/include
INCLUDEPATH += $(HOME)/pico/pico-sdk/src/common/pico_binary_info/include
INCLUDEPATH += $(HOME)/pico/pico-sdk/src/common/pico_stdlib/include
INCLUDEPATH += $(HOME)/pico/pico-sdk/src/common/pico_time/include
INCLUDEPATH += $(HOME)/pico/pico-sdk/src/rp2_common/hardware_base/include
INCLUDEPATH += $(HOME)/pico/pico-sdk/src/rp2_common/hardware_gpio/include
INCLUDEPATH += $(HOME)/pico/pico-sdk/src/rp2_common/hardware_i2c/include
INCLUDEPATH += $(HOME)/pico/pico-sdk/src/rp2_common/hardware_uart/include
INCLUDEPATH += $(HOME)/pico/pico-sdk/src/rp2_common/pico_stdio/include

SOURCES += \
	main.cpp

HEADERS +=
