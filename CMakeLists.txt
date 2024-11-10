cmake_minimum_required(VERSION 3.16)

idf_component_register(
    SRCS "src/buffer.cpp"
         "src/writer.cpp"
	 "src/misc.c"
    INCLUDE_DIRS include
)
