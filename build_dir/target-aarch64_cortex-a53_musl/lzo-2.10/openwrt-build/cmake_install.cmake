# Install script for directory: /home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/usr")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "TRUE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/staging_dir/toolchain-aarch64_cortex-a53_gcc-8.4.0_musl/bin/aarch64-openwrt-linux-musl-objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/share/doc/lzo/AUTHORS;/usr/share/doc/lzo/COPYING;/usr/share/doc/lzo/NEWS;/usr/share/doc/lzo/THANKS;/usr/share/doc/lzo/LZO.FAQ;/usr/share/doc/lzo/LZO.TXT;/usr/share/doc/lzo/LZOAPI.TXT")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/share/doc/lzo" TYPE FILE FILES
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/AUTHORS"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/COPYING"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/NEWS"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/THANKS"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/doc/LZO.FAQ"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/doc/LZO.TXT"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/doc/LZOAPI.TXT"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/include/lzo/lzo1.h;/usr/include/lzo/lzo1a.h;/usr/include/lzo/lzo1b.h;/usr/include/lzo/lzo1c.h;/usr/include/lzo/lzo1f.h;/usr/include/lzo/lzo1x.h;/usr/include/lzo/lzo1y.h;/usr/include/lzo/lzo1z.h;/usr/include/lzo/lzo2a.h;/usr/include/lzo/lzo_asm.h;/usr/include/lzo/lzoconf.h;/usr/include/lzo/lzodefs.h;/usr/include/lzo/lzoutil.h")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/include/lzo" TYPE FILE FILES
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/include/lzo/lzo1.h"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/include/lzo/lzo1a.h"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/include/lzo/lzo1b.h"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/include/lzo/lzo1c.h"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/include/lzo/lzo1f.h"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/include/lzo/lzo1x.h"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/include/lzo/lzo1y.h"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/include/lzo/lzo1z.h"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/include/lzo/lzo2a.h"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/include/lzo/lzo_asm.h"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/include/lzo/lzoconf.h"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/include/lzo/lzodefs.h"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/include/lzo/lzoutil.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/lib/liblzo2.a")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/lib" TYPE STATIC_LIBRARY FILES "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/openwrt-build/liblzo2.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/lib/liblzo2.so.2.0.0;/usr/lib/liblzo2.so.2")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/lib" TYPE SHARED_LIBRARY FILES
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/openwrt-build/liblzo2.so.2.0.0"
    "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/openwrt-build/liblzo2.so.2"
    )
  foreach(file
      "$ENV{DESTDIR}/usr/lib/liblzo2.so.2.0.0"
      "$ENV{DESTDIR}/usr/lib/liblzo2.so.2"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/openwrt-build/:" "${file}")
      endif()
    endif()
  endforeach()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/lib/liblzo2.so")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/lib" TYPE SHARED_LIBRARY FILES "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/openwrt-build/liblzo2.so")
  if(EXISTS "$ENV{DESTDIR}/usr/lib/liblzo2.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/lib/liblzo2.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/openwrt-build/:" "$ENV{DESTDIR}/usr/lib/liblzo2.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/libexec/lzo/examples/lzopack")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/libexec/lzo/examples" TYPE EXECUTABLE FILES "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/openwrt-build/lzopack")
  if(EXISTS "$ENV{DESTDIR}/usr/libexec/lzo/examples/lzopack" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/libexec/lzo/examples/lzopack")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/openwrt-build/:" "$ENV{DESTDIR}/usr/libexec/lzo/examples/lzopack")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/libexec/lzo/examples/lzotest")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/libexec/lzo/examples" TYPE EXECUTABLE FILES "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/openwrt-build/lzotest")
  if(EXISTS "$ENV{DESTDIR}/usr/libexec/lzo/examples/lzotest" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/libexec/lzo/examples/lzotest")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/openwrt-build/:" "$ENV{DESTDIR}/usr/libexec/lzo/examples/lzotest")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/libexec/lzo/examples/simple")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/libexec/lzo/examples" TYPE EXECUTABLE FILES "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/openwrt-build/simple")
  if(EXISTS "$ENV{DESTDIR}/usr/libexec/lzo/examples/simple" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/libexec/lzo/examples/simple")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/openwrt-build/:" "$ENV{DESTDIR}/usr/libexec/lzo/examples/simple")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/libexec/lzo/examples/testmini")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/libexec/lzo/examples" TYPE EXECUTABLE FILES "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/openwrt-build/testmini")
  if(EXISTS "$ENV{DESTDIR}/usr/libexec/lzo/examples/testmini" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}/usr/libexec/lzo/examples/testmini")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/openwrt-build/:" "$ENV{DESTDIR}/usr/libexec/lzo/examples/testmini")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
   "/usr/lib/pkgconfig/lzo2.pc")
  if(CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
  if(CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
    message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
  endif()
file(INSTALL DESTINATION "/usr/lib/pkgconfig" TYPE FILE FILES "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/openwrt-build/lzo2.pc")
endif()

if(CMAKE_INSTALL_COMPONENT)
  set(CMAKE_INSTALL_MANIFEST "install_manifest_${CMAKE_INSTALL_COMPONENT}.txt")
else()
  set(CMAKE_INSTALL_MANIFEST "install_manifest.txt")
endif()

string(REPLACE ";" "\n" CMAKE_INSTALL_MANIFEST_CONTENT
       "${CMAKE_INSTALL_MANIFEST_FILES}")
file(WRITE "/home/voyager717/Downloads/SDK_Sourcecode/build_openwrt_new/AX3000-MT7981-Z8102AX/build_dir/target-aarch64_cortex-a53_musl/lzo-2.10/openwrt-build/${CMAKE_INSTALL_MANIFEST}"
     "${CMAKE_INSTALL_MANIFEST_CONTENT}")
