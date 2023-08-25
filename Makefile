# 编译程序和标识
CC = gcc
CFLAGS = -Wall -g -fsanitize=thread
LIBS = -lsqlite3

# 源文件和目录
SERVER_OBJ_FILES = server/server.o dictionary_query/word_search.o history/history_manager.o logs/logger.o utils/networking.o utils/database.o user_management/user_register.o user_management/user_login.o server/thread.o server/server_handler.o
CLIENT_OBJ_FILES = client/client.o logs/logger.o utils/networking.o

# Targets
all: s c

s: $(SERVER_OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $(SERVER_OBJ_FILES) $(LIBS)

c: $(CLIENT_OBJ_FILES)
	$(CC) $(CFLAGS) -o $@ $(CLIENT_OBJ_FILES) $(LIBS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f s c $(SERVER_OBJ_FILES) $(CLIENT_OBJ_FILES)

