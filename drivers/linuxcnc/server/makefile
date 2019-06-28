ifndef EMC2_HOME
LINUXCNC_INCLUDE=/usr/include/linuxcnc/
LINUXCNC_LIBS=/usr/lib/linuxcnc/
else
LINUXCNC_INCLUDE=$(EMC2_HOME)/include/
LINUXCNC_LIBS=$(EMC2_HOME)/lib/
endif

SRCDIR=src

IDIR =include
CFLAGS=-I$(IDIR) -I$(LINUXCNC_INCLUDE) -DULAPI -D_LINUX -O3

ODIR=obj

LINKFLAGS= -L../../../lib -L ../../../../libraries/build/lib -lm -llinuxcnchal -llinuxcnc -lnml -lpthread -llinuxcncini -llinear -ltv -luv

DEPS = $(wildcard include/*.h) $(wildcard $(LINUXCNC_INCLUDE)*.h)
#DEPS = $(patsubst %,$(IDIR)/%,$(_DEPS))

_OBJ = linuxcnc.o main.o cnccomms.o cncplugin.o cncserver.o timer.o
OBJ = $(patsubst %,$(ODIR)/%,$(_OBJ))


$(ODIR)/%.o: $(SRCDIR)/%.cpp $(DEPS)
	@echo Building $<
	@$(CXX) -w -c -o $@ $< $(CFLAGS)

cncremote-linuxcnc: $(OBJ)
	@echo linking $@
	@$(CXX) -o $@ $^ $(CFLAGS) $(LINKFLAGS)

.PHONY: clean

clean:
	rm -f $(ODIR)/*.o *~ core $(INCDIR)/*~ 
