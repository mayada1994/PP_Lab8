#include "mpi.h"
#include <stdio.h>
#include <iostream>

using namespace std;

void createArray(long arr[], long size)
{
	for (int i = 0; i < size; i++)
	{
		arr[i] = i + 1;
	}
}

void printArray(long arr[], long size)
{
	cout << "[";
	for (long i = 0; i < size; i++)
	{
		cout << arr[i] << " ";
	}
	cout << "]" << endl;
}

int main(int argc, char *argv[])
{
	const long sizeOfArray = 10;
	long arr[sizeOfArray];
	long arr2[sizeOfArray];

	long tmpSum = 0;
	long mainSum = 0;

	int rank, size, nElementsReceived;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	MPI_Status status;

	if (rank == 0)
	{
		createArray(arr, sizeOfArray);

		printArray(arr, sizeOfArray);

		int index, i;
		int delta = sizeOfArray / size;

		for (i = 1; i < size - 1; i++)
		{
			index = i * delta;
			MPI_Send(&delta, 1, MPI_INTEGER, i, 0, MPI_COMM_WORLD);
			MPI_Send(&arr[index], delta, MPI_INTEGER, i, 0, MPI_COMM_WORLD);
		}

		index = i * delta;
		int elementsLeft = sizeOfArray - index;
		MPI_Send(&elementsLeft, 1, MPI_INTEGER, i, 0, MPI_COMM_WORLD);
		MPI_Send(&arr[index], elementsLeft, MPI_INTEGER, i, 0, MPI_COMM_WORLD);

		for (int i = 0; i < delta; i++)
		{
			tmpSum += arr[i];
		}
	}
	else
	{
		MPI_Recv(&nElementsReceived, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		MPI_Recv(&arr2, nElementsReceived, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
		for (int i = 0; i < nElementsReceived; i++)
		{
			tmpSum += arr2[i];
		}
		MPI_Send(&tmpSum, 1, MPI_INT, 0, 0, MPI_COMM_WORLD);
	}

	MPI_Reduce(&tmpSum, &mainSum, 1, MPI_INTEGER, MPI_SUM, 0, MPI_COMM_WORLD);

	if (rank == 0) {
		cout << "Total sum of elements: " << mainSum << endl;
	}

	MPI_Finalize();
	return 0;
}