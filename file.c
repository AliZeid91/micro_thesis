
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <inttypes.h>
#include <linux/i2c-dev.h> 
#include <sys/ioctl.h>

#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <inttypes.h>
#include <assert.h>
#include <stdbool.h>

/* Save the list */
void save(long test[]){
    FILE *fp;

    if((fp=fopen("Test 2","a"))==NULL){
        printf("Cannot open file.\n");
    }

    /* Write to file */
    for( int i=0; i<5; i++) {
        fprintf(fp,"%d\n",(int)test[i]);
    }

    fclose(fp);
}

/* Load the file */
void load(void){
    FILE *fp;

    if((fp=fopen("Test 1","rb"))==NULL){
        printf("Cannot open file\n");
    }

    /* read from file */

    fclose(fp);
}