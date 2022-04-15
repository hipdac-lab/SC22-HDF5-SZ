#include <stdio.h>
#include <mpi.h>
#include <time.h>
//#include "/opt/apps/hdf5/1.10.6/gcc/7.3.0/openmpi/4.0.3/include/hdf5.h"
#include "/blues/gpfs/software/centos7/spack/opt/spack/linux-centos7-x86_64/gcc-8.2.0/hdf5-1.10.5-vozfsah/include/hdf5.h"

int main() {
        int N = 5920000;
        N = N * 10;
        hsize_t dims[1];
        hsize_t dims2[1];
        hsize_t dims3[1];
        int n;
        int rank;
        clock_t start, end;
        double t1, t2;
        double cpu_time_used;
        MPI_Init( NULL, NULL );
        MPI_Comm_size(MPI_COMM_WORLD, &n);
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        dims[0] = N;
        dims2[0] = dims[0] * n;
        dims3[0] = dims[0] * n;
        printf("%lu %lu\n", dims[0], dims2[0]);

        if (rank == 0)
                printf("test, world = %d, rank = %d\n", n, rank);
        hid_t file_id;

        hid_t fapl_id = H5Pcreate(H5P_FILE_ACCESS);
        H5Pset_fapl_mpio(fapl_id, MPI_COMM_WORLD, MPI_INFO_NULL);
        file_id = H5Fcreate("myparfile.h5", H5F_ACC_TRUNC, H5P_DEFAULT, fapl_id);

        hid_t dxpl_id = H5Pcreate(H5P_DATASET_XFER);
        H5Pset_dxpl_mpio(dxpl_id, H5FD_MPIO_COLLECTIVE);

        dims[0] = dims[0] * ((rank+1)/n) *2;
        hid_t memspace = H5Screate_simple(1, dims, NULL);

        hid_t filespace = H5Screate_simple(1, dims2, NULL);
        //hsize_t offset = rank * N;
        hsize_t offset = N * (1/n) * (rank+1)*rank/2;
        H5Sselect_hyperslab(filespace, H5S_SELECT_SET, &offset, NULL, dims, NULL);

        float somedata0[dims[0]];
        for (int i; i < dims[0]; i++) {
                        somedata0[i] = i+12;
        }
  
        hid_t space_id = H5Screate_simple(1, dims2, NULL);
        hid_t dset_id = H5Dcreate(file_id, "dataset0", H5T_NATIVE_FLOAT, space_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);

        if (rank == 0)
                printf("Writing from rank %d\n", rank);
        MPI_Barrier(MPI_COMM_WORLD);
        t1 = MPI_Wtime();

        H5Dwrite(dset_id, H5T_NATIVE_FLOAT, memspace, filespace, dxpl_id, somedata0);

        MPI_Barrier(MPI_COMM_WORLD);
        t2 = MPI_Wtime();
        cpu_time_used = t2 - t1;
        if (rank == (n-1))
                printf("Finished rank %d, time = %f\n", rank, cpu_time_used);

        H5Dclose(dset_id);
        H5Sclose(filespace);
        H5Sclose(memspace);
        H5Pclose(dxpl_id);
        H5Fclose(file_id);
        H5Pclose(fapl_id);

        MPI_Finalize();

        return 0;
}
