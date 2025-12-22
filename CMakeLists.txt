cmake_minimum_required(VERSION 3.10)

project(rds95 VERSION 1.6)

add_compile_options(-Wall -Werror -Wextra -pedantic -O2 -std=c18 -march=native -DVERSION=\"${PROJECT_VERSION}\")

file(GLOB INIH_FILES "inih/*.c")
add_library(inih OBJECT ${INIH_FILES})

file(GLOB SOURCES src/*.c)

add_executable(rds95 ${SOURCES})

find_package(Lua REQUIRED)

target_include_directories(rds95 PRIVATE ${LUA_INCLUDE_DIR})
target_link_libraries(rds95 PRIVATE m pthread pulse pulse-simple inih ${LUA_LIBRARIES})

install(TARGETS rds95 DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)

install(CODE
"
  # Define the paths for the source and destination files
  set(PREFIX_FILE \"${CMAKE_CURRENT_SOURCE_DIR}/.command_prefix.lua\")
  set(COMMAND_FILE \"${CMAKE_CURRENT_SOURCE_DIR}/src/command.lua\")
  # The $ENV{HOME} variable is evaluated at install time, not configure time
  set(DEST_FILE \"$ENV{HOME}/.rds95.command.lua\")

  # Check if the optional prefix file exists
  if(EXISTS \${PREFIX_FILE})
    # If it exists, read the prefix and the main command file
    message(STATUS \"Prefix file found. Combining with command.lua.\")
    file(READ \${PREFIX_FILE} PREFIX_CONTENT)
    file(READ \${COMMAND_FILE} COMMAND_CONTENT)
    # Concatenate them, with the prefix content first
    set(FINAL_CONTENT \"\${PREFIX_CONTENT}\n\${COMMAND_CONTENT}\")
  else()
    # Otherwise, just use the content of the main command file
    message(STATUS \"Prefix file not found. Using command.lua directly.\")
    file(READ \${COMMAND_FILE} FINAL_CONTENT)
  endif()

  # Write the resulting content to the destination file
  message(STATUS \"Installing command file to \${DEST_FILE}\")
  file(WRITE \${DEST_FILE} \"\${FINAL_CONTENT}\")
"
)