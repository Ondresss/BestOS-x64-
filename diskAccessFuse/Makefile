CC = gcc
CFLAGS = -Wall `pkg-config fuse3 --cflags`
LIBS = `pkg-config fuse3 --libs`
TARGET = main
SRCS = diskAccess.c strings.c main.c
IMAGE = sd.img
BACKUP = sd.img.bak

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

restore:
	@if [ -f $(BACKUP) ]; then \
       cp $(BACKUP) $(IMAGE); \
    fi

backup:
	cp $(IMAGE) $(BACKUP)

clean:
	rm -f $(TARGET)