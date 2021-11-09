MAKEFILE_ROOT_PATH := $(realpath $(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
SRC := $(MAKEFILE_ROOT_PATH)/mcumgr-cli/mcumgr

.PHONY: all
all: build

ifdef CROSS
MCUMGR_OUT_DIR = $(CROSS)
ifeq ($(CROSS),linux/mips)
GO_CROSS_FLAGS = GOOS=linux GOARCH=mips GOMIPS=softfloat
else ifeq ($(CROSS),linux/arm64)
GO_CROSS_FLAGS = GOOS=linux GOARCH=arm64
else ifeq ($(CROSS),linux/amd64)
GO_CROSS_FLAGS = GOOS=linux GOARCH=amd64
else
$(error No support for specified CROSS platform: '$(CROSS)')
endif
else
MCUMGR_OUT_DIR = native
endif
BUILD_DIR = $(MAKEFILE_ROOT_PATH)/build/$(MCUMGR_OUT_DIR)
BUILD_GOPATH = $(MAKEFILE_ROOT_PATH)/build/go


define check_dep
	@which $(1) > /dev/null || (echo 'Missing dependency: $(1)'; exit 1)
endef

.PHONY: check_deps
check_deps:
	$(call check_dep, go)


.PHONY: build
build: check_deps
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(BUILD_GOPATH)
	@cd $(SRC) && \
		export GOPATH=$(BUILD_GOPATH) && \
		export PATH="$(BUILD_GOPATH)/bin:$${PATH}" && \
		$(GO_CROSS_FLAGS) go get -v && \
		$(GO_CROSS_FLAGS) go build -v -x \
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
