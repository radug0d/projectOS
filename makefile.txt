CC = gcc
CFLAGS = -Wall -g
SOURCES = operations.c customs.c treasure_manager.c treasure.c treasure_hub.c monitor.c monitor_state.c
HEADERS = operations.h customs.h treasure.h
TARGET = treasure_manager

# In case filenames overlap with make commands
.PHONY: all clean test build rebuild

all: build

build: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	$(RM) $(TARGET) $(OBJECTS)
	$(RM) -r test_hunt*
	$(RM) logged_hunt--*
	$(RM) .monitor_cmd

rebuild: clean build

# Test scenario
test: build
	@echo "\n===== Running Test Scenario =====\n"
	
	@echo "1. Adding a treasure to test_hunt1..."
	@echo "test_id1\nuser1\n10.5\n20.3\nHidden under the big rock\n100" | ./$(TARGET) --add test_hunt1
	
	@echo "\n2. Adding another treasure to test_hunt1..."
	@echo "test_id2\nuser2\n15.7\n30.8\nBehind the waterfall\n200" | ./$(TARGET) --add test_hunt1
	
	@echo "\n3. Listing treasures in test_hunt1..."
	./$(TARGET) --list test_hunt1
	
	@echo "\n4. Viewing details of test_id1..."
	./$(TARGET) --view test_hunt1 test_id1
	
	@echo "\n5. Removing treasure test_id1..."
	./$(TARGET) --remove_treasure test_hunt1 test_id1
	
	@echo "\n6. Listing treasures after removal..."
	./$(TARGET) --list test_hunt1
	
	@echo "\n7. Creating a second hunt test_hunt2..."
	@echo "second_id\nuser3\n5.5\n8.3\nInside the hollow tree\n150" | ./$(TARGET) --add test_hunt2
	
	@echo "\n8. Removing entire hunt test_hunt2..."
	./$(TARGET) --remove_hunt test_hunt2
	
	@echo "\n===== Test Scenario Complete =====\n"