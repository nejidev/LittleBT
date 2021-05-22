TARGET=LittleBT

all:
	g++ bencode.cpp main.cpp fileEntity.cpp -lcurl -lcrypto -g -o $(TARGET)

clean:
	rm -rf $(TARGET)
