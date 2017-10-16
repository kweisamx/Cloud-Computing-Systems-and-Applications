gcc:
	gcc pthread_setaffinity_np_test.c -lm -lpthread -o pthread_setaffinity_np_test
	gcc pro1.c -lm -lpthread -o pro1
	gcc pro2.c -o pro2 -lpthread -lm
	gcc pro3.c -o pro3 -lpthread -lm
clean:
	rm pthread_setaffinity_np_test
	rm pro1
	rm pro2
	rm pro3

run:
	./pthread_setaffinity_np_test
	./pro1
	./pro2
	./pro3
