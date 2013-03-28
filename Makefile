run:
	cnet SIMULATION

top:
	gcc -std=c99 topology.c -o topology

update:
	@echo "updating c tags and doxygen files"
	ctags -dt *.c *.h
	doxygen Doxyfile 

clean:
	rm *.cnet *.o
