PROJECT_ROOT_PATH := $(abspath $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST))))))

include $(PROJECT_ROOT_PATH)/../common.mk

NRF_SDK_ROOT_DIR = $(NCS_SRC_PATH)/nrf
COPROCESSOR_SAMPLE_DIR = $(NRF_SDK_ROOT_DIR)/samples/openthread/coprocessor

RCP_OVERLAY := $(COPROCESSOR_SAMPLE_DIR)/overlay-rcp.conf
USB_OVERLAY := $(COPROCESSOR_SAMPLE_DIR)/overlay-usb.conf
LEDS_OVERLAY := $(PROJECT_ROOT_PATH)/overlay-leds.conf
USB_DTC_OVERLAY := -DDTC_OVERLAY_FILE="$(COPROCESSOR_SAMPLE_DIR)/usb.overlay"
# default overlays
OVERLAY_CONFIG_DEFAULT := $(COPROCESSOR_SAMPLE_DIR)/prj.conf;$(RCP_OVERLAY);$(USB_OVERLAY);$(LEDS_OVERLAY)

.PHONY: build
build: check_deps
	$(call build_env,\
		west build -b $(TARGET_BOARD) -- \
			-DOVERLAY_CONFIG="$(OVERLAY_CONFIG_DEFAULT)" \
			$(USB_DTC_OVERLAY) \
	)
