
CC = gcc
CFLAGS = -Wall -Wextra -g -Iinclude -DSDL_COMPAT

LDFLAGS = -lSDL -lSDL_image -lSDL_ttf -lm
SRC_DIR = fatma
OBJ_DIR = ob
BIN = proj


SRCS = $(wildcard $(SRC_DIR)/*.c)


OBJS = $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRCS))


all: $(BIN)


$(BIN): $(OBJS)
	@mkdir -p $(dir $@)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Compilation terminée : $@"


$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@


clean:
	rm -rf $(OBJ_DIR)/*
	rm -rf $(BIN)
	@echo "Nettoyage terminé."


run: $(BIN)
	./$(BIN)


.PHONY: all clean run check_assets
