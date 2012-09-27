PROJ = agent 
BUILD_DIR = build
SRCS = main.cpp \
	MessageQueue.cpp
OBJS = $(SRCS:.cpp=.o)
BUILD_OBJS = $(addprefix $(BUILD_DIR)/,$(OBJS))

CC = g++
CFLAGS = -Wall -Werror -O3
LD = g++
LDFLAGS = 

$(PROJ) : createBuildDir $(BUILD_OBJS)
	@echo "Linking application $(PROJ)"
	$(LD) -o $(PROJ) $(BUILD_OBJS) $(LDFLAGS)

createBuildDir :
	mkdir -p $(BUILD_DIR)

clean : clean_dist clean_build  

clean_dist :
	rm -f $(PROJ)

clean_build :
	rm -rf $(BUILD_DIR)

$(BUILD_DIR)/%.o : %.cpp
	$(CC) $(CFLAGS) -c $< -o $@


