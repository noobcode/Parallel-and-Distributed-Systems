OBJECTS = game_of_life.cpp
CFLAGS = -std=c++17 -L/usr/X11R6/lib -lm -lpthread -lX11 -I./
CC = g++
PROGRAM_NAME = game_of_life

$(PROGRAM_NAME):$(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) -o $(PROGRAM_NAME) 
	@echo " "
	@echo "@Carlo Alessi & @Luca Corbucci"
	@echo " "
	@echo "Compilation Completed!"
	@echo "Run with "
	@echo "./"$(PROGRAM_NAME)" size_matrix seed n_iterations n_workers"
clean:
	rm -f $(PROGRAM_NAME)
	@echo "Clean done"
