###############################################################################
#
#  Makefile
#
#  Unit tests for Ludwig
#
#  make 
#  make test
#
#  with compile and run the tests, resepectively.
#
#  Edinburgh Soft Matter and Statistical Physics Group and
#  Edinburgh Parallel Computing Centre
#
#  (c) 2010-2019 The University of Edinburgh
#
#  Contributing authors:
#  Kevin Stratford (kevin@epcc.ed.ac.uk)
#
###############################################################################

include ../../Makefile.mk

ifeq (${BUILD},parallel)
MPIRUN_NTASKS=1
endif

#------------------------------------------------------------------------------
# Files
#------------------------------------------------------------------------------

SOURCES = $(wildcard *.c)
OBJECTS = ${SOURCES:.c=.o}

#------------------------------------------------------------------------------
# Compilation options, etc.
#------------------------------------------------------------------------------

SRC  = $(ROOT_DIR)./src
INCL = -I$(SRC) $(TARGET_INC_PATH) $(MPI_INC_PATH)

BLIBS = $(MPI_LIB_PATH) $(MPI_LIB) $(TARGET_LIB_PATH) $(TARGET_LIB)
LIBS = $(BLIBS) $(SRC)/libludwig.a -lm

MPI_RUN = $(LAUNCH_MPIRUN_CMD) $(MPIRUN_NTASK_FLAG) $(MPIRUN_NTASKS)

#------------------------------------------------------------------------------
#  Rules
#------------------------------------------------------------------------------

default:
	@echo "Build unit tests"
	$(MAKE) build

build:	$(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) $(LIBS)

test:
	$(MPI_RUN) ./a.out

clean:
	$(RM) core *.o a.out

#------------------------------------------------------------------------------
#  Implicit Rules
#------------------------------------------------------------------------------

.SUFFIXES:
.SUFFIXES: .c .o

.c.o:
	$(CC) $(MODEL) $(CFLAGS) $(INCL) -c $*.c
