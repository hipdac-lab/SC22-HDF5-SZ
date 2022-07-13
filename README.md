# HDF5 With SZ Lossy Compression

Some configuration parameters used in the instructions:

        VOL_DIR : directory of HDF5-SZ repository
        ABT_DIR : directory of Argobots source code
        H5_DIR  : directory of HDF5 source code
        SZ_DIR  : directory of SZ source code
        
## Step 1: Preparation

1.1 Download the code of this repository with Argobots git submodule.

    > git clone https://github.com/jinsian/HDF5-SZ
    > git clone https://github.com/pmodels/argobots
    > git clone https://github.com/szcompressor/SZ

1.2 Download the HDF5 source code, you can skip this step if you have HDF5 module avalible.

    > git clone https://github.com/HDFGroup/hdf5.git

## Step 2: Installation

2.1 Compile SZ.

    > cd $SZ_DIR
    > mkdir install
    > ./configure --prefix=[CURRENT_DIR]/install (Please use --enable-fortran if you need Fortran interface)
    > make
    > make install

2.2 Compile HDF5, you can skip this step if you have HDF5 module avalible.

    > cd $H5_DIR
    > ./autogen.sh  (may skip this step if the configure file exists)
    > ./configure --prefix=$H5_DIR/install --enable-parallel --enable-threadsafe --enable-unsupported #(may need to add CC=cc or CC=mpicc)
    > make && make install

2.3 Compile Argobots.

    > cd $ABT_DIR
    > ./autogen.sh  (may skip this step if the configure file exists)
    > ./configure --prefix=$ABT_DIR/install #(may need to add CC=cc or CC=mpicc)
    > make && make install
    # Note: using mpixlC on Summit will result in Argobots runtime error, use xlC or gcc instead.

2.4 Compile Asynchronous VOL connector with SZ lossy compression implimentations.

    > cd $VOL_DIR/src
    > # Edit "Makefile"
    > # Use a template Makefile: e.g. "cp Makefile.summit Makefile"
    > # Change the path of HDF5_DIR and ABT_DIR to $H5_DIR/install and $ABT_DIR/install
    > # (Optional) update the compiler flag macros: DEBUG, CFLAGS, LIBS, ARFLAGS
    > # (Optional) comment/uncomment the correct DYNLDFLAGS & DYNLIB macros
    > make

## Step 3: Set Environment Variables

Set the following environmental variable before running the tests:

    > export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SZ_DIR/install/lib
    > export LD_LIBRARY_PATH=$VOL_DIR/src:$H5_DIR/install/lib:$ABT_DIR/install/lib:$LD_LIBRARY_PATH
    > export HDF5_PLUGIN_PATH="$VOL_DIR/src"
    > export HDF5_VOL_CONNECTOR="async under_vol=0;under_info={}" 
    > (optional) export MPICH_MAX_THREAD_SAFETY=multiple # Some systems like Cori@NERSC need this to support MPI_THREAD_MULTIPLE
    
## Step 5. Download Test Dataset

You can download the test dataset from SDRBench with the following command:

    > cd $VOL_DIR/test
    > wget https://g-8d6b0.fd635.8443.data.globus.org/ds131.2/Data-Reduction-Repo/raw-data/EXASKY/NYX/SDRBENCH-EXASKY-NYX-512x512x512.tar.gz
    > tar -zxvf SDRBENCH-EXASKY-NYX-512x512x512.tar.gz

This will download a 2.7 GB Nyx cosmology dataset with a dimension of 512x512x512.

## Step 4. Tests

    > cd $VOL_DIR/test
    > # Edit "Makefile":
    > # Update H5_DIR, ABT_DIR and ASYNC_DIR to the correct paths of their installation directory
    > make
    
Run the overall performance test, note you may need to edit the command for mpirun in with the corresponding MPI launch command.:

    > jsrun -n16 overall_test.exe

This will distribute the data onto 16 processors wich each processor hold a data partition size of 64x64x64 on 6 data fields. Then it will write these data to a shared HDF5 file with 3 solutions and compare the write performance: (1) write original data; (2) write compressed data with SZ lossy compression filter; (3) write compressed data and overlap compression with I/O + compression schedule optimization.

<img width="181" alt="截屏2022-07-13 上午6 57 02" src="https://user-images.githubusercontent.com/50967682/178728197-5ea29eca-17cc-4738-a796-db6ca82567a9.png">
