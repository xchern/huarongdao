default: a.out
	./a.out

clean:
	rm -f *.png && true
	rm -f a.out && true
	rm -f graph.dot && true

a.out: main.cc
	g++ main.cc -O3 -lGL -lglut
