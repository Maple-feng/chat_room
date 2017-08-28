a:a.c b.c
	gcc a.c -o a
	gcc b.c -o b
	gcc a_show.c -o a_show
	gcc b_show.c -o b_show
.PHONY:clean
clean:
	rm a b a_show b_show a.fifo b.fifo
