# This file was automatically generated for projects
# without default 'CMakeLists.txt' file.

FILE(GLOB_RECURSE app_sources E:/Documents/PlatformIO/Projects/mercury-1000/src/*.*)

idf_component_register(SRCS ${app_sources})

target_add_binary_data(${COMPONENT_TARGET} "certs/ca_cert.pem" TEXT)