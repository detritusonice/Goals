CC=g++

IDIR=include
LDIR=lib
ODIR=obj
TDIR=tests

CFLAGS=-std=c++11 -I$(IDIR)
TESTFLAGS=-pthread -no-pie
LIBS=
TESTLIBS=-lgtest -lpthread

#dependencies
_DEPS= goals.h statemachine.h
DEPS= $(patsubst %,$(IDIR)/%, $(_DEPS))

#main executable file is declared separately to avoid collision of main()s
MAIN=$(ODIR)/main.o

#object files are placed in separate directory
_OBJ=goals.o parser.o statemachine.o
OBJ= $(patsubst %,$(ODIR)/%,$(_OBJ))

#test object files have their own folder hierarchy
_TESTOBJ=tests.o
TESTOBJ = $(patsubst %,$(TDIR)/$(ODIR)/%,$(_TESTOBJ))

#make object files
$(ODIR)/%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

#make test files:
$(TDIR)/$(ODIR)/%.o: $(TDIR)/%.cpp $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

#make the application executable
goals: $(OBJ) $(MAIN)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

testgoals: $(TESTOBJ) $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) $(TESTFLAGS) $(LIBS) $(TESTLIBS)

#do not attempt to build a file named clean
.PHONY: clean

#remove all intermediate make process objects
clean:
	rm -f $(ODIR)/*.o *~ core $(IDIR)/*~ $(TDIR)/*~ $(TDIR)/$(ODIR)/*.o
