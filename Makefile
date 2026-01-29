TARGET = build/bin/app

CC = gcc
CFLAGS = -Wall -Wextra -I. -Isrc -Isys -Iext/mavlink/include -MMD -MP
LDFLAGS = -pthread

SRC_DIRS = src sys
OBJ_DIR = build/obj
BIN_DIR = build/bin
SERVICE = scripts/setup_service.sh

SRCS = $(shell find src sys -name '*.c')
OBJS = $(patsubst %.c,$(OBJ_DIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)

all: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# rule compile .c -> .o
$(OBJ_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run:
	./$(TARGET)

install-service: all
	sudo chmod +x ./$(SERVICE)
	sed -i 's/\r$$//' ./$(SERVICE)
	./$(SERVICE)

clean:
	rm -rf $(OBJ_DIR) $(BIN_DIR)

-include $(DEPS)

.PHONY: all clean run install-service
