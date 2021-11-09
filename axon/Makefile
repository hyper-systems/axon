PROJECT_ROOT_PATH := $(abspath $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST))))))
include $(PROJECT_ROOT_PATH)/../common.mk

OVERLAY_CONFIG_DEFAULT = $(COMMON_ROOT_PATH)/overlays/overlay-hyper-ot.conf;overlay-nrf5.conf

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
