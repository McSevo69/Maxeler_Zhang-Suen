#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include "Maxfiles.h"
#include <MaxSLiCInterface.h>

#include "ppmIO.h"

typedef uint32_t dataType;

float verifyResults(int *outVector, int *expectedVector, int size) {
	int errors = 0;
	for (int i = 0; i < size; i++) {
		if (outVector[i] != expectedVector[i]) {
			printf("%d != %d \n", outVector[i], expectedVector[i]);
			errors++;
		}
	}

	printf("elements: %d, errors: %d\n", size, errors);

	return 100.0 - ((float) errors/ (float) size)*100;
}

//number of 0,1 patterns
int getA(int neighbors[]) {
        int count = 0;

	count += (neighbors[0] == 0 && neighbors[1] > 0) ? 1 : 0; //p2 p3
	count += (neighbors[1] == 0 && neighbors[2] > 0) ? 1 : 0; //p3 p4

	count += (neighbors[2] == 0 && neighbors[3] > 0) ? 1 : 0; //p4 p5	
	count += (neighbors[3] == 0 && neighbors[4] > 0) ? 1 : 0; //p5 p6	

	count += (neighbors[4] == 0 && neighbors[5] > 0) ? 1 : 0; //p6 p7	
	count += (neighbors[5] == 0 && neighbors[6] > 0) ? 1 : 0; //p7 p8

	count += (neighbors[6] == 0 && neighbors[7] > 0) ? 1 : 0; //p8 p9
	count += (neighbors[7] == 0 && neighbors[0] > 0) ? 1 : 0; //p9 p2
        
	return count;
}
 
int getB(int neighbors[]) {

	return neighbors[0] + neighbors[1] + neighbors[2] 
			+ neighbors[3] + neighbors[4] + neighbors[5] 
			+ neighbors[6] + neighbors[7];
}

void getNeighbors(int* binaryImage, int neighbors[], int y, int x, int width, int height) {

	int index = y*width+x;
	
	neighbors[0] = (y - 1 >= 0) ? binaryImage[index - width] : 0; //p2
	neighbors[1] = (y - 1 >= 0 && x + 1 < width) ? binaryImage[index - width + 1] : 0; //p3
	neighbors[2] = (x + 1 < width) ? binaryImage[index + 1] : 0; //p4
	neighbors[3] = (y + 1 < height && x + 1 < width) ? binaryImage[index + width + 1] : 0; //p5
	neighbors[4] = (y + 1 < height) ? binaryImage[index + width] : 0; //p6
	neighbors[5] = (y + 1 < height && x - 1 >= 0) ? binaryImage[index + width - 1] : 0; //p7
	neighbors[6] = (x - 1 >= 0) ? binaryImage[index - 1] : 0; //p8
	neighbors[7] = (y - 1 >= 0 && x - 1 >= 0) ? binaryImage[index - width - 1] : 0; //p9
		
}

int VectorsCPU(int *binaryImage, int height, int width) {

	dataType * pointsToChange = calloc(height*width, sizeof(dataType));

	int hasChanges = 1, changeCnt, a, b, it = 0;
	int neighbors[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	do {
		printf("Iteration %d\n", it++);
		hasChanges = 0;
		changeCnt = 0;		

		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				int index = y*width+x;
				getNeighbors(binaryImage, neighbors, y, x, width, height);
				a = getA(neighbors);
				b = getB(neighbors);

				if (binaryImage[index] == 1 && 2 <= b && b <= 6 && a == 1
				        && (neighbors[0] * neighbors[2] * neighbors[4] == 0)
					&& (neighbors[2] * neighbors[4] * neighbors[6] == 0)) {
					pointsToChange[changeCnt++] = index;
					hasChanges = 1;
				}
        		}
		}

		for (int i = 0; i < changeCnt; i++) {
		    binaryImage[pointsToChange[i]] = 0;
		}

		changeCnt = 0;
		for (int y = 0; y < height; y++) {
			for (int x = 0; x < width; x++) {
				int index = y*width+x;
				getNeighbors(binaryImage, neighbors, y, x, width, height);
				a = getA(neighbors);
				b = getB(neighbors);

				if (binaryImage[index] == 1 && 2 <= b && b <= 6 && a == 1
				        && (neighbors[0] * neighbors[2] * neighbors[6] == 0)
					&& (neighbors[0] * neighbors[4] * neighbors[6] == 0)) {
				    pointsToChange[changeCnt++] = index;
				    hasChanges = 1;
				}
			}
		}
        
		for (int i = 0; i < changeCnt; i++) {
		    binaryImage[pointsToChange[i]] = 0;
		}

	} while(hasChanges && it < 10000);
	
	free(pointsToChange);

	return it;
}

void printInImage(int *inImage, int width, int height) {
	printf("\n");
	for (int y = 1; y + 1 < height; y++) {
		for (int x = 1; x + 1 < width; x++) {
			int index = y*width+x;
			printf("%d ", inImage[index]);              
		}
		printf("\n");
	}
}

int checkForChanges(dataType *inImage, dataType *inImage2, int width, int height) {

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (inImage[y*Vectors_width+x] != inImage2[y*Vectors_width+x]) return 1;              
		}
	}

	return 0;
}

int convertArgToInt(char * str) {
	if (strcmp ("-in", str) == 0) return 1;
	else if (strcmp ("-out", str) == 0) return 2;
	else if (strcmp ("-print", str) == 0) return 3;
	else if (strcmp ("-benchmark", str) == 0) return 4;
	else if (strcmp ("-check", str) == 0) return 5;
	else return 0;
}

