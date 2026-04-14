# EmbedScript.cmake — cross-platform replacement for 'xxd -i'
#
# Reads a file and emits a C char array header that can be compiled
# directly into a WASM module without any filesystem access at runtime.
#
# Usage (from a CMake add_custom_command):
#   cmake -DSCRIPT_FILE=<path> -DOUTPUT_FILE=<path> -P EmbedScript.cmake

cmake_minimum_required(VERSION 3.20)

if(NOT DEFINED SCRIPT_FILE OR NOT DEFINED OUTPUT_FILE)
    message(FATAL_ERROR "EmbedScript.cmake: SCRIPT_FILE and OUTPUT_FILE must be defined")
endif()

# Read as hex bytes
file(READ "${SCRIPT_FILE}" CONTENT HEX)
string(LENGTH "${CONTENT}" HEX_LEN)
math(EXPR BYTE_COUNT "${HEX_LEN} / 2")

# Insert "0x" prefix and "," suffix for every 2-char hex pair
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," BYTES "${CONTENT}")

get_filename_component(SCRIPT_NAME "${SCRIPT_FILE}" NAME)

file(WRITE "${OUTPUT_FILE}"
"/* Auto-generated from ${SCRIPT_NAME} — do not edit */\n"
"static const char EMBEDDED_SCRIPT[] = {\n"
"${BYTES}\n"
"0x00 /* null terminator */\n"
"};\n"
"static const unsigned int EMBEDDED_SCRIPT_SIZE = ${BYTE_COUNT};\n"
)
