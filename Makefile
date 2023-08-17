CC = gcc
CFLAGS = -Wall -g

OBJS = main.o \
       concurrent/concurrent.o \
       user_management/user_register.o user_management/user_login.o user_management/user_auth.o \
       dictionary_query/word_search.o \
       history/record_save.o history/record_view.o \
       security/encryption.o security/error_handling.o \
       logs/logger.o \
       utils/networking.o utils/database.o

TARGET = OnlineEnglishDictionary

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)

