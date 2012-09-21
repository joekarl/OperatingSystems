PROJ = agent
BUILD_DIR = build
OBJS = agent.o 
BUILD_OBJS = $(addprefix $(BUILD_DIR)/,$(OBJS))
SRCS = $(OBJS,.o=.c)
CC = gcc
CFLAGS = -Wall -Werror
LD = gcc
LDFLAGS = 

$(PROJ) : $(BUILD_OBJS)
	@echo "Linking $(PROJ)"
	$(LD) -o $(PROJ) $(BUILD_OBJS) $(LDFLAGS)

clean : 
	rm -rf $(BUILD_DIR)
	rm -f $(PROJ)

$(BUILD_DIR)/%.o : %.c
	mkdir -p $(BUILD_DIR)
	@echo "Building $@"
	$(CC) $(CFLAGS) -c $< -o $@
