TARGET=LittleBT

all:
	g++ bencode.cpp main.cpp file_entity.cpp -lcurl -lcrypto -g -o $(TARGET)

clean:
	rm -rf $(TARGET)
