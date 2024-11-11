CC = gcc
CFLAGS = -Wall -pthread

SRCDIR = libs
OBJDIR = server
SRC = server/server.c $(SRCDIR)/client_handler.c $(SRCDIR)/user_manager.c $(SRCDIR)/utils.c $(SRCDIR)/message_handler.c
OBJ = $(SRC:.c=.o)

all: $(OBJDIR)/server

$(OBJDIR)/server: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJDIR)/server $(OBJDIR)/*.o
