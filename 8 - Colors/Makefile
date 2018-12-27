#OBJS specifies which files to compile as part of the project
OBJS = glad.c shader.cpp main1.cpp

#CC specifies which compiler we're using
CC = g++

#COMPILER_FLAGS specifies the additional compilation options we're using
# -w suppresses all warnings
COMPILER_FLAGS = -w -Iinclude

#LINKER_FLAGS specifies the libraries we're linking against
LINKER_FLAGS = -lGL -lSDL2 -lSDL2_image -ldl

#OBJ_NAME specifies the name of our exectuable
OBJ_NAME = gl

#This is the target that compiles our executable
all : $(OBJS)
	$(CC) $(OBJS) $(COMPILER_FLAGS) $(LINKER_FLAGS) -o $(OBJ_NAME)
