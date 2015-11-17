#include <stdio.h>
#include <string.h>
#include <stdlib.h>

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
	int firstLine;
	int lastLine;
	MatrixInfo * matInfo;
} ArgStruct;

typedef struct result_struct{
	float* r_matrix;
	int firstLine;
	int lastLine;
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
	int i, thread_num;
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
						if(k != a_size -1){
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
						if(k != b_size -1){
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
				thread_num = atoi(argv[i+1]);
				if(thread_num <= 0){
					printf("Error: Invalid thread count\n");
					return 0;
				}
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
	pthread_t* threadIDs  =(pthread_t*) malloc(thread_num * sizeof(pthread_t));


	// calculate the thread division

	int nextLine = 0;
	for(i = 0; i < thread_num - 1; i++){
		// pack the data for the threads
		ArgStruct* threadArg = (ArgStruct*)malloc(sizeof(ArgStruct));
		threadArg->matInfo = matrixInfo;
		threadArg->firstLine = nextLine;
		threadArg->lastLine = nextLine + (row_a - nextLine - 1)/(thread_num-i);
		nextLine = threadArg->lastLine +1;

		// create the threads
		err = pthread_create(threadIDs+i,NULL,multiply_thread,threadArg);

		if(err){
			printf("The thread wasn't successfully created");
			return 0;
		}
	}
	// get data back from the threads

	ResultStruct* threadResults; 
	float* resultMatrix = malloc(sizeof(float)*row_a*col_b);
	float* threadResultMatrix;
	int resultFirstLine,resultLastLine;
	int threadID;
	int resultRow, resultColumn;
	for(threadID = 0; threadID < thread_num; threadID++){
		err = pthread_join(threadIDs[i],(void**)&threadResults);
		if(err){
			printf("The thread wasn't successfully created");
			return 0;
		}
		resultFirstLine = threadResults->firstLine;
		resultLastLine = threadResults->lastLine;
		threadResultMatrix = threadResults->r_matrix;
		// put the data in the threads back together

		for(resultRow = resultFirstLine; resultRow<= resultLastLine; resultRow++){
			for(resultColumn = 0; resultColumn < col_b; resultColumn++){
				resultMatrix[resultRow*col_b+ resultColumn] = threadResultMatrix[
					(resultRow - resultFirstLine)*col_b +resultColumn];
			}
		}				
		free(threadResultMatrix);
		free(threadResults);
	}

	// print the output matrix dimmensions 
	
	printf("%d %d\n", row_a, col_b);
	
	// print out each element of the matrix
	for(resultRow = 0; resultRow < row_a; resultRow++)
		for(resultColumn = 0; resultColumn < col_b; resultColumn++)
			printf("%f\n",resultMatrix[resultRow*col_b + resultColumn]);

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
	int firstLine;
	int lastLine;
	ResultStruct* result; 
	ArgStruct* myArgs;
	float* r_mat;
	int r_row;
	int r_col;

	myArgs = (ArgStruct*)argv;
	firstLine = myArgs->firstLine;
	lastLine = myArgs->lastLine;
	r_row = lastLine - firstLine;

	r_col = myArgs->matInfo->col_b;
	r_mat = (float*)malloc(r_row*r_col*sizeof(float));
	
	a = myArgs->matInfo->matrix_a;
	b = myArgs->matInfo->matrix_b;
	a_col = myArgs->matInfo->col_a;
	b_row = myArgs->matInfo->row_b;
	free(myArgs);

	// calculate result for individual thread
	for(i=firstLine; i <= lastLine; i++){
		//each row
		for(j=0; j < r_col; j++){
			//each entry in a row
			r_sum = 0.0;
			for(k = 0; k < b_row; k++){
				r_sum += a[i*a_col + k]*b[k*r_col + j];
			}
			r_mat[(i-firstLine)*r_col + j] = r_sum;
		}
	}
	
	// package results and relevant data
	result = (ResultStruct*)malloc(sizeof(ResultStruct));
	result->firstLine = firstLine;
	result->lastLine = lastLine;
	result->r_matrix = r_mat;

	// pass back the package
	return ((void*)result);
}

