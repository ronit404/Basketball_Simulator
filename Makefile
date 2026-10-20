# ==============================================================================
# 3D Basketball Shooting Simulator - Pure C Version Makefile
# Course: Computer Graphics (OpenGL / FreeGLUT)
# ==============================================================================

CC = gcc
CFLAGS = -O2 -Wall -Wextra -std=c11
TARGET = BasketballSimulator.exe

# Source & Object Files
SRCS = main.c game.c ball.c player.c renderer.c
OBJS = $(SRCS:.c=.o)

# OS Detection & Library Linking
ifeq ($(OS),Windows_NT)
    LIBS = -lfreeglut -lopengl32 -lglu32 -lm
    RM = del /Q /F
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        LIBS = -framework OpenGL -framework GLUT -lm
    else
        LIBS = -lglut -lGL -lGLU -lm
    endif
    RM = rm -f
endif

# Default Target
all: $(TARGET)

# Link
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS) $(LIBS)
	@echo [BUILD SUCCESS] Executable created: $(TARGET)

# Compile Object Files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Run
run: $(TARGET)
	./$(TARGET)

# Clean
clean:
	$(RM) $(OBJS) $(TARGET)
	@echo [CLEAN] Removed object files and executable.

.PHONY: all run clean
