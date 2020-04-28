#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mpi.h>
#include <time.h>
#include <string.h>

int cmp(const void *a, const void *b)
{
	return *(int *)a - *(int *)b;
}

int main(int argc, char** argv) {
	// 初始化 MPI 环境
	MPI_Init(&argc, &argv);

	int p;
	MPI_Comm_size(MPI_COMM_WORLD, &p);

	// 得到当前进程的秩
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
	int nb = rank == 1 ? np0 : np;

	int *data, *data_serial;
	int *a = (int *)malloc(sizeof(int) * na);
	int *t = (int *)malloc(sizeof(int) * na);
	int *b = (int *)malloc(sizeof(int) * nb);
	
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

	qsort(a, na, sizeof(int), cmp);
#ifdef DEBUG
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
	for (int i = 0; i < p; i++) {
#ifdef DEBUG
		if (rank == 0)
			printf("<===== stage %d =====>\n", i);
#endif
		int small;
		if (i%2 == 0) {
			small = rank%2 == 0;
		} else {
			small = rank%2 == 1;
		}
		int swap_rank = rank + (small ? 1 : -1);
		if (rank == 1)
			nb = small ? np : np0;
		if (swap_rank >= 0 && swap_rank < p) {
			MPI_Sendrecv(a, na, MPI_INT, swap_rank, 0, b, nb, MPI_INT, swap_rank, 0, MPI_COMM_WORLD, &state);
			memcpy(t, a, sizeof(int) * na);
			int k1 = 0;
			int k2 = 0;
			for (int j = 0; j < na; j++) {
				int ia, it, ib;
				if (small) {
					ia = j;
					it = k1;
					ib = k2;
				} else {
					ia = np - j -1;
					it = np - k1 - 1;
					ib = nb - k2 - 1;
				}
				if (rank == 0 && k2 == nb) {
					a[ia] = t[it];
				} else {
					if ((small && t[it] <= b[ib]) || (!small && t[it] >= b[ib])) {
						a[ia] = t[it];
						k1++;
					} else {
						a[ia] = b[ib];
						k2++;
					}
				}
			}
		}
#ifdef DEBUG
		for (int j = 0; j < p; j++) {
			if (rank == j) {
				printf("rank %d:", rank);
				for (int k = 0; k < na; k++) {
					printf(" %d", a[k]);
				}
				printf("\n");
				printf("swap %d:", swap_rank);
				for (int k = 0; k < nb; k++) {
					printf(" %d", b[k]);
				}
				printf("\n");
			}
			MPI_Barrier(MPI_COMM_WORLD);
		}
#endif
	}

	if (rank == 0) {
		for (int i = 1; i < p; i++) {
			MPI_Recv(data + na + np*(i-1), np, MPI_INT, i, 0, MPI_COMM_WORLD, &state);
		}
		end = MPI_Wtime();
		double t_parallel = end - start;
		// printf("%.4f ", end-start);
		start = MPI_Wtime();
		qsort(data_serial, n, sizeof(int), cmp);
		end = MPI_Wtime();
		double t_serial = end - start;
		printf("%lf\n", t_serial / t_parallel);
		// printf("%.4f\n", end-start);
		// int failed = 0;
		// for (int i = 0; i < n; i++) {
		// 	if (data_sorted[i] != data_serial[i]) {
		// 		failed = i;
		// 		break;
		// 	}
		// }
		// if (failed)
		// 	printf("failed %d\n", failed);
		// else
		// 	printf("correct\n");
	} else {
		MPI_Send(a, na, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}

	if (rank == 0) {
		free(data);
		free(data_serial);
	}
	free(a);
	free(b);
	free(t);
	// 释放 MPI 的一些资源
	MPI_Finalize();
}
