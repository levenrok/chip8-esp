CCACHE := yes
PORT := /dev/ttyUSB0

idf != command -v idf.py
check != command -v cppcheck
format != command -v clang-format

srcs != find -type f \( -name "*.c" -o -name "*.cpp" \) -not \( -path "*/build/*" \) -print0 | xargs -0
includes != find -type f \( -name "*.h" -o -name "*.hpp" \) -not \( -path "*/build/*" \) -print0 | xargs -0

ifeq ($(CCACHE), yes)
	ccache-flag := --ccache
else
	ccache-flag := --no-ccache
endif

.PHONY: all build flash monitor run usage config

all: build

build: ## Build the project
	@$(idf) $(ccache-flag) build

flash: ## Flash the project
	@$(idf) $(ccache-flag) flash -p $(PORT)

monitor: ## Display the serial output
	@$(idf) $(ccache-flag) monitor -p $(PORT)

run: flash monitor ## Flash and monitor the application

usage: ## Show the memory usage
	@$(idf) $(ccache-flag) size

config: ## Open the project configuration
	@$(idf) $(ccache-flag) menuconfig

.PHONY: check format format-check

check: ## Run the static analyzer
	@$(check) --project=build/compile_commands.json \
	--enable=warning,performance,portability \
	--check-level=exhaustive \
	--inline-suppr \
	--suppressions-list=.suppressions \
	--error-exitcode=1 \
	-i $(IDF_PATH)

format: ## Format source and header files
	$(format) -i $(srcs)
	$(format) -i $(includes)

format-check: ## Check for formatting in source and header files
	$(format) --dry-run -Werror $(srcs)
	$(format) --dry-run -Werror $(includes)

.PHONY: clean fullclean help

clean: ## Remove the build artifacts
	@$(idf) clean

fullclean: ## Delete the build directory
	@$(idf) fullclean

help: ## Print the help message
	@echo "Available recipes:"
	@awk 'BEGIN {FS = ":.*?## "} /^[a-zA-Z_-]+:.*?## / {printf "    \033[36m%-15s\033[0m %s\n", $$1, $$2}' $(MAKEFILE_LIST) | sort
