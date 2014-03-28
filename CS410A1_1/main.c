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

/*
 WORK LEFT:
 1.) Make own printf statement
 */

#define CHECKFILENAME 1; // 0: Match file name, 1: Don't match file name

/* Convert the integer D to a string and save the string in BUF. If
 BASE is equal to 'd', interpret that D is decimal, and if BASE is
 equal to 'x', interpret that D is hexadecimal. */
void itoa (char *buf, int base, int d)
{
    char *p = buf;
    char *p1, *p2;
    unsigned long ud = d;
    int divisor = 10;
    
    /* If %d is specified and D is minus, put `-' in the head. */
    if (base == 'd' && d < 0)
    {
        *p++ = '-';
        buf++;
        ud = -d;
    }
    else if (base == 'x')
        divisor = 16;
    
    /* Divide UD by DIVISOR until UD == 0. */
    do
    {
        int remainder = ud % divisor;
        
        *p++ = (remainder < 10) ? remainder + '0' : remainder + 'a' - 10;
    }
    while (ud /= divisor);
    
    /* Terminate BUF. */
    *p = 0;
    
    /* Reverse BUF. */
    p1 = buf;
    p2 = p - 1;
    while (p1 < p2)
    {
        char tmp = *p1;
        *p1 = *p2;
        *p2 = tmp;
        p1++;
        p2--;
    }
}

int isAlNum(char *findString)
{
    int i = 0;
    while (findString[i] != '\0')
    {
        if (isalnum(findString[i]) != 0)
            i++;
        else
            return 0;
    }
    return 1;
}

int checkValidWildCards(char * findString)
{
    // 0: No valid wild cards present
    // 1: Valid wild cards present
    // 2: Invalid wild card characters present
    char *dot, *star, *question;
    if (isAlNum(findString))
        return 0;
    else
    {
        // Verify if there are only valid wild card characters
        if ((dot = strchr(findString, '.')) != NULL ||
            (star = strchr(findString, '*')) != NULL||
            (question = strchr(findString, '?')) != NULL)
            return 1;
        else
            return 2;
    }
}

