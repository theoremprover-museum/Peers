# P4 makefile for HP
P4ARCH = HP
AR = ar ruv
FC = f77
CC = gcc
# CC = cc
CLINKER = cc 
RANLIB = true
FLINKER = f77 
MDEP_LIBS = -lV3
# -lV3 is for sighold, etc.
MDEP_CFLAGS = 
MDEP_FFLAGS = 
P4_OBJ = $(P4_COMM_OBJ) $(P4_SOCK_OBJ) $(P4_SHMEM_OBJ)

RM = /bin/rm

CFLAGS = -g -I$(INCLUDEDIR) $(MDEP_CFLAGS) $(USER_CFLAGS)
#CFLAGS = -I$(INCLUDEDIR) $(MDEP_CFLAGS) $(USER_CFLAGS)

FFLAGS = -g -I$(FORTLIBDIR) $(MDEP_FFLAGS)
#FFLAGS = -I$(FORTLIBDIR) $(MDEP_FFLAGS)

# The automatic generation of Makefile in p4 inserts the option -g
# which is not found and therefore ignored by cc on HP.
# Option -g causes the compiler to generate additional information to be
# used by the symbolic debugger.

.SUFFIXES:	.o .c .f .h

.c.o:$(P)
	$(CC) $(CFLAGS) -c $*.c 

.f.o:$(P)
	$(FC) $(FFLAGS) -c $*.f 

dummy:	default

clean:
	$(RM) -f *~ *.o *.bak tags TAGS core

P4_HOME_DIR = /users/bonacina/p4/p4-1.3
INCLUDEDIR = $(P4_HOME_DIR)/include
LIBDIR = $(P4_HOME_DIR)/lib

LIBS = $(LIBDIR)/libp4.a $(MDEP_LIBS)

DFLAGS = -DHP_UX -DTP_NAMES -DTP_SIGNAL -DTP_FORK -DTP_RUSAGE

USER_CFLAGS = $(DFLAGS)

#---------------------- user defined files

FILES =   clocks.c avail.c term.c misc.c io.c options.c unify.c ac.c dioph.c btu.c btm.c demod.c discrim.c fpa.c list.c clause.c messages.c paramod.c peers.c lrpo.c

OBJECTS = clocks.o avail.o term.o misc.o io.o options.o unify.o ac.o dioph.o btu.o btm.o demod.o discrim.o fpa.o list.o clause.o messages.o paramod.o peers.o lrpo.o

#---------------------- make binary code

peers:$(P) main.o $(OBJECTS) $(LIBDIR)/libp4.a
	$(CLINKER) $(CFLAGS) -o peers main.o $(OBJECTS) $(LIBS)
	size peers
	chmod a+x peers
	
lint:
	lint $(DFLAGS) -I$(INCLUDEDIR)   main.c $(FILES)

proto:
	csh make_proto main.c $(FILES)

#---------------------- dependencies

main.o $(OBJECTS): Header.h
main.o $(OBJECTS): Types.h
main.o $(OBJECTS): Macros.h

main.o messages.o misc.o paramod.o peers.o: Peers.h

ac.o avail.o btm.o btu.o clause.o : Unify.h
demod.o dioph.o discrim.o fpa.o main.o paramod.o peers.o unify.o: Unify.h
avail.o io.o main.o paramod.o: Io.h
avail.o demod.o discrim.o fpa.o main.o: Index.h
avail.o paramod.o peers.o: Paramod.h

main.o $(OBJECTS): Cos.h
main.o $(OBJECTS): Proto.h
