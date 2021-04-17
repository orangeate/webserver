CXX ?= g++

DEBUG ?= 1
ifeq ($(DEBUG), 1)
    CXXFLAGS += -g
else
    CXXFLAGS += -O2

endif

server: main.cpp  ./config/config.cpp ./log/block_queue.cpp ./log/log.cpp ./epoll/epoll.cpp ./buffer/buffer.cpp ./http/http_request.cpp ./http/http_response.cpp ./http/http_conn.cpp ./lock/lock.cpp ./threadpool/thread_pool.cpp ./webserver/webserver.cpp 
	$(CXX) -o server  $^ $(CXXFLAGS) -lpthread -lmysqlclient

clean:
	rm  -r server