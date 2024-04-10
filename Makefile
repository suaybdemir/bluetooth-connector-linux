build: run

run:
	g++ main.cpp con.h Entities/connection.h Entities/pair.h Entities/scanBluetoothDevices.h -o output
	./output

clean:
	rm output

