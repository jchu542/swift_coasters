CC=gcc
CFLAGS=
all:
	$(CC) $(CFLAGS) -o client client.c main.c connection.c easyzlib.c handlers.c
xml:
	$(gcc) $(CFLAGS) `xml2-config --cflags --libs` -o parseTest parser.c parserTest.c
