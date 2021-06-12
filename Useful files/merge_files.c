//merge_files.c

#include <stdio.h>
#include <stdlib.h>
  
int main(int argc, char** argv){
	if(argc <= 1 ){
		fprintf(stderr, "To merge files, insert their names and the name of the file you want to merge them in.\n");
	}
	
	// Open files to be merged 
	FILE** fp = (FILE**) calloc(argc - 1, sizeof(FILE*));

	// Open file to store the result
	FILE *fp_tot = fopen(argv[argc - 1], "w");
	char c;
	int i;

	//read the files and copy them
	for(i = 0; i < argc - 2; i++){
		fp[i] = fopen(argv[i + 1], "r");

		if(fp[i] == NULL){
		       	fprintf(stderr,"Couldn't open file %s\n",argv[i+1]);
			exit(0);
		}	
		while ((c = fgetc(fp[i])) != EOF) fputc(c, fp_tot);
	}
      
	printf("Files merged correctly \n");	
	
	//close files
	for(i = 0; i < argc - 2; i++) fclose(fp[i]);
	fclose(fp_tot);
	
	return 0;
}
