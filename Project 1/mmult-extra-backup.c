#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include <pthread.h>


void* multiply_thread(void* );


typedef struct matrix_info{
	float* matrix_a;
	float* matrix_b;
	int row_a;
	int col_a;
	int row_b;
	int col_b;
} MatrixInfo;

typedef struct arg_struct{
	int firstRLine;
	int lastRLine;
	int firstCLine;
	int lastCLine;
	MatrixInfo * matInfo;
} ArgStruct;

typedef struct result_struct{
	float* r_matrix;
	int firstRLine;
	int lastRLine;
	int firstCLine;
	int lastCLine;
} ResultStruct;

int main(int argc, char** argv){
	if(argc != 7){
		printf("Correct usage is: my_matrix_multiply -a a_matrix_file.txt -b b_matrix_file.txt -t thread_count\n");
		return 0;
	}
	if(argv[1][0] != '-' || argv[3][0] != '-' || argv[5][0] != '-'){
		printf("Correct usage is: -something\n");
		return 0;
	} 
	if(argv[1][2] != '\0' || argv[3][2] != '\0' || argv[5][2] != '\0'){
		printf("Incorrect usage\n");
		return 0;
	}
	int i, num_threads;
	FILE* a_matrix_file = NULL;
	FILE* b_matrix_file = NULL;
	char buff1[300];
	float* matrix_a;
	float* matrix_b;
	int row_a, col_a, row_b, col_b, a_size, b_size, k;

	// error codes
	int err;
	//int j; 

	for(i=1;i< 7; i+=2){
		switch(argv[i][1]){
			case 'a':
				if(a_matrix_file != NULL){
					printf("Error: Matrix A has already been supplied\n");
					return 0;
				}
				a_matrix_file = fopen(argv[i+1],"r");
				if(a_matrix_file != NULL){
					// first pull line of text into buffer
					if(fgets(buff1, 300, a_matrix_file) != NULL){
						// for first line, separate into two tokens
						char* token = strtok(buff1, " ");
						// for each token, put into row and column for this matrix
						row_a = atoi(token);
						token = strtok(NULL, " ");
						col_a = atoi(token);
						// error checking
						if(row_a < 1 || col_a < 1){
							printf("Error: Matrix dimensions should be natural numbers!\n");
							return 0;
						}
						// determine size of matrix 
						a_size = row_a*col_a;
						// allocate the matrix
						matrix_a = (float*)malloc(a_size*sizeof(float));
						// initialize matrix index counter
						k=0;
						// for each item in the matrix: read in a line, turn into a float
						while(fgets(buff1, 300, a_matrix_file) != NULL){
							if(buff1[0] != '#'){
								matrix_a[k] = atof(buff1 );
								k++;
							}	
						}
						if(k != a_size){
							printf("Error: Too few entries in Matrix A!\n");
							return 0;
						}
						if(feof(a_matrix_file) == 0){
							printf("Error: End of file -a not reached!\n");
							return 0;
						}
					}
					else{
						printf("Error: Empty file!\n");
						return 0;
					}
				}
				else{
					printf("Error: file for Matrix A cannot be opened\n");
					return 0;
				}
				fclose(a_matrix_file);
				break;
			case 'b':
			if(b_matrix_file != NULL){
					printf("Error: Matrix B has already been supplied\n");
					return 0;
				}
				b_matrix_file = fopen(argv[i+1],"r");
				if(b_matrix_file != NULL){
					// first pull line of text into buffer
					if(fgets(buff1, 300, b_matrix_file) != NULL){
						// for first line, separate into two tokens
						char* token = strtok(buff1, " ");
						// for each token, put into row and column for this matrix
						row_b = atoi(token);
						token = strtok(NULL, " ");
						col_b = atoi(token);
						// error checking
						if(row_b < 1 || col_b < 1){
							printf("Error: Matrix dimensions should be natural numbers!\n");
							return 0;
						}
						// determine size of matrix 
						b_size = row_b*col_b;
						// allocate the matrix
						matrix_b = (float*)malloc(b_size*sizeof(float));
						// initialize matrix index counter
						k=0;
						// for each item in the matrix: read in a line, turn into a float
						while(fgets(buff1, 300, b_matrix_file) != NULL){
							if(buff1[0] != '#'){
								matrix_b[k] = atof(buff1);
								k++;
							}	
						}
						if(k != b_size){
							printf("Error: Too few entries in Matrix B!\n");
							return 0;
						}
						if(feof(b_matrix_file) == 0){
							printf("Error: End of file -b not reached!\n");
							return 0;
						}
					}
					else{
						printf("Error: Empty file!\n");
						return 0;
					}
				}
				else{
					printf("Error: file for Matrix B cannot be opened\n");
					return 0;
				}
				fclose(b_matrix_file);
				break;
			case 't':
				num_threads = atoi(argv[i+1]);
				if(num_threads <= 0){
					printf("Error: Invalid thread count\n");
					return 0;
				}
				break;
			default:
				printf("unrecognized command: %s", argv[i]);
				return 0;
		}
	}

	// do more matrix multiplication type checking
	if(col_a != row_b){
		printf("Error: Matrix dimensions do not match!");
		return 0;
	}

	MatrixInfo* matrixInfo = (MatrixInfo*)malloc(sizeof(MatrixInfo));
	matrixInfo->matrix_a = matrix_a;
	matrixInfo->matrix_b = matrix_b;
	matrixInfo->row_a = row_a;
	matrixInfo->row_b = row_b;
	matrixInfo->col_a = col_a;
	matrixInfo->col_b = col_b;


	// make the place to store all of the thread ids
	pthread_t* threadIDs  =(pthread_t*) malloc(num_threads * sizeof(pthread_t));


	// calculate the thread division
	
	int numColSets = (int) ceil(sqrt((double) num_threads)*((double)col_b/((double)row_a)));
	if(numColSets > num_threads)
		numColSets = num_threads;
	
	int threadsLeft = num_threads;
	int colSetThreads, linesPerColSet, colSetNum;
	int colFirstThread = 0; // the first thread for the column
	int leftCol, rightCol, nextRowLine;
	int nextColLine = 0;
	for(colSetNum = 0; colSetNum < numColSets; colSetNum++){

		// calculate the thread division
		colSetThreads = threadsLeft/(numColSets - colSetNum);
		linesPerColSet = ((col_b - nextColLine)*colSetThreads)/threadsLeft;
		threadsLeft -= colSetThreads;
		nextRowLine = 0;	
		
		leftCol = nextColLine;
		rightCol = nextColLine + linesPerColSet - 1;
		// pack the data for the threads
		for(i = colFirstThread; i < colFirstThread + colSetThreads; i++){

			ArgStruct* threadArg = (ArgStruct*)malloc(sizeof(ArgStruct));
			threadArg->matInfo = matrixInfo;
			threadArg->firstCLine = leftCol;
			threadArg->lastCLine = rightCol;
			threadArg->firstRLine = nextRowLine;
			threadArg->lastRLine = nextRowLine + (row_a - nextRowLine - 1)/(colSetThreads-(i - colFirstThread));
			//printf("the last rLine for thread %d is %d of %d \n", i, threadArg->lastRLine, row_a);
			nextRowLine = threadArg->lastRLine +1;

			// create the threads
			err = pthread_create(threadIDs+i,NULL,multiply_thread,(void*)threadArg);
			//printf("thread %d created \n", i);
			if(err){
				printf("The thread wasn't successfully created");
				return 0;
			}
		}
		nextColLine += linesPerColSet;
		colFirstThread += colSetThreads;
	}
	// get data back from the threads

	ResultStruct* threadResults; 
	float* resultMatrix = malloc(sizeof(float)*row_a*col_b);
	printf("ResultMatrix -1 value is: %x\n", resultMatrix[-1]);
	fflush(stdout);
	float* threadResultMatrix;
	int resultFirstRLine,resultLastRLine, resultFirstCLine, resultLastCLine;
	int threadID;
	int resultRow, resultColumn;
	for(threadID = 0; threadID < num_threads; threadID++){
		err = pthread_join(threadIDs[threadID],(void**)&threadResults);
		//printf("thread %d joined \n", threadID);

		if(err){
			printf("The thread wasn't successfully created");
			return 0;
		}
		threadResultMatrix = threadResults->r_matrix;
		if(threadResultMatrix == NULL)
		{
			free(threadResults);
			continue;
		}
		resultFirstRLine = threadResults->firstRLine;
		resultLastRLine = threadResults->lastRLine;
		resultFirstCLine = threadResults->firstCLine;
		resultLastCLine = threadResults->lastCLine;
		printf("before copy of thread %d, ResultMatrix -1 value is: %x\n",threadID, resultMatrix[-1]);
		int threadResultCols = resultLastCLine-resultFirstCLine+1;
		// put the data in the threads back together

		for(resultRow = resultFirstRLine; resultRow<= resultLastRLine; resultRow++){
			int currentThreadRow = resultRow - resultFirstRLine;
			int currentResultRow = resultRow*col_b;
			for(resultColumn = resultFirstCLine; resultColumn <= resultLastCLine; resultColumn++){
				resultMatrix[currentResultRow+ resultColumn] = threadResultMatrix[
					currentThreadRow*threadResultCols +(resultColumn-resultFirstCLine)];
			}
		}	
		printf("before free of thread %d, ResultMatrix -1 value is: %x\n",threadID, resultMatrix[-1]);
		free(threadResults);			
		free(threadResultMatrix);
		fflush(stdout);
		
	}

	// print the output matrix dimmensions 
	
	free(matrix_a);
	free(matrix_b);

	printf("%d %d\n", row_a, col_b);
	
	// print out each element of the matrix
	for(resultRow = 0; resultRow < row_a; resultRow++)
		for(resultColumn = 0; resultColumn < col_b; resultColumn++)
			printf("%f\n",resultMatrix[resultRow*col_b + resultColumn]);

	free(resultMatrix);
	return 0; 
}



