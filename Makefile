main: main.c
	gcc -g -o main main.c -lreadline

clean:
	rm main