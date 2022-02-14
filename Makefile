CFLAGS = -ggdb3 -m64 -Wno-deprecated -Wno-write-strings -Wformat=2 -DDEBUG \
-Wold-style-cast
#CFLAGS = -O3

ybdd: bdd.o 
	g++ $(CFLAGS) -o ybdd bdd.o 

bdd.o: bdd.cxx bdd_node.h bdd_functs.h
	g++ $(CFLAGS) -c bdd.cxx

clean:
	rm -rf ybdd bdd.o
