CC          = c++ 

#-----------------------------------------
#Optimization ----------------------------
OPT   = -O3 -Wno-deprecated -g

#-----------------------------------------

TARGETS = seamcarving

OBJECTS = seamcarving.o slVector.o 

#-----------------------------------------

LIBS = $(GL_LIB) 

CCOPTS = $(OPT) $(INCS) 
LDOPTS = $(OPT) $(INCS) $(LIBS) 

#-----------------------------------------
#-----------------------------------------

all: $(TARGETS)


clean:
	/bin/rm -f *.o $(TARGETS)

#-----------------------------------------
#-----------------------------------------

seamcarving: $(OBJECTS)
	$(CC) $(OBJECTS) $(LDOPTS) -o seamcarving

seamcarving.o : seamcarving.cpp
	$(CC) -Wall -c seamcarving.cpp

sLvector.o: sLvector.cpp sLvector.H
	$(CC) -Wall -c sLvector.cpp
#-----------------------------------------
#-----------------------------------------

.cpp.o: 
	$(CC) $(CCOPTS) -c $< 

#-----------------------------------------
#-----------------------------------------















