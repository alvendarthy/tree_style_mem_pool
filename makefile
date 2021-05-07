CC = gcc -g
FLAGS = 
LINK = $(CC)

#OBJECTS = main.o  mg_array.o mg_pool.o mg_log.o mg_file.o  mg_parser.o mg_line_format.o\
			 #parser_cmd.o mg_line_format.o


OBJECTS = main.o mg_pool.o mg_log.o

all: main  clean

main: $(OBJECTS)
	$(LINK) -o main $(OBJECTS) 

main.o: main.c memory_manager.h mg_type.h \
  mg_pool.h mg_queue.h mg_array.h mg_string.h mg_file.h
	$(CC) $(FLAGS) -c $(*).c
#mg_array.o: mg_array.c mg_array.h memory_manager.h mg_type.h mg_pool.h \
#  mg_queue.h mg_string.h
#	$(CC) $(FLAGS) -c $(*).c
mg_pool.o: mg_pool.c mg_pool.h mg_queue.h
	$(CC) $(FLAGS) -c $(*).c
mg_log.o: mg_log.c  mg_log.h
	$(CC) $(FLAGS) -c $(*).c
#mg_file.o: mg_file.c  mg_file.h
#	$(CC) $(FLAGS) -c $(*).c
#mg_line_format.o: mg_line_format.c mg_line_format.h
#	$(CC) $(FLAGS) -c $(*).c
#mg_parser.o: mg_parser.c mg_parser.h
#	$(CC) $(FLAGS) -c $(*).c
#parser_cmd.o: parser_cmd.c parser_cmd.h
#	$(CC) $(FLAGS) -c $(*).c

clean:
	-rm -rf *.o
