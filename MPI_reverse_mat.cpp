// MPI_reverse_mat.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <math.h>
#include <ctime>
#include <cstdio>
#include <mpi.h>
#include <algorithm>

#define eps 1e-12

using std::endl;
using std::cout;
using std::setw;
using std::setprecision;

void generation(double *A, int n, int formula);
void print_mat(double *A, int n);
void print_limit_mat(double *A, int n);
void transpose(double *A, int n);
void reverseGauss(double *A, double *B, int n);
void reverse(double *A, double *B, int n, int n_new, int size, int rank);
double norm_mult(double* A, double* Ans, int n, int n_new, int size, int rank);

void generation(double *A, int n, int formula)
{
	if (formula == 1)
	{
		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++)
				A[n*i + j] = fabs(i - j);
	}
	if (formula == 2)
	{
		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++)
				A[n*i + j] = 1 / double(i + j + 1);
	}
	if (formula == 3)
	{
		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++)
				A[i*n + j] = 1;
		//memset(A, 1, n*n*sizeof(double));
		for (int i = 0; i < n; i++)
			A[(i + 1)*(n - 1)] = -1;
	}
}

void transpose(double *A, int n)
{
	for (int i = 0; i < n; i++)
		for (int j = i + 1; j < n; j++)
		{
			double x = A[i * n + j];
			A[i * n + j] = A[j * n + i];
			A[j * n + i] = x;
		}
}

void print_mat(double *A, int n)
{
	for (int i = 0; i < n; i++)
	{
		for (int j = 0; j < n; j++)
			cout << setw(8) << setprecision(3) << A[i*n + j] << " ";
		cout << endl;
	}
}

void print_limit_mat(double *A, int n)
{
	if (n < 7)
	{
		print_mat(A, n);
		return;
	}
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
			cout << setw(8) << setprecision(2) << A[i*n + j] << " ";
		cout << " ... " << A[(i + 1)*n - 1] << endl;
	}
	for (int i = 0; i < 7; i++)
		cout << setw(8) << "...";
	cout << endl;
	for (int j = 0; j < 5; j++)
		cout << setw(8) << setprecision(2) << A[(n - 1)*n + j] << " ";
	cout << " ... " << A[(n)*n - 1] << endl;
}

void reverseGauss(double *A, double *B, int n)
{
	for (int i = n - 1; i >= 0; i--)
	{
		if (fabs(A[(n + 1)*i]) < eps)
		{
			cout << "zero at diag" << endl;
			return;
		}
		B[(n + 1)*i] = 1 / A[(n + 1)*i];
		for (int t = i + 1; t < n; t++)
			B[n*i + t] /= A[(n + 1)*i];
		for (int j = i - 1; j >= 0; j--)
		{
			for (int t = i; t < n; t++)
				B[j*n + t] -= B[i*n + t] * A[j*n + i];
			//			print_limit_mat(B, n);
			//			cout << endl;
		}
	}
}

