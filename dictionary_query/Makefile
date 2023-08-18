CC = gcc

SRCS = test_dictionary_query.c ../utils/database.c ../logs/logger.c

INC_DIRS = -I../utils -I../logs

SQLITE_LIB = -lsqlite3

TARGET = test_dictionary_query

$(TARGET): $(SRCS)
	$(CC) $(INC_DIRS) -o $@ $^ $(SQLITE_LIB)

clean:
	rm -f $(TARGET)