int checkDot(char * findString, char * mainString)
{
    char test1String[1024];
    char part2test[256];
    strcpy(test1String, findString);
    
    if (strchr(findString, '.') !=NULL)
    {
        const char * part1 = strtok(test1String, ".");
        const char * part2 = strchr(findString, '.') + 1;
        const char * found = strstr(mainString, part1);
        strcpy(part2test, part2);
        
        //Check for 2 wild cards
        if(strchr(findString, '*'))
        {
            part2 = strtok(part2test,"*");
        }
        if(strchr(findString, '?'))
        {
            part2 = strtok(part2test,"?");
        }
        if (found)
        {
            int part1index = found - mainString;
            int part2index = part1index + strlen(part1) + 1; // Index of the 2nd half of string
            if (isalnum(mainString[part2index - 1])) // Check if the middle character is alphanumeric
            {
                int i,k=0;
                for(i = part2index; part2[k] != '\0';i++)
                {
                    if (mainString[i] != part2[k])
                    {
                        return 0;
                    }
                    k++;
                }
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
    return 1;
}

int checkStar(char * findString, char * mainString)
{
    if (strchr(findString, '*') != NULL)
    {
        const char part1 = *(strchr(findString, '*') - 1); // Character before the wildcard
        const char * part2 = strchr(findString, '*') + 1; // Part of findString right after the wildcard
        const char * found = strstr(mainString, part2);
        if(found)
        {
            int part2index = found - mainString; // Index of the 2nd part of findString in the file line (mainString)
            int i,k=0;
            for(i = part2index; part2[k] != '\0';i++)
            {
                // If the second string does not match
                if (mainString[i] != part2[k])
                {
                    return 0;
                }
                k++;
            }
            // If the character before the wildcard does not match
            if (mainString[part2index-1] != part1)
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
    return 1;
}

int checkQuestion(char * findString, char * mainString)
{
    if (strchr(findString, '?') != NULL)
    {
        const char part1 = *(strchr(findString, '?') - 1); // Character before the wildcard
        const char * part2 = strchr(findString, '?') + 1; // Part of findString right after the wildcard
        const char * found = strstr(mainString, part2);
        if(found)
        {
            int part2index = found - mainString; // Index of the 2nd part of findString in the file line (mainString)
            int i, k=0;
            for( i = part2index; part2[k] != '\0';i++)
            {
                // If the second string does not match
                if (mainString[i] != part2[k])
                {
                    return 0;
                }
                k++;
            }
            // If the character before the wildcard does not match or 2 chars before the string matches
            if (mainString[part2index-1] != part1 || mainString[part2index-2] == part1)
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }
    return 1;
}


int readFile(char * fileName, char *findString, char *fullPathName, int isWildCard)
{
    FILE *fp;
    int foundFlag = 0;
    if ((fp=fopen(fileName,"r")) == NULL)
    {
        fprintf(stderr, "CANNOT OPEN THE FILE: %s\n\n",strcat(fullPathName,fileName));
        return 0;
    }
    char tmp[256]={0x0};
    while(fp!=NULL && fgets(tmp, sizeof(tmp),fp)!=NULL)
    {
        if(isWildCard) // If there are valid wildcards
        {
            int isDot = checkDot(findString, tmp);
            int isStar = checkStar(findString, tmp);
            int isQuestion = checkQuestion(findString, tmp);
            if ((foundFlag == 0) && (isDot || isStar || isQuestion)) // Test for wildcards
            {
                foundFlag = 1;
                printf("%s\n", strcat(fullPathName,fileName));
                printf("%s", tmp);
            }
            else if (foundFlag && (isDot || isStar|| isQuestion)) // Test for wildcards
            {
                printf("%s", tmp);
            }
        }
        else // If there are no valid wildcards
        {
            if ((strstr(tmp, findString)) && (foundFlag == 0)) // Implement wildcards here
            {
                foundFlag = 1;
                printf("%s\n", strcat(fullPathName,fileName));
                printf("%s", tmp);
            }
            else if ((strstr(tmp, findString)) && foundFlag) // Implement wildcards here
            {
                printf("%s", tmp);
            }
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

void printdir(char *dir, char * findString, int checkFileName, char * findType, int isWildCard, int depth)
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
            printdir(entry->d_name,findString, checkFileName, findType, isWildCard, depth+4);
        }
        else if(strstr(entry->d_name, findString) || checkFileName)
        {
            const char *extension = get_filename_ext(entry->d_name);
            if((strcmp(findType,extension) == 0) || strcmp(findType,"0") == 0)
            {
                long size;
                char *buf;
                char *fullPath;
                size = pathconf(".", _PC_PATH_MAX);
                if ((buf = (char *)malloc((size_t)size)) != NULL)
                {
                    fullPath = getcwd(buf, (size_t)size);
                }
                fullPath[strlen(fullPath)] = '/';
                fullPath[strlen(fullPath)+1] = '\0';
                readFile(entry->d_name, findString, fullPath, isWildCard);
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
    int checkFileName = CHECKFILENAME;
    opterr = 0;
    
    while ((c = getopt (argc, argv, "p:s:f:")) != -1)
        switch (c)
    {
        case 'p':
            pflag = 1;
            if (optarg[0] != '/') // If the path name doesn't start with '/'
            {
                fprintf(stderr, "INVALID PATH NAME: %s\n", optarg);
                return 1;
            }
            pvalue = optarg;
            break;
        case 'f':
            fflag = 1;
            if (optarg[0] != 'c' && optarg[0] != 'h' && optarg[0] != 'S')
            {
                fprintf(stderr, "EXTENSION '%s' NOT SUPPORTED.\n", optarg);
                return 1;
            }
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
    int isWildCard = checkValidWildCards(svalue);
    fvalue == NULL ? fvalue = "0" : fvalue; // Flag to show NULL 'f' argument
    if (isWildCard == 2)
    {
        printf("Invalid wildcard character(s) present in string '%s'\n",svalue);
        return 0;
    }
    printdir(pvalue, svalue, checkFileName, fvalue, isWildCard, 0);
    for (index = optind; index < argc; index++)
        printf ("Non-option argument %s\n", argv[index]);
    return 0;
}