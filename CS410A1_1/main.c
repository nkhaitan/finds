#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <dirent.h>
#include <limits.h>

#define MAX_PATHSIZE 1024

int readFile(char * fileName, char *findString)
{
    FILE *fp;
    if ((fp=fopen(fileName,"r")) == NULL)
    {
        fprintf(stderr, "ERROR OPENING THE FILE\n");
        return 0;
    }
    char  tmp[256]={0x0};
    while(fp!=NULL && fgets(tmp, sizeof(tmp),fp)!=NULL)
    {
        if ((strstr(tmp, findString)))
        {
            printf("%s", tmp);
        }
    }
    if(fp!=NULL)
    {
        fclose(fp);
    }
    return 0;
}

const char *get_filename_ext(const char *filename)
{
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

void printdir(char *dir, char * findString, char * findType, int depth)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;
    if((dp = opendir(dir)) == NULL)
    {
        fprintf(stderr,"cannot open directory: %s\n", dir);
        return;
    }
    
    
    chdir(dir);
    while((entry = readdir(dp)) != NULL)
    {
        lstat(entry->d_name,&statbuf);
        if(S_ISDIR(statbuf.st_mode))
        {
            /* Found a directory, but ignore . and .. */
            if(strcmp(".",entry->d_name) == 0 ||strcmp("..",entry->d_name) == 0)
            {
                continue;
            }
            if(strstr(entry->d_name, findString))
            {
                printf("%*s%s/\n",depth,"",entry->d_name);
            }
            /* Recurse at a new indent level */
            printdir(entry->d_name,findString, findType, depth+4);
        }
        else if(strstr(entry->d_name, findString))
        {
            const char *extension = get_filename_ext(entry->d_name);
            if((strcmp(findType,extension) == 0))
            {
                printf("%*s%s\n",depth,"",entry->d_name);
                readFile(entry->d_name, findString);
                printf("********EOF*********\n");
            }
        }
    }
    chdir("..");
    closedir(dp);
}


int main (int argc, char **argv)
{
    int pflag = 0;
    int fflag = 0;
    int sflag = 0;
    char *pvalue = NULL;
    char *fvalue = NULL;
    char *svalue = NULL;
    int index;
    int c;
    
    opterr = 0;
    
    while ((c = getopt (argc, argv, "p:f:s:")) != -1)
        switch (c)
    {
        case 'p':
            pflag = 1;
            if (strcmp(optarg,"f") == 0 || strcmp(optarg, "s") == 0) {
                fprintf(stderr, "INVALID ARGUMENT");
                return 1;
            }
            pvalue = optarg;
            break;
        case 'f':
            fflag = 1;
            fvalue = optarg;
            break;
        case 's':
            sflag = 1;
            svalue = optarg;
            break;
        case '?':
            if (optopt == 'p' && (strcmp(pvalue,"f") != 0||strcmp(svalue,"s") != 0))
                fprintf (stderr, "Option -%c requires an argument.\n", optopt);
            else if (isprint (optopt))
                
                fprintf (stderr, "Unknown option `-%c'.\n", optopt);
            else
                fprintf (stderr, "Unknown option character `\\x%x'.\n",optopt);
            return 1;
        default:
            abort ();
    }
    
    printf ("pathname = %s,  filetype = .%s,  string = %s\n", pvalue, fvalue, svalue);
    printdir(pvalue, svalue, fvalue, 0);
    for (index = optind; index < argc; index++)
        printf ("Non-option argument %s\n", argv[index]);
    return 0;
}