CNET	= cnet
#CNET	= cnet

run:
	@make clean
	gcc -std=c99 optimize.c -o optimize
	$(CNET) -NTqg -e4h  SIMULATION

update:
	@echo "updating c tags and doxygen files"
	ctags -dt *.c *.h
#	doxygen Doxyfile 

clean:
	@rm -f *.cnet *.o 
