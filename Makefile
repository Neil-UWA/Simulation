run:
	cnet SIMULATION

update:
	@echo "updating c tags and doxygen files"
	ctags -dt *.c *.h
	doxygen Doxyfile 

clean:
	rm *.cnet *.o
