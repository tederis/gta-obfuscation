cmake_minimum_required(VERSION 2.8) 
project (RWDeobfuscator)
add_library(RWDeobfuscator SHARED dllmain.cpp)
set_target_properties(RWDeobfuscator PROPERTIES SUFFIX ".asi")