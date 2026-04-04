idf := require('idf.py')
check := require('cppcheck')

srcs := shell('find . -name "$1" -not \( -path "*/$2/*" \) -print0 | xargs -0', '*.c', 'build')
include := shell('find . -name "$1" -not \( -path "*/$2/*" \) -print0 | xargs -0', '*.h', 'build')

ccache := 'true'
_ccache-flag := if ccache == 'true' { '--ccache' } else { '--no-ccache' }

# Show available commands
default:
    @just -f {{justfile()}} --list

# Build the project
build:
    @{{idf}} {{_ccache-flag}} build

# Flash the project
[arg('port', long)]
flash port="/dev/ttyUSB0":
    @{{idf}} {{_ccache-flag}} flash -p {{port}}

# Display the serial output
[arg('port', long)]
monitor port="/dev/ttyUSB0":
    @{{idf}} monitor -p {{port}}

# Show the memory usage
usage:
    @{{idf}} {{_ccache-flag}} size

# Open the project configuration
config:
    @{{idf}} {{_ccache-flag}} menuconfig

# Run the static analyzer
check:
    @{{check}} --project=build/compile_commands.json \
    --enable=warning,performance,portability \
    --check-level=exhaustive \
    --inline-suppr \
    --suppress=missingIncludeSystem \
    --error-exitcode=1 \
    -i $IDF_PATH

# Format source and header files
format:
    clang-format -i {{srcs}}
    clang-format -i {{include}}

# Check for formatting in source and header files
format_check:
    clang-format --dry-run -Werror {{srcs}}
    clang-format --dry-run -Werror {{include}}

# Remove the build artifacts
clean:
    @{{idf}} clean

# Delete the build directory
fullclean:
    @{{idf}} fullclean
