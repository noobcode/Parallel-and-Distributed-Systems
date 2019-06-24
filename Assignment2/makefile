OBJECTS = main.cpp
CFLAGS = -std=c++11 -pthread
CC = g++
PROGRAM_NAME = main

$(PROGRAM_NAME):$(OBJECTS)
	$(CC) $(CFLAGS) -o $(PROGRAM_NAME) $(OBJECTS)
	@echo " "
	@echo "@Carlo Alessi"
	@echo " "
	@echo "Compilation Completed!"
	@echo "Run with "
	@echo "./"$(PROGRAM_NAME)" n_tasks n_workers"
clean:
	rm -f $(PROGRAM_NAME)
	@echo "Clean done"
