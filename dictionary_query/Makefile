CC = gcc

SRCS = test_dictionary.c  ../logs/logger.c

INC_DIRS = -I../utils -I../logs

TARGET = test_dictionary_query

$(TARGET): $(SRCS)
	$(CC) $(INC_DIRS) -o $@ $^ 

clean:
	rm -f $(TARGET)