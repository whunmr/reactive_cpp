CPPFLAGS=-g -I/usr/include -I/home/j/lab/gtest-1.7.0/include -I/home/j/lab/mockcpp/mockcpp/include
LDFLAGS=
LDLIBS=-L/usr/include/mockcpp/lib/ -L/home/j/lab/gtest-1.7.0/lib -lgtest -lpthread -lmockcpp

SRCS=$(shell find . -iname *.cpp)
OBJS=$(subst .cpp,.o,$(SRCS))

all: $(OBJS)
	g++ $(CPPFLAGS) $(LDFLAGS) -g -o main $^ $(LDLIBS) && ./main --gtest_filter="*"

clean:
	rm -rdf $(OBJS) main main.dSYM

pp:
	g++ -gstabs -I/home/j/lab/gtest-1.6.0/include -I/home/j/lab/mockcpp/include -I/home/j/lab/mockcpp -I/home/j/lab/mockcpp/3rdparty -I/home/j/lab/boost  -E  test/basic_msg_interaction_test7.cpp > a.txt
