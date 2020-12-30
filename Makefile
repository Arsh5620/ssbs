all:
	gcc deserializer.c main.c serializer.c list.c -O2 -ggdb

clean:
	rm deserializer.o main.o serializer.o list.o a.out