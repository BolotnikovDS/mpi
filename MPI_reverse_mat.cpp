// MPI_reverse_mat.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <mpi.h>
#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <math.h>
#include <ctime>
#include <cstdio>

#define eps 0.000000000001
double start;
double endf;

using namespace std;

void generation(double *A, int n, int formula);
void print_mat(double *A, int n);
void print_limit_mat(double *A, int n);
void print_vector(double *v, int n);
void mult_mat(double *A, double *B, double *C, int n);
void mult_mat_t(double *A, double *B, double *C, int n);
void mult_vector(double *Mat, double *vector, int n, double *ans);
void obrGauss(double *A, double *B, int n);
void turn(double *A, double *C, int n);
double norm(double* A, int n);
bool test();

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

void print_vector(double *v, int n)
{
	for (int j = 0; j < n; j++)
		cout << setw(8) << setprecision(2) << v[j] << " ";
	cout << endl;
}

void mult_mat(double *A, double *B, double *C, int n)
{
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
		{
			C[i*n + j] = 0;
			for (int k = 0; k < n; k++)
				C[i*n + j] += A[i*n + k] * B[k*n + j];
		}
}

void mult_mat_t(double *A, double *B, double *C, int n)
{
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
		{
			C[i*n + j] = 0;
			for (int k = 0; k < n; k++)
				C[i*n + j] += A[i*n + k] * B[j*n + k];
		}
}

void mult_vector(double *Mat, double *vector, int n, double *ans)
{
	for (int i = 0; i < n; i++)
	{
		ans[i] = 0;
		for (int j = 0; j < n; j++)
			ans[i] += Mat[i*n + j] * vector[j];
	}
}

void obrGauss(double *A, double *B, int n)
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

void turn(double *A, double *C, int n)
{
	//	cout << "Turn 1\n";
	double *B, *D;
	B = new double[n*n];
	D = new double[n*n];
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
		{
			B[i*n + j] = 0;
			D[i*n + j] = 0;
		}
	for (int i = 0; i < n; i++)
		B[i*(n + 1)] = 1;

	start = clock();
	for (int t = 0; t < n - 1; t++)
		for (int i = t + 1; i < n; i++)
		{
			if (fabs(A[i*n + t]) > eps)
			{
				double cs, sn, znam, bx, by;
				znam = sqrt(A[t*(n + 1)] * A[t*(n + 1)] + A[i*n + t] * A[i*n + t]);
				if (fabs(znam) < eps)
				{
					cout << "det = 0" << endl;
					return;
				}
				cs = A[t*(n + 1)] / znam;
				sn = -A[i*n + t] / znam;
				//for (int j = 0; j < n; j++)
				//{
				//bx = cs*B[t*n+j] - sn*B[i*n+j];
				//by = sn*B[t*n+j] + cs*B[i*n+j];
				//B[t*n+j] = bx;
				//B[i*n+j] = by;
				//}
				for (int j = 0; j < n; j++)
				{
					bx = cs*B[j*n + t] - sn*B[j*n + i];
					by = sn*B[j*n + t] + cs*B[j*n + i];
					B[j*n + t] = bx;
					B[j*n + i] = by;
				}
				for (int j = t; j < n; j++)
				{
					double x, y = 0;
					x = cs*A[t*n + j] - sn*A[i*n + j];
					if (j != t)
						y = sn*A[t*n + j] + cs*A[i*n + j];
					A[t*n + j] = x;
					A[i*n + j] = y;
				}
			}
		}
	//cout << "Turn\n";
	//print_limit_mat(A, n);
	//cout << endl;

	//cout << "obrTurn\n";
	//print_limit_mat(B, n);
	//cout << endl;


	obrGauss(A, D, n);
	//    cout << "obrGauss\n";
	//	print_limit_mat(D, n);
	//	cout << endl;

	//cout << "E\n";	
	//mult_mat(A, C, D, n);
	//print_limit_mat(D, n);
	mult_mat_t(D, B, C, n);
	endf = clock();
	free(B);
	free(D);
}

double norm(double* A, int n)
{
	double max = 0, sum = 0;
	for (int i = 0; i < n; i++)
	{
		max = 0;
		for (int j = 0; j < n; j++)
			if (fabs(max) < A[i*n + j])
				max = A[i*n + j];
		sum += max;
	}
	return sum;
}

bool test()
{
	double A[25], v[5], ans[5], t[5] = { 30, 20, 12, 8, 10 };
	int n = 5, k = 1;
	generation(A, n, k);
	for (int i = 0; i < n; i++)
		v[i] = i;
	mult_vector(A, v, n, ans);
	for (int i = 0; i < n; i++)
		if (fabs(t[i] - ans[i]) > eps)
			return false;
	return true;
}

int main(int argc, char **argv)
{
	MPI_Init(&argc, &argv);
	int rank, total_procs;
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &total_procs);
	FILE *stream;
	double *A, *ans;
	double *E, *B;
	int n, k = 2;
	cout << "Proccess #" << rank << endl;
	if (!test())
	{
		cout << "test has an error" << endl;
		return 0;
	}
	if (argc > 1)
		n = atoi(argv[argc - 1]);
	else
		return 0;
	if (argc > 2)
		freopen_s(&stream, argv[argc - 2], "r", stdin);
	A = new double[n*n];
	ans = new double[n*n];
	E = new double[n*n];
	B = new double[n*n];
	if (argc <= 2)
		generation(A, n, k);
	else
		for (int i = 0; i < n; i++)
			for (int j = 0; j < n; j++)
				cin >> A[i*n + j];
	for (int i = 0; i < n; i++)
		for (int j = 0; j < n; j++)
			B[i*n + j] = A[i*n + j];
	//print_limit_mat(A, n);
	cout << endl;
	turn(A, ans, n);
	print_limit_mat(B, n);
	cout << endl;
	print_limit_mat(ans, n);
	mult_mat(B, ans, E, n);
	for (int i = 0; i < n; i++)
		E[i*(n + 1)] -= 1;
	//	print_limit_mat(E, n);
	cout << norm(E, n) << endl;
	cout << double(endf - start) / CLOCKS_PER_SEC << " sec" << endl;
	free(A);
	free(ans);
	free(B);
	free(E);
	MPI_Finalize();
	return 0;
}