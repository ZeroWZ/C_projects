#include<stdio.h>
#include<stdlib.h>
#include<dirent.h>
#include<string.h>
#include<sys/stat.h>
#include<unistd.h>
#include<errno.h>

#ifndef PATH_MAX
#define PATH_MAX 512
#endif

int count;

int search(char *path,char *parentpath,char *target);
char *get_cwd(size_t size);

int main(int argc, char **argv)
{
    // Check input
    if(argc != 3 && argc != 2)
    {
        printf("Usage: search [path] target\n");
        return 1;
    }

    char c[] = ".";
    char *path = NULL;
    char *target = NULL;
    // If the user didn't provide a path
    if(argc == 2)
    {
        target = argv[1];
        path = c;
    }
    else // If the user provide a path
    {
        target = argv[2];
        path = argv[1];
    }

    // Changes the current working directory
    if(chdir(path) == -1)
    {
        fprintf(stderr, "chdir: %s\n", strerror(errno));
        return 1;
    }

    // Get the current working directory
    char *current_path = get_cwd(PATH_MAX);
    if(current_path == NULL)
        return 1;

    // Start searching
    printf("Looking for '%s'\n", target);
    if(search(current_path, NULL, target)){
        free(current_path);
        return 1;
    }

    printf("Found %d match", count);
    if(count != 1)
        printf("es");
    putchar('\n');

    free(current_path);
    return(0);
}

int search(char *path, char *parentpath, char *target)
{
    struct dirent *entry;
    struct stat st_buf;
    char *subpath = NULL;
 
    // Open a directory 
    DIR *dp = opendir(path);
    if(dp == NULL)
    {
        fprintf(stderr, "opendir: %s\n%s\n", path, strerror(errno));
        return 1;
    }

    // Read what in it 
    while((entry=readdir(dp)))
    {
        // If there is a match
        if(strcmp(entry->d_name, target) == 0)
        {
            printf("%s/%s\n", path, target);
            count++;
        }

        // Get the stat of an entry
        if (lstat(entry->d_name, &st_buf) != 0)
        {
            printf("Error %d: %s/%s\n", errno, path, entry->d_name);
            closedir(dp); 
            return 1;
        }   

        // If the entry if a directory
        if(S_ISDIR(st_buf.st_mode))
        {
            // Ignore "." and ".."
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
                continue;

            // Changes the current working directory
            if(chdir(entry->d_name) == -1)
            {
                fprintf(stderr,"Unable to change to %s\n", entry->d_name);
                closedir(dp);
                return 1;
            }
 
            // Get the current working directory
            subpath = get_cwd(PATH_MAX);
            if(subpath == NULL)
            {
                closedir(dp);
                return 1;
            }

            if(search(subpath, path, target))
            {
                free(subpath);
                closedir(dp);
                return 1;
            }       
            free(subpath);
        }
    }

    closedir(dp);

    if(chdir(parentpath) == -1)
    {
        if(parentpath == NULL)
            return 0;
        fprintf(stderr,"Parent directory lost\n");
        return 1;
    }
}

char *get_cwd(size_t size)
{
	char *buf;
	char *ptr;
		
	for (buf = ptr = NULL; ptr == NULL; size *=2)
	{
		if((buf = realloc(buf, size)) == NULL)
		{
			fprintf(stderr, "malloc: %s\n", strerror(errno));
			return NULL;
		}
		
		ptr = getcwd(buf, size);
		if(ptr == NULL && errno != ERANGE){
			fprintf(stderr, "getcwd: %s\n", strerror(errno));
			free(buf);
			return NULL;
		}
	}
	
	return buf;
}
