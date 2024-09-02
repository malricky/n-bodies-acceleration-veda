#include<stdio.h>
#include<stdlib.h>
#include<veda.h>
#include<time.h>
#include<string.h>
#include<mpi.h>
#include<math.h>

#define NOW() ({ struct timespec ts; clock_gettime(CLOCK_MONOTONIC, &ts); ts; })
#define TIME(start, end) ((double)(end.tv_sec - start.tv_sec) * 1000.0 + (double)(end.tv_nsec - start.tv_nsec) / 1000000.0)

#define VEDA(err) check(err, __FILE__, __LINE__)

void check(VEDAresult err, const char* file, const int line) {
	if(err != VEDA_SUCCESS) {
		const char *name, *str;
		vedaGetErrorName	(err, &name);
		vedaGetErrorString	(err, &str);
		printf("%s: %s @ %s:%i\n", name, str, file, line);
		exit(1);
	}
}

void print2DMatrix(double*,int,int);

void fillParticlesPos(double *m,int n,int p){
    srand(time(NULL)+p);
    int max=1000000,min=100000;
    for(int i=0;i<n;i++){
        m[i] = (rand() % (max - min + 1)) + min;
        m[i+n] = ((double)rand()/RAND_MAX);
        m[i+n*2] = ((double)rand()/RAND_MAX);
    }
}

int main(int argc,char *argv[]){
        int t=atoi(argv[1]);
        int rank,t_proc,ierr;
        double *matrix_a;

        struct timespec start, end;
        start = NOW();    

        MPI_Init(&argc,&argv); 
        MPI_Comm_rank(MPI_COMM_WORLD,&rank);
        MPI_Comm_size(MPI_COMM_WORLD,&t_proc);

        MPI_Alloc_mem(t*3*sizeof(double), MPI_INFO_NULL, &matrix_a);

       if(rank == 0){
            fillParticlesPos(matrix_a,t,0);
       }

        ierr = MPI_Bcast(matrix_a, t*3, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        if (ierr != MPI_SUCCESS) {
        fprintf(stderr, "MPI_Bcast failed\n");
        MPI_Abort(MPI_COMM_WORLD, ierr);
    }

        VEDA(vedaInit(0)); 

        int p_size = t/t_proc;  
        
        int particles=p_size;

        size_t matrix_size = sizeof(double)*t*3;

        VEDAcontext ctx;
        VEDAdevice vd;
        VEDA(vedaDeviceGet(&vd,0));
        VEDA(vedaCtxCreate(&ctx,0,vd));
        VEDA(vedaCtxPushCurrent(ctx));

        VEDAmodule mod;
        VEDA(vedaModuleLoad(&mod,"./lib_device.vso"));

        VEDAfunction func;
        VEDA(vedaModuleGetFunction(&func,mod,"solve"));

        VEDAdeviceptr a;
        VEDA(vedaMemAllocAsync(&a,matrix_size,0));

        VEDA(vedaMemcpyHtoDAsync(a,matrix_a,matrix_size,0));

        VEDAargs args;
        VEDA(vedaArgsCreate(&args));
        VEDA(vedaArgsSetVPtr(args,0,a));
        VEDA(vedaArgsSetI64(args,1,particles));
        int pos = rank*particles;
        VEDA(vedaArgsSetI64(args,2,pos));
        VEDA(vedaArgsSetI64(args,3,t));

        VEDA(vedaLaunchKernelEx(func,0,args,1,0));

        VEDA(vedaCtxSynchronize());

        MPI_Barrier(MPI_COMM_WORLD);

        VEDA(vedaMemFreeAsync(a,0));

        MPI_Finalize();

        VEDA(vedaExit());

        end = NOW();
        double elapsed = TIME(start, end);
        

    return 0;
}

void print2DMatrix(double *matrix,int row,int col){

    for(int i=0;i<row;i++){
        for(int j=0;j<col;j++){
                printf(" %lf ",*(matrix+(i*col)+j));
        }
        printf("\n");
    }

}