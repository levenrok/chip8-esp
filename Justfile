idf := require('idf.py')
check := require('cppcheck')

srcs := shell('find . -type f \( -name "$1" -o -name "$2" \) -not \( -path "*/$3/*" \) -print0 | xargs -0', '*.c', '*.cpp', 'build')
include := shell('find . -type f \( -name "$1" -o -name "$2" \) -not \( -path "*/$3/*" \) -print0 | xargs -0', '*.h', '*.hpp', 'build')

ccache := 'true'
port := '/dev/ttyUSB0'

[private]
ccache-flag := if ccache == 'true' { '--ccache' } else { '--no-ccache' }

# Show available commands
default:
    @just -f {{ justfile() }} --list

# Build the project
build:
    @{{ idf }} {{ ccache-flag }} build

# Flash the project
flash:
    @{{ idf }} {{ ccache-flag }} flash -p {{ port }}

# Display the serial output
monitor:
    @{{ idf }} monitor -p {{ port }}

# Flash and monitor the application
run: flash monitor

# Show the memory usage
usage:
    @{{ idf }} {{ ccache-flag }} size

# Open the project configuration
config:
    @{{ idf }} {{ ccache-flag }} menuconfig

# Run the static analyzer
check:
    @{{ check }} --project=build/compile_commands.json \
    --enable=warning,performance,portability \
    --check-level=exhaustive \
    --inline-suppr \
    --suppressions-list=.suppressions \
    --error-exitcode=1 \
    -i $IDF_PATH

# Format source and header files
format:
    clang-format -i {{ srcs }}
    clang-format -i {{ include }}

# Check for formatting in source and header files
format-check:
    clang-format --dry-run -Werror {{ srcs }}
    clang-format --dry-run -Werror {{ include }}

# Remove the build artifacts
clean:
    @{{ idf }} clean

# Delete the build directory
fullclean:
    @{{ idf }} fullclean
