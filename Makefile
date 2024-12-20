CC = gcc
CFLAGS = -Wall -pthread -I/opt/homebrew/opt/openssl@3/include
LDFLAGS = -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto

SRCDIR = libs
OBJDIR = server
SRC = server/server.c $(SRCDIR)/client_handler.c $(SRCDIR)/user_manager.c $(SRCDIR)/utils.c \
      $(SRCDIR)/message_handler.c $(SRCDIR)/user.c $(SRCDIR)/websocket_handshake.c $(SRCDIR)/room_manager.c $(SRCDIR)/protocol.c
OBJ = $(patsubst %.c, $(OBJDIR)/%.o, $(notdir $(SRC)))

all: $(OBJDIR)/server

$(OBJDIR)/server: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(OBJDIR)/server.o: server/server.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJDIR)/server $(OBJDIR)/*.o



# CC = gcc
# CFLAGS = -Wall -pthread -I/opt/homebrew/opt/openssl@3/include
# LDFLAGS = -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto


# SRCDIR = libs
# OBJDIR = server
# SRC = server/server.c $(SRCDIR)/client_handler.c $(SRCDIR)/user_manager.c $(SRCDIR)/utils.c $(SRCDIR)/message_handler.c $(SRCDIR)/user.c $(SRCDIR)/websocket_handshake.c $(SRCDIR)/room_manager.c
# OBJ = $(patsubst %.c, $(OBJDIR)/%.o, $(notdir $(SRC)))

# all: $(OBJDIR)/server

# $(OBJDIR)/server: $(OBJ)
# 	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# %.o: %.c
# 	$(CC) $(CFLAGS) -c -o $@ $<

# clean:
# 	rm -f $(OBJDIR)/server $(OBJ) server/*.o client/*.o


CC = gcc
CFLAGS = -Wall -pthread -I/opt/homebrew/opt/openssl@3/include
LDFLAGS = -L/opt/homebrew/opt/openssl@3/lib -lssl -lcrypto


SRCDIR = libs
OBJDIR = server
SRC = server/server.c $(SRCDIR)/client_handler.c $(SRCDIR)/user_manager.c $(SRCDIR)/utils.c $(SRCDIR)/message_handler.c $(SRCDIR)/user.c $(SRCDIR)/websocket_handshake.c $(SRCDIR)/room_manager.c
OBJ = $(patsubst %.c, $(OBJDIR)/%.o, $(notdir $(SRC)))

all: $(OBJDIR)/server

$(OBJDIR)/server: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJDIR)/server $(OBJ)

# SRCDIR = libs
# OBJDIR = server
# SRC = server/server.c $(SRCDIR)/client_handler.c $(SRCDIR)/user_manager.c $(SRCDIR)/utils.c $(SRCDIR)/message_handler.c $(SRCDIR)/user.c $(SRCDIR)/websocket_handshake.c
# OBJ = $(patsubst %.c, $(OBJDIR)/%.o, $(notdir $(SRC)))

# all: $(OBJDIR)/server

# $(OBJDIR)/server: $(OBJ)
# 	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

# $(OBJDIR)/%.o: $(SRCDIR)/%.c
# 	@mkdir -p $(OBJDIR)
# 	$(CC) $(CFLAGS) -c -o $@ $<

# $(OBJDIR)/server.o: server/server.c
# 	@mkdir -p $(OBJDIR)
# 	$(CC) $(CFLAGS) -c -o $@ $<

# clean:
# 	rm -f $(OBJDIR)/server $(OBJDIR)/*.o
