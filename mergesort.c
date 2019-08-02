#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mpi.h"

// Declarations

void display(int array[], int x)
{
	int i;
	printf(">");
    printf("length: %d\n", x);
	for (i = 0; i < x; i++)
		printf(" %d", array[i]);
	printf("\n");
}

void merge(int *left, int llength, int *right, int rlength)
{
	//Temporary memory locations for the 2 segments of the array to merge.
	int *ltmp = (int *) malloc(llength * sizeof(int));
	int *rtmp = (int *) malloc(rlength * sizeof(int));

	
	 // Pointers to the elements being sorted in the temporary memory locations.
	int *ll = ltmp;
	int *rr = rtmp;

	int *result = left;

	
	// Copy the segment of the array to be merged into the temporary memory
	// locations.
	memcpy(ltmp, left, llength * sizeof(int));
	memcpy(rtmp, right, rlength * sizeof(int));

	while (llength > 0 && rlength > 0) {
		if (*ll <= *rr) {
			
			// Merge the first element from the left back into the main array
			// if it is smaller or equal to the one on the right.
			*result = *ll;
			++ll;
			--llength;
		} else {
			/*
			 * Merge the first element from the right back into the main array
			 * if it is smaller than the one on the left.
			 */
			*result = *rr;
			++rr;
			--rlength;
		}
		++result;
	}
	
	 // All the elements from either the left or the right temporary array
	 // segment have been merged back into the main array.  Append the remaining
	 // elements from the other temporary array back into the main array.
	if (llength > 0)
		while (llength > 0) {
			// Appending the rest of the left temporary array.
			*result = *ll;
			++result;
			++ll;
			--llength;
		}
	else
		while (rlength > 0) {
			// Appending the rest of the right temporary array.
			*result = *rr;
			++result;
			++rr;
			--rlength;
		}

	// Release the memory used for the temporary arrays.
	free(ltmp);
	free(rtmp);
}

void mergesort(int array[], int length)
{
	// This is the middle index and also the length of the right array.
	int middle;

	
	//  Pointers to the beginning of the left and right segment of the array
	//  to be merged. 
	int *left, *right;

	// Length of the left segment of the array to be merged.
	int llength;

	if (length <= 1)
		return;

	// Let integer division truncate the value.
	middle = length / 2;

	llength = length - middle;

	
	 //Set the pointers to the appropriate segments of the array to be merged.
	left = array;
	right = array + llength;

	mergesort(left, llength);
	mergesort(right, middle);
	merge(left, llength, right, middle);
}

int main(int argc, char *argv[])
{
	int *array = NULL;
	int length = 0;
	FILE *fh;
	int data;
    int *sub;
    int *send;
    int *disp;
    int *done;
    int numranks, rank;
    double start, end, time;

	if (argc != 2) {
		printf("usage: %s <filename>\n", argv[0]);
		return 1;
	}

	// Initialize data.
	printf("attempting to sort file: %s\n", argv[1]);

	fh = fopen(argv[1], "r");
	if (fh == NULL) {
		printf("error opening file\n");
		return 0;
	}

    MPI_Init(&argc,&argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numranks);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
 
    //Stuff for File
    while (fscanf(fh, "%d", &data) != EOF) {
	    ++length;
	    array = (int *) realloc(array, length * sizeof(int));
        if(rank == 0){
	        array[length - 1] = data;
        }
	}
    printf("%d elements read\n", length);
    if(rank == 0){
       // display(array, length);
    }
	fclose(fh);
    done = (int *) malloc(length*sizeof(int));

    int size = length/numranks;
    int remainder = length%numranks;
    send = (int *)malloc(numranks * sizeof(int));
    disp = (int *)malloc(numranks*sizeof(int));
    int total = 0;
    for(int i = 0; i <numranks; i++){
        send[i] = size;

        if(remainder > 0){
            send[i] = send[i] + 1;
            remainder--;
        }
        disp[i] = total;
        total += send[i];
    }

    sub = (int *)malloc(send[rank]*sizeof(int));

    start = MPI_Wtime();
    MPI_Scatterv(array, send, disp, MPI_INT, sub, send[rank], MPI_INT, 0, MPI_COMM_WORLD);
    
	mergesort(sub, send[rank]);
    if(rank%2 == 1){
        //Send to rank to the left
    }

    MPI_Gatherv(sub,send[rank],MPI_INT,done,send,disp,MPI_INT,0,MPI_COMM_WORLD);
    if(rank == 0){
        if(numranks>1){
            printf("here");
	   mergesort(done,length);
    }
       end = MPI_Wtime();
       time = end - start;
       printf("Total time: %f\n", time);

	   //display(done, length);
       printf("Smallest: %d Largest: %d",done[0],done[length-1]);
   }
    MPI_Finalize();
	return 0;
}
