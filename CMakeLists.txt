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
  set(PREFIX_FILE \"${CMAKE_CURRENT_SOURCE_DIR}/.script_prefix.lua\")
  set(SCRIPT_FILE \"${CMAKE_CURRENT_SOURCE_DIR}/src/script.lua\")
  set(DEST_FILE \"/etc/rds95.lua\")

  # Check if the optional prefix file exists
  if(EXISTS \${PREFIX_FILE})
    message(STATUS \"Prefix file found. Combining with script.lua.\")
    file(READ \${PREFIX_FILE} PREFIX_CONTENT)
    file(READ \${SCRIPT_FILE} SCRIPT_CONTENT)
    set(FINAL_CONTENT \"\${PREFIX_CONTENT}\n\${SCRIPT_CONTENT}\")
  else()
    message(STATUS \"Prefix file not found. Using script.lua directly.\")
    file(READ \${SCRIPT_FILE} FINAL_CONTENT)
  endif()

  # Write the resulting content to the destination file
  message(STATUS \"Installing script file to \${DEST_FILE}\")
  file(WRITE \${DEST_FILE} \"\${FINAL_CONTENT}\")
  
  # Change ownership to the user who invoked sudo (if applicable)
  if(DEFINED ENV{SUDO_USER})
    message(STATUS \"Changing ownership of \${DEST_FILE} to \$ENV{SUDO_USER}\")
    execute_process(COMMAND chown \$ENV{SUDO_USER}:\$ENV{SUDO_USER} \${DEST_FILE})
  else()
    message(STATUS \"No SUDO_USER detected, skipping chown\")
  endif()
"
)