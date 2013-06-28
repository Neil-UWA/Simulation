CNET	= cnet

CC			=	gcc	
CFLAGS		=	-std=c99  
LIBS		=	-lm

TARGET		=	optimize
SOURCES		=	ga.c globals.c optimize.c brute.c
HEADERS		=	brute.h ga.h optimize.h	

GA			=	-DUSE_GA_OPTIMISE
BRUTE		=	-DUSE_BRUTE_SEARCH

MAP_WIDTH	=	-DWIDTH=500
MAP_HEIGH	=	-DHEIGHT=500

WLAN		=	-DUSE_WLAN_MODEL
ITERATION 	=	-DMAX_ITERATION=100	
POP			=	-DMAX_POP=50
MUTATIION	=	-DP_MUTAION=0.01
CROSSOVER	=	-DP_CROSSOVER=0.6
NUM_AP		=	-DNUM_AP=11

ga: 
	$(CC) $(CFLAGS)  -o $(TARGET) $(SOURCES) $(HEADERS) $(GA) $(ITERATION) $(POP) $(MAP_WIDTH) $(MAP_HEIGH) $(MUTATIION) $(CROSSOVER) $(NUM_AP) $(WLAN) $(LIBS) -g


brute: $(BRUTE_SOURCES)
	$(CC) $(CFLAGS)  -o $(TARGET) $(SOURCES) $(HEADERS) $(BRUTE) $(MAP_WIDTH) $(MAP_HEIGH) $(WLAN) $(LIBS) 


run:
	@make clean
	$(CNET) -NTq -e4h -DUSE_FSL_WLAN_MODEL -DMAX_CLIENTS_PER_AP=30 ./Testing/B/4/RESULT 

update:
	@echo "updating c tags and doxygen files"
	ctags -dt *.c *.h
	doxygen Doxyfile 

clean:
	@rm -f *.cnet *.o $(TARGET)  
