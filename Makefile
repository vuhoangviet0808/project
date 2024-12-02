CC = gcc
CFLAGS = -Wall -pthread
LDFLAGS = -ljson-c

SRCDIR = libs
OBJDIR = server
SRC = server/server.c $(SRCDIR)/client_handler.c $(SRCDIR)/user_manager.c $(SRCDIR)/utils.c $(SRCDIR)/message_handler.c $(SRCDIR)/user.c
OBJ = $(SRC:.c=.o)

all: $(OBJDIR)/server

$(OBJDIR)/server: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJDIR)/server $(OBJ) server/*.o client/*.o
