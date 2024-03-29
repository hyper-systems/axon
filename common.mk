SHELL := /bin/bash
COMMON_ROOT_PATH := $(abspath $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST))))))
VENV_PATH = $(COMMON_ROOT_PATH)/.venv
NCS_SRC_PATH = $(COMMON_ROOT_PATH)/ncs
NRF_CONNECT_SDK_REPO = https://github.com/nrfconnect/sdk-nrf
NRF_CONNECT_SDK_TAG = v1.9.1
GNUARMEMB_TOOLCHAIN_VERSION ?= 9-2019-q4-major
UNAME_S := $(shell uname -s)

ifeq ($(GNUARMEMB_TOOLCHAIN_PATH),)
ifeq ($(UNAME_S),Darwin)
GNUARMEMB_TOOLCHAIN_PATH = $(shell brew --prefix)/opt/arm-gcc-bin@$(GNUARMEMB_TOOLCHAIN_VERSION)
else ifeq ($(UNAME_S),Linux)
$(error GNUARMEMB_TOOLCHAIN_PATH variable not set!)
else
$(error OS not supported!)
endif
$(info Overriding GNUARMEMB_TOOLCHAIN_PATH variable, with value: '$(GNUARMEMB_TOOLCHAIN_PATH)')
endif

ifndef CI
ifndef FLASHER_RUNNER
FLASHER_RUNNER = nrfjprog
endif
ifeq ($(FLASHER_RUNNER),nrfjprog)
ifeq (, $(shell which nrfjprog))
$(warning "Missing dependency: nrfjprog")
endif
endif
endif

ifndef TARGET_BOARD
TARGET_BOARD = axon_v1_3_0_nrf52840
$(warning TARGET_BOARD not defined, defaulting to: "$(TARGET_BOARD)")
endif

define check_dep
	@which $(1) > /dev/null || (echo 'Missing dependency: $(1)'; exit 1)
endef

define build_env
	@( \
		set -e; \
		. $(VENV_PATH)/bin/activate; \
		export ZEPHYR_TOOLCHAIN_VARIANT=gnuarmemb; \
		export GNUARMEMB_TOOLCHAIN_PATH="$(GNUARMEMB_TOOLCHAIN_PATH)"; \
		test -f $(NCS_SRC_PATH)/zephyr/zephyr-env.sh && \
			. $(NCS_SRC_PATH)/zephyr/zephyr-env.sh; \
		rm -f $(VENV_PATH)/gnuarmemb; \
		ln -s $(abspath $(GNUARMEMB_TOOLCHAIN_PATH)) \
					$(VENV_PATH)/gnuarmemb; \
		export PYTHONPATH="$$PYTHONPATH:$$(python -c 'import site; print(site.getsitepackages()[0])')"; \
		$(1) \
	)
endef

define UPDATE_NRF_CMDS
	( \
		cd $(NCS_SRC_PATH)/nrf && \
		git fetch --all && \
		git checkout $(NRF_CONNECT_SDK_TAG) && \
		west update && \
		if [ ! -n "$IN_NIX_SHELL" ]; then \
			west zephyr-export; \
		fi && \
		pip3 install -r $(NCS_SRC_PATH)/zephyr/scripts/requirements.txt && \
		pip3 install -r $(NCS_SRC_PATH)/nrf/scripts/requirements.txt && \
		pip3 install -r $(NCS_SRC_PATH)/bootloader/mcuboot/scripts/requirements.txt \
	)
endef

.PHONY: all
all: build

.PHONY: check_deps
check_deps:
	$(call check_dep, cmake)
	$(call check_dep, ninja)
	$(call check_dep, gperf)
	$(call check_dep, ccache)
	$(call check_dep, dfu-util)
	$(call check_dep, dtc)
	$(call check_dep, python3)
	@which $(GNUARMEMB_TOOLCHAIN_PATH)/bin/arm-none-eabi-gcc > /dev/null || \
		(echo 'Missing 'arm-none-eabi-gcc-$(GNUARMEMB_TOOLCHAIN_VERSION)' toolchain, not installed at '$(GNUARMEMB_TOOLCHAIN_PATH)' !'; exit 1)

$(VENV_PATH)/bin/activate:
	@set -e && \
	python3 -m venv $(VENV_PATH) && \
	. $(VENV_PATH)/bin/activate && \
	python3 -m pip install --upgrade pip && \
	pip install wheel

.PHONY: bootstrap_python_venv
bootstrap_python_venv: $(VENV_PATH)/bin/activate

$(NCS_SRC_PATH):
	$(call build_env,\
		pip3 install west; \
		mkdir $(NCS_SRC_PATH); \
		cd $(NCS_SRC_PATH); \
		west init -m $(NRF_CONNECT_SDK_REPO); \
		west update; \
		$(UPDATE_NRF_CMDS); \
	)

.PHONY: bootstrap_build_env
bootstrap_build_env: check_deps bootstrap_python_venv $(NCS_SRC_PATH)

.PHONY: build
build: bootstrap_build_env check_deps
	$(call build_env,\
		west build -b $(TARGET_BOARD) -- \
			-DOVERLAY_CONFIG=$(COMMON_ROOT_PATH)/overlays/overlay-hyper-ot.conf \
	)

.PHONY: menuconfig
menuconfig: check_deps
	$(call build_env,\
		west build -t menuconfig -b $(TARGET_BOARD) -- \
			-DOVERLAY_CONFIG=$(COMMON_ROOT_PATH)/overlays/overlay-hyper-ot.conf \
	)

.PHONY: flash
flash:
	$(call build_env,\
		west --verbose flash --runner $(FLASHER_RUNNER) --erase \
	)

.PHONY: flash-noerase
flash-noerase:
	$(call build_env,\
		west flash \
	)

.PHONY: recover
recover:
	$(call build_env,\
		nrfjprog --recover \
	)

.PHONY: debug
debug:
	$(call build_env,\
		west debug -r jlink --device nRF52840_xxAA \
	)

.PHONY: debugserver
debugserver:
	$(call build_env,\
		west debugserver -r jlink --device nRF52840_xxAA \
	)

.PHONY: clean
clean:
	rm -Rf build

.PHONY: distclean
distclean: clean
	rm -Rf $(VENV_PATH)
	rm -Rf $(NCS_SRC_PATH)
