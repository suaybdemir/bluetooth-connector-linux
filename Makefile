build: run

run:
	g++ main.cpp con.cpp con.h -o output
	./output

clean:
	rm output

