OBJECTS = af_main.cpp safe_queue.h autonomic_farm.h farm_emitter.h farm_worker.h farm_collector.h farm_manager.h farm_utility.h
CFLAGS = -Wall -std=c++17 -pthread -O2 #-pg
CC = g++
PROGRAM_NAME = af_main

$(PROGRAM_NAME):$(OBJECTS)
	$(CC) $(CFLAGS) -o $(PROGRAM_NAME) $(OBJECTS)
	@echo "=== Autonomic Farm ==="
	@echo "Author: Carlo Alessi"
	@echo ""
	@echo "Compilation Completed!"
	@echo "Run with "
	@echo "./"$(PROGRAM_NAME)" args..."
clean:
	rm -f $(PROGRAM_NAME)
	@echo "Clean done"
