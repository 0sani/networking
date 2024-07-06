# Variables
CC = gcc
CFLAGS = -Wall -Wextra -Werror -pedantic -std=c11 -g
OBJ_SERVER = server.o utils.o
OBJ_CLIENT = client.o utils.o
TARGET_SERVER = server
TARGET_CLIENT = client

# Default target
all: $(TARGET_SERVER) $(TARGET_CLIENT)

# Link object files to create the final server executable
$(TARGET_SERVER): $(OBJ_SERVER)
	$(CC) $(CFLAGS) -o $(TARGET_SERVER) $(OBJ_SERVER)

# Link object files to create the final client executable
$(TARGET_CLIENT): $(OBJ_CLIENT)
	$(CC) $(CFLAGS) -o $(TARGET_CLIENT) $(OBJ_CLIENT)

# Compile .c files to .o files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up build files
clean:
	rm -f $(OBJ_SERVER) $(OBJ_CLIENT) $(TARGET_SERVER) $(TARGET_CLIENT)

# Phony targets (these are not files)
.PHONY: all clean
