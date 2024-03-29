ifndef HDF5_DIR
    HDF5_DIR = /gpfs/alpine/csc300/world-shared/xl_build/hdf5/build_develop/install
endif

ifndef ABT_DIR
    ABT_DIR = /sw/summit/spack-envs/base/opt/linux-rhel8-ppc64le/gcc-8.3.1/argobots-1.1-sg55nep5da7y2lb5unasidlj4xwvnz5r
endif

ifndef SZ_DIR
    SZ_DIR = ../../SZ/install
endif

ifndef ASYNC_DIR
    ASYNC_DIR = /global/homes/h/houhun/hdf5vol/vol-async/src
endif

STATLIB = libasynchdf5.a
# Uncomment this line Linux builds:
DYNLIB = libh5async.so
# Uncomment this line MacOS builds:
# DYNLIB = libh5async.dylib

# CC = cc
# CC = gcc-9
CC = mpicc

HDF5_LIBDIR = $(shell if [ -d $$HDF5_DIR/lib ]; then echo "lib"; else echo "lib64"; fi)
ABT_LIBDIR =  $(shell if [ -d $$ABT_DIR/lib ]; then echo "lib"; else echo "lib64"; fi)

# These lines may need to be adjusted for your compiler environment:
DEBUG = -g -O0 -Wall
CFLAGS = $(DEBUG) -I$(ABT_DIR)/include -I$(ASYNC_DIR) -I$(HDF5_DIR)/include -I$(SZ_DIR)/include -lm

# Uncomment this line Linux builds:
LDFLAGS = $(DEBUG) -L$(ASYNC_DIR) -L$(ABT_DIR)/lib -L$(HDF5_DIR)/lib -L$(SZ_DIR)/lib -Wl,-rpath=$(ASYNC_DIR) -Wl,-rpath=$(ABT_DIR)/lib -Wl,-rpath=$(HDF5_DIR)/lib -Wl,-rpath=$(SZ_DIR)/lib -labt -lhdf5 -lh5async -lasynchdf5 -lSZ -lzlib -lzstd

# Uncomment this line MacOS builds:
# LDFLAGS = $(DEBUG) -L$(ASYNC_DIR) -L$(ABT_DIR)/lib -L$(HDF5_DIR)/lib -labt -lhdf5 -lasynchdf5 -lh5async 

SRC = async_test_serial.c async_test_serial2.c \
    async_test_multifile.c async_test_multifile_env.c \
    async_test_parallel.c async_test_parallel2.c async_test_parallel3.c \
    async_test_parallel4.c async_test_parallel5.c \
    async_test_serial_event_set.c async_test_serial_error_stack.c \
    async_test_serial_event_set_error_stack.c \
    overall_test.c

#async_test_no_fclose.c
OBJ = $(SRC:.c=.o)
EXE = $(SRC:.c=.exe)
DBG = $(SRC:.c=.exe.dSYM)

# Add '.exe' as a targetable suffix
.SUFFIXES: .exe

.c.exe: $(ASYNC_DIR)/$(DYNLIB) $(ASYNC_DIR)/$(STATLIB)
	$(CC) $(CFLAGS) $^ -o $*.exe $(LDFLAGS)

all: $(EXE)

check: 
	python3 ./pytest.py -p

check_serial: 
	python3 ./pytest.py

.PHONY: clean
clean:
	rm -rf $(OBJ) $(EXE) $(DBG) ./*.h5
