

APP = mailbox 
SRCS = main.c 
OBJS = $(SRCS:.c=.o)
BUILD_OBJS = $(addprefix $(BUILD_DIR)/,$(OBJS))
BUILD_DIR = build

CC = gcc
CFLAGS = -Wall -Werror -O3
LD = gcc
LDFLAGS = -lpthread 

$(APP) : createBuildDir $(BUILD_OBJS)
	@echo "Linking application $(APP)"
	$(LD) -o $(APP) $(BUILD_OBJS) $(LDFLAGS)

clean : clean_build clean_$(APP)

clean_build :
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

createBuildDir :
	mkdir -p $(BUILD_DIR)

clean_$(APP) : clean_build  
	rm -f $(APP)

