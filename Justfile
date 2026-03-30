idf := require('idf.py')

srcs := shell('find . -name "$1" -not \( -path "*/$2/*" \) -print0 | xargs -0', '*.c', 'build')
include := shell('find . -name "$1" -not \( -path "*/$2/*" \) -print0 | xargs -0', '*.h', 'build')

# Show available commands
default:
    @just -f {{justfile()}} --list

# Build the project
[arg('no-ccache', long, value='true')]
build no-ccache="false":
    @{{idf}} {{ if no-ccache == "true" { "--no-ccache" } else { "--ccache" } }} build

# Flash the project
[arg('port', long)]
flash port="/dev/ttyUSB0": build
    @{{idf}} flash -p {{port}}

# Display the serial output
[arg('port', long)]
monitor port="/dev/ttyUSB0":
    @{{idf}} monitor -p {{port}}

# Show the memory usage
usage:
    @{{idf}} size

# Open the project configuration
config:
    @{{idf}} menuconfig

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
