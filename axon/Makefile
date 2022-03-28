PROJECT_ROOT_PATH := $(abspath $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST))))))
include $(PROJECT_ROOT_PATH)/../common.mk

# add Nordic OpenThread Defaults overlay
OT_OVERLAY := $(COMMON_ROOT_PATH)/overlays/overlay-ot.conf
# insert prj.conf again to reset some of the previous overrides
OT_OVERLAY := $(OT_OVERLAY);$(PROJECT_ROOT_PATH)/prj.conf
# add Hyper OpenThread overlay
OT_OVERLAY := $(OT_OVERLAY);$(COMMON_ROOT_PATH)/overlays/overlay-hyper-ot.conf
# nRF5 specific overlay
NRF5_OVERLAY := $(PROJECT_ROOT_PATH)/overlay-nrf5.conf
# default overlays
OVERLAY_CONFIG_DEFAULT = $(OT_OVERLAY);$(NRF5_OVERLAY)

.PHONY: build
build: check_deps
	$(call build_env,\
		west build -b $(TARGET_BOARD) -- \
			-DOVERLAY_CONFIG="$(OVERLAY_CONFIG_DEFAULT);overlay-smp.conf;overlay-dev.conf" \
	)

.PHONY: build-rtt
build-rtt: check_deps
	$(call build_env,\
		west build -b $(TARGET_BOARD) -- \
			-DOVERLAY_CONFIG="$(OVERLAY_CONFIG_DEFAULT);overlay-smp.conf;overlay-dev.conf;overlay-rtt.conf" \
	)

.PHONY: build-prod
build-prod: check_deps
	$(call build_env,\
		west build -b $(TARGET_BOARD) -- \
			-DOVERLAY_CONFIG="$(OVERLAY_CONFIG_DEFAULT);overlay-smp.conf;overlay-prod.conf" \
	)

.PHONY: build-without-smp
build-without-smp: check_deps
	$(call build_env,\
		west build -b $(TARGET_BOARD) -- \
			-DOVERLAY_CONFIG="$(OVERLAY_CONFIG_DEFAULT);overlay-dev.conf" \
	)
