all : test cond

test : test.cpp ../src/coroutine.cpp
	g++ -g -Wall -o $@ $^

cond : cond.cpp ../src/coroutine.cpp
	g++ -g -Wall -o $@ $^

clean :
	rm test cond
        
