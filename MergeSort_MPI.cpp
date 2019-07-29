#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

// Function to merge, with swapping in each level
void merge(int *a, int *b, int low, int mid, int high) {

	int h, i, j, k;
	h = low;
	i = low;
	j = mid + 1;

	// Compare the elements from first and second halves and place the one with the lowest value in the b array
	while ((h <= mid) && (j <= high)) {

		if (a[h] <= a[j]) {
			b[i] = a[h];
			h++;
		}
		else {
			b[i] = a[j];
			j++;
		}
		i++;
	}

	// if the elements in the first half are not sorted yet
	if (mid < h) {
		for (k = j; k <= high; k++) {
			b[i] = a[k];
			i++;
		}
	}
	// if the elements in the second half are not sorted yet
	else {
		for (k = h; k <= mid; k++) {
			b[i] = a[k];
			i++;
		}
	}

	//finall copy all the sorted value to the "a" array in sorted order
	for (k = low; k <= high; k++) {
		a[k] = b[k];
	}

}

// Recursive function for merge sort
void mergeSort(int *a, int *b, int low, int high) {
	
	// Break the array down until we get down to one element.
	if (low < high) {

		//calculate the mid with low and high
		int mid = (low + high) / 2;

		// Recursive call for first mid half
		mergeSort(a, b, low, mid);
		// Recursive call for second mid half
		mergeSort(a, b, (mid + 1), high);

		// Merge both the halves
		merge(a, b, low, mid, high);

	}

}

int main(int argc, char** argv) {
	
	// Initialize the global
	int globalArraySize = 1024;
	int *globalArray = (int*)malloc(globalArraySize * sizeof(int));

	// Insert random integer value into global array
	for (int i = 0; i < globalArraySize; i++) {
		globalArray[i] = rand();
	}

	// Initialize MPI
	int world_rank;
	int world_size;

	MPI_Init(&argc, &argv);
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);

	// Get the size of the local array which will be populated for processes with equal number of global array chumk
	int localArraySize = globalArraySize / world_size;
	int *localArray = (int*)malloc(localArraySize * sizeof(int));

	// Noting the start time
	double starttime, endtime;
	starttime = MPI_Wtime();

	// Send the local data to each processos, with "NODE 0 being the host" 

	/*
	MPI_Scatter Sends data from one process to all other processes in a communicator

	int MPI_Scatter(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
	void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm)

	Below are Input Parameters,
	sendbuf - address of send buffer (choice, significant only at root)
	sendcount - number of elements sent to each process (integer, significant only at root)
	sendtype - data type of send buffer elements (significant only at root) (handle)
	recvcount - number of elements in receive buffer (integer)
	recvtype - data type of receive buffer elements (handle)
	root - rank of sending process (integer)
	comm - communicator (handle)
	*/

	MPI_Scatter(globalArray, localArraySize, MPI_INT, localArray, localArraySize, MPI_INT, 0, MPI_COMM_WORLD);

	// Call merge sort to perform sorting operation
	int *tempArray = (int*)malloc(localArraySize * sizeof(int));
	mergeSort(localArray, tempArray, 0, (localArraySize - 1));

	// Collect the sorted local array using MPI_Gather

	/*
	MPI_Gather receives all values from a group of processes together

	int MPI_Gather(const void *sendbuf, int sendcount, MPI_Datatype sendtype,
	void *recvbuf, int recvcount, MPI_Datatype recvtype, int root, MPI_Comm comm)

	Below are Input Parameters in the order,
	sendbuf	- starting address of send buffer (choice)
	sendcount - number of elements in send buffer (integer)
	sendtype - data type of send buffer elements (handle)
	recvcount - number of elements for any single receive (integer, significant only at root)
	recvtype - data type of recv buffer elements (significant only at root) (handle)
	root - rank of receiving process (integer)
	comm - communicator (handle)
	*/

	int *sortedArray = NULL;

	if (world_rank == 0) {
		sortedArray = (int*)malloc(globalArraySize * sizeof(int));
	}

	MPI_Gather(localArray, localArraySize, MPI_INT, sortedArray, localArraySize, MPI_INT, 0, MPI_COMM_WORLD);

	// Perform the final merge sort on the gathered arrays, in the host node which is "NODE 0"
	if (world_rank == 0) {

		int *tempArray_1 = (int*)malloc(globalArraySize * sizeof(int));
		mergeSort(sortedArray, tempArray_1, 0, (globalArraySize - 1));

		// Print the sorted array
		printf("This is the sorted array: \n");
		for (int j = 0; j < globalArraySize; j++) {
			printf("%d\n", sortedArray[j]);
		}

		// Freeing the memory space used for pointers array
		free(sortedArray);
		free(tempArray_1);

		printf("\n\n\n");
		// Calculate and print the total time taken for sorting in MPI
		endtime = MPI_Wtime();
		printf("The total time taken to perfrom the merge sort using MPI is %f seconds\n", endtime - starttime);
	}

	// Freeing the memory space used for pointers array
	free(globalArray);
	free(localArray);
	free(tempArray);

	

	// Finalize MPI
	MPI_Barrier(MPI_COMM_WORLD);
	MPI_Finalize();
}
