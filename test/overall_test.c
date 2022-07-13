#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "hdf5.h"
#include "mpi.h"
#include "h5_async_lib.h"
#include "sz.h"

#define DIMLEN 1024

int
main(int argc, char *argv[])
{
    hid_t       file_id, grp_id, dset1_id, dset0_id, dset2_id, dset3_id, dset4_id, dset5_id, fspace_id, mspace_id, dxpl_id;
    const char *file_name = "async_test_parallel.h5";
    const char *grp_name  = "Group";
    int *       data0_write, *data0_read, *data1_write, *data1_read, *data2_write, *data2_read, *data3_write, *data3_read, *data4_write, *data4_read, *data5_write, *data5_read;
    int         i, is_verified, tmp_size;
    hsize_t     ds_size[2] = {DIMLEN, DIMLEN};
    hsize_t     my_size[2] = {DIMLEN, DIMLEN};
    hsize_t     offset[2]  = {0, 0};
    herr_t      status;
    hid_t       async_fapl;
    int         proc_num, my_rank, ret = 0;
    int         mpi_thread_lvl_provided = -1;
    double      t0, t1, t2;

    MPI_Init_thread(&argc, &argv, MPI_THREAD_MULTIPLE, &mpi_thread_lvl_provided);
    assert(MPI_THREAD_MULTIPLE == mpi_thread_lvl_provided);
    MPI_Comm_size(MPI_COMM_WORLD, &proc_num);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    double  ratio = 16;
    hbool_t op_failed;
    size_t  n_running;
    hid_t   es_id = H5EScreate();

    my_size[0] = DIMLEN / ratio;
    ds_size[0] = DIMLEN * proc_num / ratio;
    ds_size[1] = DIMLEN * proc_num;
    tmp_size   = DIMLEN * proc_num / ratio;
    if (my_rank == proc_num - 1) {
        if (DIMLEN % proc_num != 0)
            my_size[0] += DIMLEN % proc_num;
    }

    async_fapl = H5Pcreate(H5P_FILE_ACCESS);

    /* status = H5Pset_vol_async(async_fapl); */
    /* assert(status >= 0); */
    status = H5Pset_fapl_mpio(async_fapl, MPI_COMM_WORLD, MPI_INFO_NULL);
    assert(status >= 0);

    file_id = H5Fcreate_async(file_name, H5F_ACC_TRUNC, H5P_DEFAULT, async_fapl, es_id);
    if (file_id < 0) {
        fprintf(stderr, "Error with file create\n");
        ret = -1;
        goto done;
    }

    grp_id = H5Gcreate_async(file_id, grp_name, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, es_id);
    if (grp_id < 0) {
        fprintf(stderr, "Error with group create\n");
        ret = -1;
        goto done;
    }

    data0_write = calloc(sizeof(int), DIMLEN * DIMLEN);
    data1_write = calloc(sizeof(int), DIMLEN * DIMLEN);
    data2_write = calloc(sizeof(int), DIMLEN * DIMLEN);
    data3_write = calloc(sizeof(int), DIMLEN * DIMLEN);
    data4_write = calloc(sizeof(int), DIMLEN * DIMLEN);
    data5_write = calloc(sizeof(int), DIMLEN * DIMLEN);
    data0_read  = calloc(sizeof(int), DIMLEN * DIMLEN);
    data1_read  = calloc(sizeof(int), DIMLEN * DIMLEN);
    for (i = 0; i < DIMLEN * DIMLEN; ++i) {
        data0_write[i] = my_rank * tmp_size * DIMLEN + i;
        data1_write[i] = (my_rank * tmp_size * DIMLEN + i) * 2;
        data2_write[i] = (my_rank * tmp_size * DIMLEN + i) * 3;
        data3_write[i] = (my_rank * tmp_size * DIMLEN + i) * 4;
        data4_write[i] = (my_rank * tmp_size * DIMLEN + i) * 5;
        data5_write[i] = (my_rank * tmp_size * DIMLEN + i) * 6;
    }

    // Set collective operation
    dxpl_id = H5Pcreate(H5P_DATASET_XFER);
    H5Pset_dxpl_mpio(dxpl_id, H5FD_MPIO_COLLECTIVE);

    mspace_id = H5Screate_simple(2, my_size, NULL);
    fspace_id = H5Screate_simple(2, ds_size, NULL);
            
    dset0_id = H5Dcreate_async(grp_id, "dset0", H5T_NATIVE_INT, fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, es_id);
    if (dset0_id < 0) {
        fprintf(stderr, "Error with dset0 create\n");
        ret = -1;
        goto done;
    }

    dset1_id = H5Dcreate_async(grp_id, "dset1", H5T_NATIVE_INT, fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, es_id);
    if (dset1_id < 0) {
        fprintf(stderr, "Error with dset1 create\n");
        goto done;
    }
    dset2_id = H5Dcreate_async(grp_id, "dset2", H5T_NATIVE_INT, fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, es_id);
    if (dset2_id < 0) {
        fprintf(stderr, "Error with dset2 create\n");
        goto done;
    }
    dset3_id = H5Dcreate_async(grp_id, "dset3", H5T_NATIVE_INT, fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, es_id);
    dset4_id = H5Dcreate_async(grp_id, "dset4", H5T_NATIVE_INT, fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, es_id);
    dset5_id = H5Dcreate_async(grp_id, "dset5", H5T_NATIVE_INT, fspace_id, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT, es_id);

    offset[0] = my_rank * (DIMLEN / ratio);
    offset[1] = 0;
    H5Sselect_hyperslab(fspace_id, H5S_SELECT_SET, offset, NULL, my_size, NULL);

    /* printf("Rank %d: write offset (%d, %d), size (%d, %d)\n", my_rank, offset[0], offset[1], my_size[0],
     * my_size[1]); */

    double comp;
    double through;
    through = ((240.6 - 101.7) * pow(3.0, -1.716) * pow((32/ratio), 1.716) + 101.7);
    comp = my_size[0] * my_size[1] * ratio * 4 / 1000000 / ((240.6 - 101.7) * pow(3.0, -1.716) * pow((32/ratio), 1.716) + 101.7);


    if (my_rank == 0)
        fprintf(stderr, "Write time compare:\n");

    t0 = MPI_Wtime();
    for (i = 1; i < 16; i++) {
          status = H5Dwrite_async(dset0_id, H5T_NATIVE_INT, mspace_id, fspace_id, dxpl_id, data0_write, es_id);
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &n_running, &op_failed);
    }
    for (i = 1; i < 16; i++) {
        status = H5Dwrite_async(dset1_id, H5T_NATIVE_INT, mspace_id, fspace_id, dxpl_id, data1_write, es_id);
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &n_running, &op_failed);
    }
    for (i = 1; i < 16; i++) {
        status = H5Dwrite_async(dset2_id, H5T_NATIVE_INT, mspace_id, fspace_id, dxpl_id, data2_write, es_id);
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &n_running, &op_failed);
    }
    for (i = 1; i < 16; i++) {
        status = H5Dwrite_async(dset3_id, H5T_NATIVE_INT, mspace_id, fspace_id, dxpl_id, data3_write, es_id);
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &n_running, &op_failed);
    }
    for (i = 1; i < 16; i++) {
        status = H5Dwrite_async(dset4_id, H5T_NATIVE_INT, mspace_id, fspace_id, dxpl_id, data4_write, es_id);
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &n_running, &op_failed);
    }
    for (i = 1; i < 16; i++) {
        status = H5Dwrite_async(dset5_id, H5T_NATIVE_INT, mspace_id, fspace_id, dxpl_id, data5_write, es_id);
        H5ESwait(es_id, H5ES_WAIT_FOREVER, &n_running, &op_failed);
    }
    t2 = MPI_Wtime();
    if (my_rank == 0)
        fprintf(stderr, "Original: %f s\n", t2-t0);

    t0 = MPI_Wtime();
    t1 = MPI_Wtime();
    while (MPI_Wtime() - t1 < comp) {}
    status = H5Dwrite_async(dset0_id, H5T_NATIVE_INT, mspace_id, fspace_id, dxpl_id, data0_write, es_id);
    H5ESwait(es_id, H5ES_WAIT_FOREVER, &n_running, &op_failed);
    t1 = MPI_Wtime();
    while (MPI_Wtime() - t1 < comp) {}
    status = H5Dwrite_async(dset1_id, H5T_NATIVE_INT, mspace_id, fspace_id, dxpl_id, data1_write, es_id);
    H5ESwait(es_id, H5ES_WAIT_FOREVER, &n_running, &op_failed);
    t1 = MPI_Wtime();
    while (MPI_Wtime() - t1 < comp) {}
    status = H5Dwrite_async(dset2_id, H5T_NATIVE_INT, mspace_id, fspace_id, dxpl_id, data1_write, es_id);
      H5ESwait(es_id, H5ES_WAIT_FOREVER, &n_running, &op_failed);
    t1 = MPI_Wtime();
    while (MPI_Wtime() - t1 < comp) {}
    status = H5Dwrite_async(dset3_id, H5T_NATIVE_INT, mspace_id, fspace_id, dxpl_id, data1_write, es_id);
    H5ESwait(es_id, H5ES_WAIT_FOREVER, &n_running, &op_failed);
    t1 = MPI_Wtime();
    while (MPI_Wtime() - t1 < comp) {}
    status = H5Dwrite_async(dset4_id, H5T_NATIVE_INT, mspace_id, fspace_id, dxpl_id, data1_write, es_id);
    H5ESwait(es_id, H5ES_WAIT_FOREVER, &n_running, &op_failed);
    t1 = MPI_Wtime();
    while (MPI_Wtime() - t1 < comp) {}
    status = H5Dwrite_async(dset5_id, H5T_NATIVE_INT, mspace_id, fspace_id, dxpl_id, data1_write, es_id);
    H5ESwait(es_id, H5ES_WAIT_FOREVER, &n_running, &op_failed);
    t2 = MPI_Wtime();
    if (my_rank == 0)
        fprintf(stderr, "Previous: %f s\n", t2-t0);

    t1 = MPI_Wtime();
    while (MPI_Wtime() - t1 < comp) {}

    // W0, R0, W1, R1, W1', W0'
    t1 = MPI_Wtime();
    t0 = MPI_Wtime();
    status = H5Dwrite_async(dset0_id, H5T_NATIVE_INT, mspace_id, fspace_id, dxpl_id, data0_write, es_id);
    t2 = MPI_Wtime();
    if (status < 0) {
        fprintf(stderr, "Error with dset 0 write\n");
        ret = -1;
        goto done;
    }

    t1 = MPI_Wtime();
    while (MPI_Wtime() - t1 < comp) {}
    t1 = MPI_Wtime();
    status = H5Dwrite_async(dset1_id, H5T_NATIVE_INT, mspace_id, fspace_id, dxpl_id, data1_write, es_id);
    t2 = MPI_Wtime();
    if (status < 0) {
        fprintf(stderr, "Error with dset 0 write\n");
        ret = -1;
        goto done;
    }
    t1 = MPI_Wtime();
    while (MPI_Wtime() - t1 < comp) {}
    status = H5Dwrite_async(dset2_id, H5T_NATIVE_INT, mspace_id, fspace_id, dxpl_id, data1_write, es_id);
    t1 = MPI_Wtime();
    while (MPI_Wtime() - t1 < comp) {}
    status = H5Dwrite_async(dset3_id, H5T_NATIVE_INT, mspace_id, fspace_id, dxpl_id, data1_write, es_id);
    t1 = MPI_Wtime();
    while (MPI_Wtime() - t1 < comp) {}
    status = H5Dwrite_async(dset4_id, H5T_NATIVE_INT, mspace_id, fspace_id, dxpl_id, data1_write, es_id);
    t1 = MPI_Wtime();
    while (MPI_Wtime() - t1 < comp) {}
    status = H5Dwrite_async(dset5_id, H5T_NATIVE_INT, mspace_id, fspace_id, dxpl_id, data1_write, es_id);


    H5ESwait(es_id, H5ES_WAIT_FOREVER, &n_running, &op_failed);
    t2 = MPI_Wtime();
    if (my_rank == 0)
        fprintf(stderr, "Ours: %f s\n", t2-t0);



    H5Pclose(async_fapl);
    H5Sclose(mspace_id);
    H5Dclose(dset0_id);
    H5Dclose(dset1_id);
    H5Dclose(dset2_id);
    H5Dclose(dset3_id);
    H5Dclose(dset4_id);
    H5Dclose(dset5_id);
    H5Gclose(grp_id);

done:
    /* H5Fwait(file_id); */
    H5Fclose(file_id);

    if (data0_write != NULL)
        free(data0_write);
    if (data0_read != NULL)
        free(data0_read);
    if (data1_write != NULL)
        free(data1_write);
    if (data1_read != NULL)
        free(data1_read);

    MPI_Finalize();
    return ret;
}
