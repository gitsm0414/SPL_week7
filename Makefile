target=cron
object=cron.o

$(target): $(object)
	gcc -o $@ $^
cron.o: cron.c
	gcc -c $^
clean:
	rm $(target) $(object) 
