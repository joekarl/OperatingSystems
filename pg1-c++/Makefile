PROJ = test
BUILD_DIR = build
OBJS = test.o \
	testClass.o 
BUILD_OBJS = $(addprefix $(BUILD_DIR)/,$(OBJS))
SRCS = $(OBJS,.o=.cpp)
CC = g++
CFLAGS = -Wall -Werror
LD = g++
LDFLAGS = 

$(PROJ) : $(BUILD_OBJS)
	@echo "Linking $(PROJ)"
	$(LD) -o $(PROJ) $(BUILD_OBJS) $(LDFLAGS)

clean : 
	rm -rf $(BUILD_DIR)
	rm -f $(PROJ)

$(BUILD_DIR)/%.o : %.cpp
	mkdir -p $(BUILD_DIR)
	@echo "Building $@"
	$(CC) $(CFLAGS) -c $< -o $@
