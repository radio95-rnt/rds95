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
  set(SCRIPTS_DIR \"${CMAKE_CURRENT_SOURCE_DIR}/scripts\")
  set(DEST_FILE \"/etc/rds95.lua\")

  # Initialize content variable
  set(FINAL_CONTENT \"\")

  # Check if the optional prefix file exists
  if(EXISTS \${PREFIX_FILE})
    message(STATUS \"Prefix file found. Adding .script_prefix.lua.\")
    file(READ \${PREFIX_FILE} PREFIX_CONTENT)
    set(FINAL_CONTENT \"\${PREFIX_CONTENT}\")
  else()
    message(STATUS \"Prefix file not found.\")
  endif()

  # Check if scripts directory exists and contains .lua files
  if(EXISTS \${SCRIPTS_DIR} AND IS_DIRECTORY \${SCRIPTS_DIR})
    file(GLOB LUA_SCRIPTS \"\${SCRIPTS_DIR}/*.lua\")
    list(LENGTH LUA_SCRIPTS SCRIPT_COUNT)
    
    if(SCRIPT_COUNT GREATER 0)
      message(STATUS \"Found \${SCRIPT_COUNT} Lua script(s) in scripts directory.\")
      list(SORT LUA_SCRIPTS)
      
      foreach(LUA_SCRIPT \${LUA_SCRIPTS})
        get_filename_component(SCRIPT_NAME \${LUA_SCRIPT} NAME)
        message(STATUS \"Adding script: \${SCRIPT_NAME}\")
        file(READ \${LUA_SCRIPT} SCRIPT_CONTENT)
        set(FINAL_CONTENT \"\${FINAL_CONTENT}\n\${SCRIPT_CONTENT}\")
      endforeach()
    else()
      message(STATUS \"No Lua scripts found in scripts directory.\")
    endif()
  else()
    message(STATUS \"Scripts directory not found.\")
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