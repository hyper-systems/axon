MAKEFILE_ROOT_PATH := $(realpath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))

.PHONY: all
all: build

ifdef CROSS
MCUMGR_OUT_DIR = $(CROSS)
ifeq ($(CROSS),linux/mips)
export GOOS=linux
export GOARCH=mips
export GOMIPS=softfloat
else ifeq ($(CROSS),linux/arm64)
export GOOS=linux
export GOARCH=arm64
else ifeq ($(CROSS),linux/amd64)
export GOOS=linux
export GOARCH=amd64
else
$(error No support for specified CROSS platform: '$(CROSS)')
endif
else
MCUMGR_OUT_DIR = native
endif

BUILD_DIR = $(MAKEFILE_ROOT_PATH)/build/$(MCUMGR_OUT_DIR)
BUILD_GOPATH = $(MAKEFILE_ROOT_PATH)/build/go
SRC_DIR = $(MAKEFILE_ROOT_PATH)/build/src


define check_dep
	@which $(1) > /dev/null || (echo 'Missing dependency: $(1)'; exit 1)
endef

.PHONY: check_deps
check_deps:
	$(call check_dep, go)

.PHONY: build
build: check_deps
	@mkdir -p $(BUILD_DIR) $(BUILD_GOPATH) $(SRC_DIR) && \
	export GOPATH=$(BUILD_GOPATH) && \
	export PATH="$(BUILD_GOPATH)/bin:$${PATH}" && \
	cd $(SRC_DIR) && \
	if [ ! -f mynewt-mcumgr-cli/.git/config ]; then \
		git clone https://github.com/apache/mynewt-mcumgr-cli; \
	else \
		git pull; \
	fi && \
	cd mynewt-mcumgr-cli/mcumgr && \
	go get -v && \
	go build -v -x \
		-trimpath \
		-ldflags="-s -w" \
		-o $(BUILD_DIR)/mcumgr && \
	if $$(which upx > /dev/null); then \
		upx -9 $(BUILD_DIR)/mcumgr -o $(BUILD_DIR)/mcumgr.upx; \
	fi

.PHONY: clean
clean:
	rm -Rf $(BUILD_DIR)

.PHONY: distclean
distclean:
	@chmod -R 777 $(MAKEFILE_ROOT_PATH)/build > /dev/null 2>&1 || true
	rm -Rf $(MAKEFILE_ROOT_PATH)/build