int main(int argc, char *argv[]) {
	//time measurement
	struct timeval begin, end;
	double timeSpentCPU = 0, timeSpent = 0;
	int *inImage;
	int width = 0, height = 0, it=0;

	printf("=================\n");
	printf("Thinning stream\n");
	printf("Precision: %ld\n",sizeof(dataType)*8);
	printf("=================\n");
	
	size_t benchmark = 0, print = 0, check = 0, inSet = 0, outSet = 0;
	char *inPath, *outPath;

	//commandline parameter parsing
	for (int i=1; i<argc; i++) {
		switch(convertArgToInt(argv[i])) {
			case 1: inPath = argv[++i]; inSet = 1; break;
			case 2: outPath = argv[++i]; outSet = 1; break;
			case 3: print = 1; break;
			case 4: benchmark = 1; break;
			case 5: check = 1; break;
			default: break;
		}
	}

	if (!inSet) {
		printf("No input path (param: -in) set. Aborting...\n");
		return 1;
	}

	if (!outSet) {
		printf("WARNING: No output path (param: -out) set. Given image will be overwritten.\n");
		outPath = inPath;
	}

	printf("Loading image.\n");
	
	loadImage(inPath, &inImage, &width, &height, 1);

	int dataSize = width * height * sizeof(int);
	// Allocate a buffer for the output image
	int *outImage = malloc(dataSize);
	int *outImageCPU = malloc(dataSize);

	printf("Converting image.\n");
	printf("width: %d, height: %d\n", width, height);
	for (int i = 0; i < height*width; i++) {
	    if (inImage[i]) { //0 (black) turns to 1, all other values to 0
	        outImage[i] = 0;
		outImageCPU[i] = 0;
	    } else {
	        outImage[i] = 1;
		outImageCPU[i] = 1;
	    }
	}

	//Zero padding
	dataType *inImageZeroPadded = calloc(Vectors_width*Vectors_height, sizeof(dataType));

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			inImageZeroPadded[y*Vectors_width+x] = outImage[y*width+x];
		}
	}
	
	dataType *outImageDFE = malloc(Vectors_width * Vectors_height * sizeof(dataType));
	dataType *outImageDFE2 = malloc(Vectors_width * Vectors_height * sizeof(dataType)); //used as a buffer	

	if (print) {
		printf("Printing image.\n");
		printInImage(inImage, width, height);
	}

	printf("Running DFE.\n");
	printf("Iteration %d\n", it++);
	gettimeofday(&begin, NULL);
	Vectors(Vectors_width*Vectors_height, inImageZeroPadded, outImageDFE);
	int hasChanges = checkForChanges(inImageZeroPadded, outImageDFE, width, height);
	gettimeofday(&end, NULL);
		timeSpent += (end.tv_sec - begin.tv_sec) +
	            ((end.tv_usec - begin.tv_usec)/1000000.0);	
	
	while (hasChanges) {
		printf("Iteration %d\n", it++);
		
		gettimeofday(&begin, NULL);
		Vectors(Vectors_width*Vectors_height, outImageDFE, outImageDFE2);
		hasChanges = checkForChanges(outImageDFE, outImageDFE2, width, height);
		gettimeofday(&end, NULL);
		timeSpent += (end.tv_sec - begin.tv_sec) +
	            ((end.tv_usec - begin.tv_usec)/1000000.0);

		if (!hasChanges) {
			for (int i=0; i<Vectors_width*Vectors_height; i++) outImageDFE[i] = outImageDFE2[i];
			break;
		}
	
		printf("Iteration %d\n", it++);
		gettimeofday(&begin, NULL);
		Vectors(Vectors_width*Vectors_height, outImageDFE2, outImageDFE);
		hasChanges = checkForChanges(outImageDFE, outImageDFE2, width, height);
		gettimeofday(&end, NULL);
		timeSpent += (end.tv_sec - begin.tv_sec) +
	            ((end.tv_usec - begin.tv_usec)/1000000.0);
	}

	printf("Time DFE: %lf iterations: %d\n", timeSpent, it-1);

	//Removing zero padding
	
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			 outImage[y*width+x] = outImageDFE[y*Vectors_width+x];
		}
	}	

	if (check) {
		printf("Running CPU.\n");
		gettimeofday(&begin, NULL);
		it = VectorsCPU(outImageCPU, height, width);
		gettimeofday(&end, NULL);
		timeSpentCPU += (end.tv_sec - begin.tv_sec) +
	            ((end.tv_usec - begin.tv_usec)/1000000.0);
		printf("Time CPU: %lf\n", timeSpentCPU);
		printf("Speedup: %f\n", timeSpentCPU/timeSpent);

		float acc = verifyResults(outImage, outImageCPU, width*height);
		printf("The accuracy measured is: %f\n", acc);
	}

	//Exporting benchmark results
	if (benchmark) {
		FILE* time_res;
		char filename[32];
		printf("Exporting to file...\n");
		snprintf(filename, sizeof(filename), "benchmark.csv");
		time_res = fopen(filename,"a");
		fprintf(time_res, "%d, %d, %d, %f, %f\n", width, height, it, timeSpentCPU, timeSpent);
		fclose(time_res);
	}
	
	printf("Converting image.\n");
	for (int i = 0; i < height*width; i++) {
	    if (outImage[i]) { //0 (white) turns to 255, black to 0
	        inImage[i] = 0;
	    } else {
	        inImage[i] = 255;
	    }
	}

	printf("Saving image.\n");
	writeImage(outPath, inImage, width, height, 1);

	free(outImage);
	free(inImage);
	free(inImageZeroPadded);
	free(outImageCPU);	
	free(outImageDFE);
	free(outImageDFE2);
	
	return 0;
}
