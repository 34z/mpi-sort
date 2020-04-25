#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <string.h>
#include <time.h>

int cmp(const void *a, const void *b)
{
	return *(int *)a - *(int *)b;
}

void merge_segs(int **segs, int *segs_size, int p, int **seg, int *seg_size);

int main(int argc, char *argv[])
{
	MPI_Init(&argc, &argv);

	int p;
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	int rank;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);

	MPI_Status state;

	int n = 10000000;
	if (argc == 2) {
		n = atoi(argv[1]);
	}
	int np = n / p;
	int np0 = n - np*(p-1);
	int na = rank == 0 ? np0 : np;

	int *data, *data_serial;
	int **data_p;
	int *data_size;
	int *a = (int *)malloc(sizeof(int) * na);
	
	double start, end;
	MPI_Barrier(MPI_COMM_WORLD);

	if (rank == 0) {
		data = (int *)malloc(sizeof(int) * n);
		data_serial = (int *)malloc(sizeof(int) * n);
		data_p = (int **)malloc(sizeof(int *) * p);
		data_size = (int *)malloc(sizeof(int) * p);
		srand(time(0));
		for (int i = 0; i < n; i++) {
			data[i] = rand() % (n*2);
			data_serial[i] = data[i];
		}
		start = MPI_Wtime();
		memcpy(a, data, sizeof(int) * na);
		data_p[0] = data;
		data_size[0] = na;
		for (int i = 1; i < p; i++) {
			data_p[i] = data + na + np*(i-1);
			data_size[i] = np;
			MPI_Send(data_p[i], data_size[i], MPI_INT, i, 0, MPI_COMM_WORLD);
		}
	} else {
		MPI_Recv(a, na, MPI_INT, 0, 0, MPI_COMM_WORLD, &state);
	}
	MPI_Barrier(MPI_COMM_WORLD);

	qsort(a, na, sizeof(int), cmp);

	if (rank == 0) {
		for (int i = 1; i < p; i++) {
			MPI_Recv(data_p[i], data_size[i], MPI_INT, i, 0, MPI_COMM_WORLD, &state);
		}
	} else {
		MPI_Send(a, na, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}

	int *data_sorted;
	int seg_size;
	if (rank == 0) {
		merge_segs(data_p, data_size, p, &data_sorted, NULL);
		end = MPI_Wtime();
		printf("parallel time %.4f\n", end-start);
		start = MPI_Wtime();
		qsort(data_serial, n, sizeof(int), cmp);
		end = MPI_Wtime();
		printf("serial time %.4f\n", end-start);
		int failed = 0;
		for (int i = 0; i < n; i++) {
			if (data_sorted[i] != data_serial[i]) {
				failed = i;
				break;
			}
		}
		if (failed)
			printf("failed %d\n", failed);
		else
			printf("correct\n");
	}

	if (rank == 0) {
		free(data);
		free(data_serial);
		free(data_p);
		free(data_size);
		free(data_sorted);
	}
	free(a);

	MPI_Finalize();
	return 0;
}

void merge_segs(int **segs, int *segs_size, int p, int **seg, int *seg_size)
{
	int size;
	size = 0;
	for (int i = 0; i < p; i++) {
		size += segs_size[i];
	}
	int *a = (int *)malloc(sizeof(int) * size);

	int *idx = (int *)malloc(sizeof(int) * p);
	memset(idx, 0, sizeof(int) * p);

	int j = 0;
	 while (1) {
		int min;
		int min_idx = -1;
		for (int i = 0; i < p; i++) {
			if (idx[i] < segs_size[i]) {
				if (min_idx == -1 || segs[i][idx[i]] < min) {
					min = segs[i][idx[i]];
					min_idx = i;
				}
			}
		}
		if (min_idx == -1) break;
		a[j++] = min;
		idx[min_idx]++;
	};
	if (seg_size)
		*seg_size = size;
	if (seg)
		*seg = a;
}
