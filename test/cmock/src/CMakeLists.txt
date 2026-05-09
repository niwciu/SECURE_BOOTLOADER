cmake_minimum_required(VERSION 3.20)

add_library(cmock STATIC
	cmock.c
) 

# set_target_properties(menu_lib PROPERTIES OUTPUT_NAME "lib_lcd_hd44780")
set_target_properties(cmock PROPERTIES 
  OUTPUT_NAME "cmock"
  PREFIX ""
  )
target_compile_options(cmock PRIVATE "-Wall")
target_include_directories(cmock PUBLIC .)
target_link_libraries(cmock PUBLIC unity)