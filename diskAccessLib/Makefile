CC = gcc
CFLAGS = -Wall -Wextra -std=c11
TARGET = main
SRCS = diskAccess.c strings.c main.c
IMAGE = sd.img
BACKUP = sd.img.bak

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(SRCS) -o $(TARGET)

restore:
	@if [ -f $(BACKUP) ]; then \
		cp $(BACKUP) $(IMAGE); \
		echo "Disk obraz $(IMAGE) byl obnoven ze zálohy."; \
	else \
		echo "Chyba: Soubor $(BACKUP) neexistuje! Nejdřív si vytvoř zálohu."; \
	fi

backup:
	cp $(IMAGE) $(BACKUP)

clean:
	rm -f $(TARGET)