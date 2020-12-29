#include <iostream>
#include "mpi.h"

using namespace std;

struct request {
	int index;
	int val1;
	int val2;
};

struct result {
	int rank;
	int index;
	int sum;
};

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

int main(int argc, char* argv[]) {

	int rank, size;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &rank);
	MPI_Comm_size(MPI_COMM_WORLD, &size);

	if (size < 2) {
		cout << "Need more proccessors" << endl;
		return -1;
	}

	int requestBlocksLengths[3] = { 1, 1, 1 };
	MPI_Datatype requestBlocksTypes[3] = { MPI_INT, MPI_INT, MPI_INT };
	MPI_Datatype MPI_REQUEST_TYPE;
	MPI_Aint requestsOffsets[3] = {
		offsetof(request, index),
		offsetof(request, val1),
		offsetof(request, val2)
	};

	MPI_Type_create_struct(3, requestBlocksLengths, requestsOffsets, requestBlocksTypes, &MPI_REQUEST_TYPE);
	MPI_Type_commit(&MPI_REQUEST_TYPE);

	int resultBlocksLengths[3] = { 1, 1, 1 };
	MPI_Datatype resultBlocksTypes[3] = { MPI_INT, MPI_INT, MPI_INT };
	MPI_Datatype MPI_RESULT_TYPE;
	MPI_Aint resultsOffsets[3] = {
		offsetof(result, rank),
		offsetof(result, index),
		offsetof(result, sum)
	};
	MPI_Type_create_struct(3, resultBlocksLengths, resultsOffsets, resultBlocksTypes, &MPI_RESULT_TYPE);
	MPI_Type_commit(&MPI_RESULT_TYPE);

	if (rank == 0) {
		const long sizeOfArray = 10;
		long arr[sizeOfArray];

		createArray(arr, sizeOfArray);
		printArray(arr, sizeOfArray);

		int currentSize = sizeOfArray;
		int receiver = 0;
		int receiversAvailable = size - 1;

		while (currentSize > 1) {
			for (int i = 0; i < currentSize / 2; i++) {
				request rq;
				rq.index = i;
				rq.val1 = arr[i];
				rq.val2 = arr[currentSize - i - 1];
				if (receiversAvailable > 0) {
					int receiverRank = 1 + (receiver++) % (size - 1);
					MPI_Send(&rq, 1, MPI_REQUEST_TYPE, receiverRank, 0, MPI_COMM_WORLD);
					receiversAvailable--;
				}
				else {
					result rs;
					MPI_Recv(&rs, 1, MPI_RESULT_TYPE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
					arr[rs.index] = rs.sum;
					MPI_Send(&rq, 1, MPI_REQUEST_TYPE, rs.rank, 0, MPI_COMM_WORLD);
				}
			}
			while (receiversAvailable < (size - 1)) {
				result rs;
				MPI_Recv(&rs, 1, MPI_RESULT_TYPE, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
				arr[rs.index] = rs.sum;
				receiversAvailable++;
			}
			currentSize = currentSize / 2 + currentSize % 2;
		}
		for (int i = 0; i < (size - 1); i++) {
			request rq;
			rq.index = -1;
			int receiverRank = i + 1;
			MPI_Send(&rq, 1, MPI_REQUEST_TYPE, receiverRank, 0, MPI_COMM_WORLD);
		}

		cout << "Total sum of elements: " << arr[0] << endl;
	}
	else {
		while (true) {
			request rq;
			MPI_Recv(&rq, 1, MPI_REQUEST_TYPE, 0, 0, MPI_COMM_WORLD, MPI_STATUSES_IGNORE);
			if (rq.index < 0) {
				break;
			}
			result rs;
			rs.rank = rank;
			rs.index = rq.index;
			rs.sum = rq.val1 + rq.val2;
			MPI_Send(&rs, 1, MPI_RESULT_TYPE, 0, 0, MPI_COMM_WORLD);
		}
	}

	MPI_Finalize();
	return 0;
}