void reverse(double *A, double *Ans, int n, int n_new, int size, int rank)
{
	double *Q = NULL, *A_part = NULL, *Ans_part = NULL, *Q_part = NULL;
	double *cs, *sn;
	Q = new double[n * n_new];
	Ans_part = new double[n * n_new / size];
	A_part = new double[n * n_new / size];
	Q_part = new double[n * n_new / size];
	cs = new double[n - 1];
	sn = new double[n - 1];

	int s = n_new / size;

	memset(Q_part, 0, n * s * sizeof(double));
	for (int i = 0; i < s; i++)
		Q_part[i+rank*s + i*n] = 1;
	
	double t1, t2, dt, ta, tb, tsum = 0;

	t1 = MPI_Wtime();

	ta = MPI_Wtime();

	MPI_Scatter(A, n*s, MPI_DOUBLE, A_part, n*s, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	tb = MPI_Wtime();
	tsum += tb - ta;

	for (int k = 0; k < n - 1; k++) {
		int root = k / s;

		if (rank == root) {
			for (int l = k + 1; l < n; l++) {

				double x = A_part[k + (k%s)*n];
				double y = A_part[l + (k%s)*n];
				double denom = sqrt(x * x + y * y);
				if (fabs(x) < eps && fabs(y) < eps)
					continue;
				cs[l - k - 1] = x / denom;
				sn[l - k - 1] = -y / denom;

				A_part[k + (k%s)*n] = denom;
				A_part[l + (k%s)*n] = 0;
			}
		}

		MPI_Bcast(cs, n - 1, MPI_DOUBLE, root, MPI_COMM_WORLD);
		MPI_Bcast(sn, n - 1, MPI_DOUBLE, root, MPI_COMM_WORLD);

		for (int m = 0; m < s; m++) {
			for (int l = k + 1; l < n; l++) {
				double temp_k, temp_l;
				if (m + s * rank > k) {
					temp_k = cs[l - k - 1] * A_part[k + m*n] - sn[l - k - 1] * A_part[l + m*n];
					temp_l = sn[l - k - 1] * A_part[k + m*n] + cs[l - k - 1] * A_part[l + m*n];
					A_part[k + m*n] = temp_k;
					A_part[l + m*n] = temp_l;
				}
				temp_k = cs[l - k - 1] * Q_part[k + m*n] - sn[l - k - 1] * Q_part[l + m*n];
				temp_l = sn[l - k - 1] * Q_part[k + m*n] + cs[l - k - 1] * Q_part[l + m*n];
				Q_part[k + m*n] = temp_k;
				Q_part[l + m*n] = temp_l;
			}
		}
	}

	t2 = MPI_Wtime();

	if (rank == 0)
	{
		dt = t2 - t1;
		cout << "Rotation: " << dt << " sec" << endl;
	}

	ta = MPI_Wtime();

	MPI_Allgather(A_part, n*s, MPI_DOUBLE, A, n*s, MPI_DOUBLE, MPI_COMM_WORLD);
	MPI_Allgather(Q_part, n*s, MPI_DOUBLE, Q, n*s, MPI_DOUBLE, MPI_COMM_WORLD);
//	MPI_Gather(A_part, n*s, MPI_DOUBLE, A, n*s, MPI_DOUBLE, 0, MPI_COMM_WORLD);
//	MPI_Gather(Q_part, n*s, MPI_DOUBLE, Q, n*s, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	tb = MPI_Wtime();
	tsum += tb - ta;

	if (rank == 0) {
		cout << "R" << endl;
		print_limit_mat(A, n);
		cout << endl << "Q" << endl;
		print_limit_mat(Q, n);
	}

	t1 = MPI_Wtime();

	for (int j = 0; j < s && j + s * rank < n; j++)
		for (int i = n - 1; i >= 0; i--) {
			Ans_part[i + j*n] = Q[i + (j+s*rank)*n];
			for (int k = i + 1; k < n; k++)
				Ans_part[i + j*n] -= A[i + k*n] * Ans_part[k + j*n];
			Ans_part[i + j*n] /= A[i+i*n];
		}

	t2 = MPI_Wtime();
	if (rank == 0)
		cout << "Reverse gauss: " << t2 - t1 << " sec" << endl;

	ta = MPI_Wtime();

	MPI_Gather(Ans_part, n * s, MPI_DOUBLE, Ans, n * s, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	tb = MPI_Wtime();
	tsum += tb - ta;

	if (rank == 0)
		cout << "Mesages time: " << tsum << endl;
	delete[] Q;
	delete[] Q_part;
	delete[] A_part;
	delete[] Ans_part;
	delete[] sn;
	delete[] cs;
}

double norm_mult(double* A, double* Ans, int n, int n_new, int size, int rank)
{

	int s = n_new / size;

	MPI_Bcast(Ans, n * n, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	double *A_part;
	A_part = new double[n * s];

	double *Ans_part;
	Ans_part = new double[n * s];

	double *res_matrix = NULL;

	if (rank == 0)
	{
		res_matrix = new double[n_new * n];
		memset(res_matrix, 0, n_new * n * sizeof(double));
	}

	MPI_Scatter(A, n * s, MPI_DOUBLE, A_part, n * s, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	transpose(Ans, n);

	for (int j = 0; j < s; j++)
		for (int i = 0; i < n; i++) {
			Ans_part[i + j * n] = 0;
			for (int k = 0; k < n; k++)
				Ans_part[i + j * n] += A[k + i * n] * Ans[k + (j + s * rank) * n];
		}

	MPI_Gather(Ans_part, n * s, MPI_DOUBLE, res_matrix, n * s, MPI_DOUBLE, 0, MPI_COMM_WORLD);

	if (rank == 0)
	{
		for (int i = 0; i < n; i++)
			res_matrix[i * n + i] -= 1;
		double res = 0;
		for (int i = 0; i < n; i++) {
			double preRes = 0;
			for (int j = 0; j < n; j++)
				preRes += fabs(res_matrix[i * n + j]);
			res = std::max(res, preRes);
		}
		delete[] res_matrix;
		delete[] A_part;
		delete[] Ans_part;
		return res;
	}

	delete[] A_part;
	delete[] Ans_part;

	return 0;
}

int main(int argc, char** argv)
{
	char *t;
	int rank, size;
	int n = 1000, n_new = 1000, formula = 1;
	n = strtol(argv[1], &t, 10);
	double *A = NULL, *Ans = NULL;
	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);
	MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);

	if (rank == 0)
		cout << size << endl;
	//generation
	if (n % size)
		n_new = n + size - n % size;
	else
		n_new = n;
	A = new double[n * n_new];
	Ans = new double[n * n_new];
	generation(A, n, formula);
	if (rank == 0) {
		cout << "A" << endl;
		print_limit_mat(A, n);
	}

	//timer
	double t1, t2, dt;
	t1 = MPI_Wtime();
	reverse(A, Ans, n, n_new, size, rank);
	if (rank == 0) {
		cout << "Ans" << endl;
		print_limit_mat(Ans, n);
	}
	t2 = MPI_Wtime();
	dt = t2 - t1;
	if (rank == 0)
		cout << "Time: " << dt << " seconds" << endl;
	generation(A, n, formula);
	double r = norm_mult(A, Ans, n, n_new, size, rank);
	if (rank == 0)
		cout << r << endl;
	delete[] A;
	delete[] Ans;
	MPI_Finalize();
	return 0;
}
