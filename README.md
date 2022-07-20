# Artifacts of SC’22 paper “Accelerating Parallel Write via Deeply Integrating Predictive Lossy Compression with HDF5”

This demo has been successfully run on multiple systems including OLCF Summit supercomputer, ANL Bebop cluster, WSU Kamiak cluster, and Chameleon Cloud.

## Environment

- OS: CentOS (>= 7.8)
- Compiler: GCC (>=4.8.5)
- MPI: GCC built OpenMPI (>=4.1.1) or MPICH (>=3.3.1)

   * For users in HPC systems (such as Summit), please try “module load openmpi” to load the MPI library. 

   * For users in Chameleon Cloud, please follow [this instruction](https://chameleoncloud.readthedocs.io/en/latest/getting-started/index.html) to create an instance using ANL-MPICH image.

- Other dependencies: parallel HDF5, Argobots, and SZ (please follow Step 1 and 2 to build them).

## Step 1: Download Dependencies

### 1.1 Setup test directory

```
export TEST_HOME=$(pwd)
```

### 1.2 Download code of HDF5, Argobots, SZ, and our HDF5-SZ

```
cd $TEST_HOME
git clone https://github.com/HDFGroup/hdf5
git clone https://github.com/pmodels/argobots
git clone https://github.com/szcompressor/SZ
git clone https://github.com/jinsian/HDF5-SZ
```
    
### 1.3 Configure home directory of each software

```
export ABT_HOME=$TEST_HOME/argobots
export H5_HOME=$TEST_HOME/hdf5
export SZ_HOME=$TEST_HOME/SZ
export VOL_HOME=$TEST_HOME/HDF5-SZ
```

## Step 2: Build Dependencies

### 2.1 Build parallel HDF5

```   
cd $H5_HOME
./autogen.sh
mkdir install
./configure --prefix=$H5_HOME/install --enable-parallel --enable-threadsafe --enable-unsupported
make -j8
make install
```
    
Please use the below command to double check if the installed HDF5 supports parallel mode.

```
$H5_HOME/install/bin/h5pcc -showconfig
```

You should find *Parallel HDF5: yes*.

### 2.2 Build Argobots

```
cd $ABT_HOME
./autogen.sh
mkdir install
./configure --prefix=$ABT_HOME/install
make -j8
make install
```

### 2.3 Build SZ

```
cd $SZ_HOME
mkdir install
./configure --prefix=$SZ_HOME/install
make -j8
make install
```

### 2.4 Build asynchronous VOL connector with SZ

```    
cd $VOL_HOME/src
export HDF5_DIR=$H5_HOME/install
export ABT_DIR=$ABT_HOME/install
make
```

## Step 3: Build HDF5-SZ Demo

### 3.1 Set environment variables

```
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$SZ_HOME/install/lib
export LD_LIBRARY_PATH=$VOL_HOME/src:$H5_HOME/install/lib:$ABT_HOME/install/lib:$LD_LIBRARY_PATH
export HDF5_PLUGIN_PATH="$VOL_HOME/src"
export HDF5_VOL_CONNECTOR="async under_vol=0;under_info={}"
```

### 3.2 Compile HDF5-SZ demo code

```
cd $VOL_HOME/test
export H5_DIR=$HDF5_DIR
export ASYNC_DIR=$VOL_HOME/src
make
```
    
## Step 4. Test HDF5-SZ Demo

### 4.1 Download dataset

You can download the test dataset (i.e., a 2.7 GB Nyx cosmology dataset with a dimension of 512x512x512) from SDRBench with the following command.

```
cd $VOL_HOME/test
wget https://g-8d6b0.fd635.8443.data.globus.org/ds131.2/Data-Reduction-Repo/raw-data/EXASKY/NYX/SDRBENCH-EXASKY-NYX-512x512x512.tar.gz
tar -zxvf SDRBENCH-EXASKY-NYX-512x512x512.tar.gz
```

### 4.2 Run test

Run the overall performance test. Note that you may need to change the execution command from mpirun to the corresponding MPI launch command in your system. If you are using the MPICH on Chameleon, please change *mpirun* to *mpiexec*.

```
mpirun -n 16 overall_test.exe
```

This demo code is to first distribute the example data to 16 processors where each holds a partition of 6 64x64x64 data fields and then write these data to a shared HDF5 file using three different solutions (with different write performances): (1) write original data, (2) write compressed data with SZ lossy compression filter, and (3) write compressed data and overlap compression with I/O + compression schedule optimization.

<img width="300" alt="example result" src="https://user-images.githubusercontent.com/50967682/178728197-5ea29eca-17cc-4738-a796-db6ca82567a9.png">
