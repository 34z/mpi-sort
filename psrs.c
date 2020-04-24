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
	int *a = (int *)malloc(sizeof(int) * na);
	
	double start, end;
	MPI_Barrier(MPI_COMM_WORLD);

	if (rank == 0) {
		data = (int *)malloc(sizeof(int) * n);
		data_serial = (int *)malloc(sizeof(int) * n);
		srand(time(0));
		for (int i = 0; i < n; i++) {
			data[i] = rand() % (n*2);
			data_serial[i] = data[i];
		}
		start = MPI_Wtime();
		memcpy(a, data, sizeof(int) * na);
		for (int i = 1; i < p; i++) {
			MPI_Send(data + na + np*(i-1), np, MPI_INT, i, 0, MPI_COMM_WORLD);
		}
	} else {
		MPI_Recv(a, na, MPI_INT, 0, 0, MPI_COMM_WORLD, &state);
	}
	MPI_Barrier(MPI_COMM_WORLD);

	qsort(a, na, sizeof(int), cmp);

	int *samples;
	int *sample = (int *)malloc(sizeof(int) * p);
	for (int i = 0; i < p; i++) {
		sample[i] = a[(na/p)*i];
	}

	if (rank == 0) {
		samples = (int *)malloc(sizeof(int) * p * p);
		memcpy(samples, sample, sizeof(int) * p);
		for (int i = 1; i < p; i++) {
			MPI_Recv(samples+p*i, p, MPI_INT, i, 0, MPI_COMM_WORLD, &state);
		}
	} else {
		MPI_Send(sample, p, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}
#ifdef DEBUG
	if (rank == 0) {
		printf("samples:");
		for (int k = 0; k < p*p; k++) {
			printf(" %d", samples[k]);
		}
		printf("\n");
	}

	for (int j = 0; j < p; j++) {
		if (rank == j) {
			printf("rank %d:", rank);
			for (int k = 0; k < na; k++) {
				printf(" %d", a[k]);
			}
			printf("\n");
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}
#endif
	if (rank == 0) {
		qsort(samples, p * p, sizeof(int), cmp);
#ifdef DEBUG
		printf("samples:");
		for (int k = 0; k < p*p; k++) {
			printf(" %d", samples[k]);
		}
		printf("\n");
		for (int i = 0; i < p-1; i++) {
			samples[i] = samples[p*(i+1)];
		}
		printf("samples:");
		for (int k = 0; k < p-1; k++) {
			printf(" %d", samples[k]);
		}
		printf("\n");
#endif
		memcpy(sample, samples, sizeof(int) * (p-1));
		for (int i = 1; i < p; i++) {
			MPI_Send(samples, p-1, MPI_INT, i, 0, MPI_COMM_WORLD);
		}
	} else {
		MPI_Recv(sample, p-1, MPI_INT, 0, 0, MPI_COMM_WORLD, &state);
	}

	int *that_segs_size = (int *)malloc(sizeof(int) * p);
	int *segs_size = (int *)malloc(sizeof(int) * p);
	int *segs_start = (int *)malloc(sizeof(int) * p);
	int j = 0;
	for (int i = 0; i < p; i++) {
		if (i < p-1) {
			segs_size[i] = 0;
			while (j < na && a[j] < sample[i]) {
				j++;
				segs_size[i]++;
			}
			segs_start[i] = j - segs_size[i];
		} else {
			segs_size[i] = na - j;
			segs_start[i] = j;
		}
		if (rank == i) {
			that_segs_size[i] = segs_size[i];
		} else {
			MPI_Sendrecv(segs_size+i, 1, MPI_INT, i, 0,
				that_segs_size+i, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &state);
		}
	}

	int **segs = (int **)malloc(sizeof(int *) * p);
	for (int i = 0; i < p; i++) {
		segs[i] = (int *)malloc(sizeof(int) * that_segs_size[i]);
		if (rank == i) {
			memcpy(segs[i], a+segs_start[i], sizeof(int) * segs_size[i]);
		} else {
			MPI_Sendrecv(a+segs_start[i], segs_size[i], MPI_INT, i, 0,
				segs[i], that_segs_size[i], MPI_INT, i, 0, MPI_COMM_WORLD, &state);
		}
	}
#ifdef DEBUG
	if (rank == 0) printf("=====\n");
	for (int i = 0; i < p; i++) {
		if (rank == i) {
			printf("rank %d:", rank);
			for (int j = 0; j < p; j++) {
				printf(" %d", that_segs_size[j]);
			}
			printf("\n");
			for (int j = 0; j < p; j++) {
				for (int k = 0; k < that_segs_size[j]; k++) {
					printf("%d ", segs[j][k]);
				}
				printf("\n");
			}
		}
		MPI_Barrier(MPI_COMM_WORLD);
	}
#endif
	int *seg;
	int seg_size;
	merge_segs(segs, that_segs_size, p, &seg, &seg_size);
	if (rank == 0) {
		memcpy(data, seg, sizeof(int) * seg_size);
		int idx = seg_size;
		for (int i = 1; i < p; i++) {
			int that_seg_size;
			MPI_Recv(&that_seg_size, 1, MPI_INT, i, 0, MPI_COMM_WORLD, &state);
			MPI_Recv(data+idx, that_seg_size, MPI_INT, i, 0, MPI_COMM_WORLD, &state);
			idx += that_seg_size;
		}
		end = MPI_Wtime();
		printf("parallel time %.4f\n", end-start);
		start = MPI_Wtime();
		qsort(data_serial, n, sizeof(int), cmp);
		end = MPI_Wtime();
		printf("serial time %.4f\n", end-start);
		int failed = 0;
		for (int i = 0; i < n; i++) {
			if (data[i] != data_serial[i]) {
				failed = i;
				break;
			}
		}
		if (failed)
			printf("failed %d\n", failed);
		else
			printf("correct\n");
	} else {
		MPI_Send(&seg_size, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
		MPI_Send(seg, seg_size, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}

	if (rank == 0) {
		free(data);
		free(samples);
	}
	free(a);
	free(sample);
	for (int i = 0; i < p; i++) {
		free(segs[i]);
	}
	free(segs);
	free(seg);

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

	*seg_size = size;
	*seg = a;
}