void* multiply_thread(void* argv){

	//unpack the data & allocate destination matrix
	int i;
	int j;
	int k;
	float r_sum;
	float* a;
	float* b;
	int a_col;
	int b_row;
	int b_col;
	int firstRLine;
	int lastRLine;
	int firstCLine;
	int lastCLine;
	ResultStruct* result; 
	ArgStruct* myArgs;
	float* r_mat;
	int r_row;
	int r_col;

	myArgs = (ArgStruct*)argv;
	firstRLine = myArgs->firstRLine;
	lastRLine = myArgs->lastRLine;
	firstCLine = myArgs->firstCLine;
	lastCLine = myArgs->lastCLine;
	r_row = lastRLine - firstRLine + 1;
	r_col = lastCLine - firstCLine + 1;
	result = (ResultStruct*)malloc(sizeof(ResultStruct));
	//first check to see that the size is nonzero
	if(r_row*r_col != 0)
	{		
		r_mat = (float*)malloc(r_row*r_col*sizeof(float));
		a = myArgs->matInfo->matrix_a;
		b = myArgs->matInfo->matrix_b;
		a_col = myArgs->matInfo->col_a;
		b_row = myArgs->matInfo->row_b;
		b_col = myArgs->matInfo->col_b;
		free(myArgs);

		// calculate result for individual thread
		for(i=firstRLine; i <= lastRLine; i++){
			//each row
			int rowLoc= i*a_col;
			for(j=firstCLine; j <= lastCLine; j++){
				//each entry in a row
				r_sum = 0.0;
				for(k = 0; k < b_row; k++){
					r_sum += a[rowLoc + k]*b[k*b_col + j];
				}
				//assert((i-firstLine)*r_col + j >= 0);
				//assert((i-firstLine)*r_col + j <= (lastLine- firstLine+1)*r_col);
				r_mat[(i-firstRLine)*r_col + (j-firstCLine)] = r_sum;
			}
		}
	
		result->r_matrix = r_mat;
	}
	else
	{
		free(myArgs);
		result->r_matrix = NULL;
	}
	// package results and relevant data
	result->firstRLine = firstRLine;
	result->lastRLine = lastRLine;
	result->firstCLine = firstCLine;
	result->lastCLine = lastCLine;

	// pass back the package
	return ((void*)result);
}

