all:
	gcc deserializer.c main.c serializer.c list.c -O2

clean:
	rm *.out
