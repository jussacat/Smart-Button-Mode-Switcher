# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/uzine/espidf/esp-idf/components/bootloader/subproject"
  "/home/uzine/espidf/Smart-Button-Mode-Switcher/Project/build/bootloader"
  "/home/uzine/espidf/Smart-Button-Mode-Switcher/Project/build/bootloader-prefix"
  "/home/uzine/espidf/Smart-Button-Mode-Switcher/Project/build/bootloader-prefix/tmp"
  "/home/uzine/espidf/Smart-Button-Mode-Switcher/Project/build/bootloader-prefix/src/bootloader-stamp"
  "/home/uzine/espidf/Smart-Button-Mode-Switcher/Project/build/bootloader-prefix/src"
  "/home/uzine/espidf/Smart-Button-Mode-Switcher/Project/build/bootloader-prefix/src/bootloader-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/uzine/espidf/Smart-Button-Mode-Switcher/Project/build/bootloader-prefix/src/bootloader-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/uzine/espidf/Smart-Button-Mode-Switcher/Project/build/bootloader-prefix/src/bootloader-stamp${cfgdir}") # cfgdir has leading slash
endif()
