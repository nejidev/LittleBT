TARGET=LittleBT

all:
	g++ bencode.cpp main.cpp file_entity.cpp tracker.cpp common_utils.cpp schedule.cpp \
		-std=c++11 -lpthread -lcurl -lcrypto -g -o $(TARGET)

clean:
	rm -rf $(TARGET)
