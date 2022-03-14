# The Directories, Source, Includes, Objects, Binary 
ROOT_DIR       := ../../../
SRC_DIR        := .
TEST_DIR   	   := $(SRC_DIR)
DATA_DIR   	   := $(TEST_DIR)/test_data
BUILD_DIR  	   := $(TEST_DIR)/obj
BIN_DIR        := $(TEST_DIR)/bin

# Flags, Libraries and Includes
GOOGLE_TEST_INSTALL = $(ROOT_DIR)/3rd-party/gtest
GOOGLE_TEST_LIB = gtest
GOOGLE_MOCK_LIB = gmock
GOOGLE_TEST_INCLUDE = $(GOOGLE_TEST_INSTALL)/usr/include
G++ = g++
G++_FLAGS = -c -Wall -I $(GOOGLE_TEST_INCLUDE) -std=c++17
LD_FLAGS = -L $(GOOGLE_TEST_INSTALL)/usr/lib -l $(GOOGLE_TEST_LIB) -l $(GOOGLE_MOCK_LIB) -l pthread 
RM = rm -rf

# The Target Binary Program
TARGET      := $(BIN_DIR)/test-causalize
RUN_TESTS   := test-causalize

# Source files.
MAIN_SRC = $(SRC_DIR)/main.cpp

TEST_SRC = $(TEST_DIR)/models_test.cpp

# Objects
TEST_OBJ=$(addprefix $(BUILD_DIR)/, $(TEST_SRC:.cpp=.o))

MAIN_OBJ=$(addprefix $(BUILD_DIR)/, $(MAIN_SRC:.cpp=.o))

$(BUILD_DIR)/%.o : %.cpp
	$(G++) $(INCLUDE) $(CXXFLAGS) -MM -MT $@ -MF $(patsubst %.o,%.d,$@) $<
	$(G++) $(G++_FLAGS) $(INCLUDE) $(LIB) $< -o $@

all: $(TARGET) $(RUN_TESTS)

$(TARGET): $(MAIN_OBJ) $(TEST_OBJ) $(BUILD_DIR)
		g++ -o $(TARGET) $(MAIN_OBJ) $(TEST_OBJ) $(LD_FLAGS) $(LIB)

$(RUN_TESTS): $(TARGET)
		@echo Clean up test data dir.
		@rm -rf $(DATA_DIR)/*
		@echo Done
		@echo Running tests.
		$(TARGET)
		@echo Done.

$(TEST_OBJ): | $(BUILD_DIR)

$(MAIN_OBJ): | $(BUILD_DIR)

$(BUILD_DIR):
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(BUILD_DIR)
	@mkdir -p $(DATA_DIR)

clean:
	$(RM) $(BIN_DIR) $(BUILD_DIR) $(DATA_DIR) 
                    
.PHONY: all clean