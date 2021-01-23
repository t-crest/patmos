#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "sdc_debug.h"
#include "ff.h"

FRESULT scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    static FILINFO fno;


    res = f_opendir(&dir, path);                       /* Open the directory */
    DEBUG_PRINT("res = %d", res);
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            DEBUG_PRINT("res = %d", res);
            if (res != FR_OK || fno.fname[0] == 0){
                //DEBUG_PRINT("res = %d", res);
                break;  /* Break on error or end of dir */
            } 
            DEBUG_PRINT("res = %d", res);
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path);                    /* Enter the directory */
                DEBUG_PRINT("res = %d", res);
                if (res != FR_OK) {
                    //DEBUG_PRINT("res = %d", res);
                    break;
                }
                path[i] = 0;
            } else {                                       /* It is a file. */
                printf("%s/%s\n", path, fno.fname);
            }
        }
        f_closedir(&dir);
    }

    return res;
}

int main()
{
    FATFS fs;
    FRESULT res;
    char buff[256];

    res = f_mount(&fs, "", 1);
    /*DEBUG_PRINT("res = %d", res);
    if (res == FR_OK) {
        strcpy(buff, "/");
        
        DEBUG_PRINT("res = %d", res);
    }*/
    FIL f;
    UINT a;
    res = f_open(&f,"/demo.txt", FA_CREATE_ALWAYS | FA_READ | FA_WRITE);
    DEBUG_PRINT("f_open: %d", res);
    res = f_write(&f, "ABCDEFG", 8, &a);
    DEBUG_PRINT("f_write: %d", res);
    res = f_close(&f);
    DEBUG_PRINT("f_close: %d", res);

    res = scan_files(buff);

    return res;

}
