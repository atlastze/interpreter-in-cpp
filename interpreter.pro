############################################################
# Project file
# Makefile will include this project file
############################################################

# Specify target name
TARGET = interpreter

# Specify the #include directories which should be searched when compiling the project.
INCLUDEPATH = ./src

# Specify the source directories which should be searched when compiling the project.
DEPENDPATH = ./src

# Defines the header files for the project.
HEADERS = $(wildcard ./src/*.h)

# Defines the source files for the project.
SOURCES = $(wildcard ./src/*.cpp)
