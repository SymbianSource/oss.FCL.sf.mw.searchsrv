
#if 0 
#define _CL_HAVE_DIRENT_H

#include <dirent.h>
#include <stdio.h>
#include <string.h>


/**
 * The path of 'child' directory's parent is copied to 'parent'. 
 * The 'parent' char array is sized parent_max. If the size of 
 * parent path exceeds parent_max or some other error occurs, -1 is 
 * returned, otherwise 0. '\' and '/' are considered as path delimiters
 * independent of the underlying OS and filesystem. 
 */
int parentdir(char* parent, const char* child, size_t parent_max) {
	int len = strlen(child);
	if (child[len-1] == '\\' ||
			child[len-1] == '/') {
		len--;
	}
	int i = len-1;
	while (i >= 0) {
		char c = child[i];
		if (c == '\\' ||
		    c == '/') {
		    // found parent path 
		    break;
		}
		i--;
	}
	if (i+1 >= parent_max || i < 0) {
		return -1; // WTF?
	}
	memcpy(parent, child, i);
    parent[i] = 0;
    return 0;
}

/**
 * Creates directories recursively for given path. Returns 
 * 0 on if full directory path is exist after this call, non-zero otherwise.
 */
int mkdirs(const char* path, int mod) {
	DIR* d = opendir(path);
	if (d) {
		closedir(d); // already exist
		return 0;
	} 
	char parent[FILENAME_MAX];
	if (parentdir(parent, path, FILENAME_MAX) >= 0) {
	    // make the parent
    	mkdirs(parent, mod); 
	} 
	return mkdir(path, mod);
}


#endif // 0
