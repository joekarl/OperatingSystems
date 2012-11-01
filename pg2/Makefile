
BUILD_DIR = build

SERVER = semaphore_server 
SERVER_SRCS = server.c 
SERVER_OBJS = $(SERVER_SRCS:.c=.o)
SERVER_BUILD_OBJS = $(addprefix $(BUILD_DIR)/,$(SERVER_OBJS))

CLIENT = semaphore_client 
CLIENT_SRCS = client.c 
CLIENT_OBJS = $(CLIENT_SRCS:.c=.o)
CLIENT_BUILD_OBJS = $(addprefix $(BUILD_DIR)/,$(CLIENT_OBJS))


CC = gcc
CFLAGS = -Wall -Werror -O3
LD = gcc
LDFLAGS = 

all : $(SERVER) $(CLIENT)

clean : clean_build clean_$(SERVER) clean_$(CLIENT)

clean_build :
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/%.o : %.c
	$(CC) $(CFLAGS) -c $< -o $@

createBuildDir :
	mkdir -p $(BUILD_DIR)

$(SERVER) : createBuildDir $(SERVER_BUILD_OBJS)
	@echo "Linking application $(SERVER)"
	$(LD) -o $(SERVER) $(SERVER_BUILD_OBJS) $(LDFLAGS)

clean_$(SERVER) : clean_build  
	rm -f $(SERVER)

$(CLIENT) : createBuildDir $(CLIENT_BUILD_OBJS)
	@echo "Linking application $(CLIENT)"
	$(LD) -o $(CLIENT) $(CLIENT_BUILD_OBJS) $(LDFLAGS)

clean_$(CLIENT) :
	rm -f $(CLIENT)

