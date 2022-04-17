GCC=gcc
FLAGS=-Wall -std=c99 -g 
LIBS=-pthread -lrt

SHM_FILE=shared_buffer_ADT.c

APP=app
WORKER=worker
VIEW=view

all: $(APP) $(WORKER) $(VIEW)

$(APP): 
	$(GCC) $(APP).c $(SHM_FILE) $(FLAGS) $(LIBS) -o $(APP)

$(WORKER): 
	$(GCC) $(WORKER).c $(FLAGS) $(LIBS) -o $(WORKER)

$(VIEW): 
	$(GCC) $(VIEW).c $(SHM_FILE) $(FLAGS) $(LIBS) -o $(VIEW)


clean:
	rm -rf $(APP) $(WORKER) $(VIEW)

.PHONY= all clean