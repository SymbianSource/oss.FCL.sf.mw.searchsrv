/*------------------------------------------------------------------------------
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#ifndef CPIXTOOLS_CPIXFSTOOLS_H_
#define CPIXTOOLS_CPIXFSTOOLS_H_

#include <dirent.h>
#include <errno.h>

#include <string>

// A lot of POSIX system calls (open, close, read, write) can fail
// with EINTR which means that they just have to be retried. Now, even
// if OpenC does not support this, the code here is supposed to be
// platform independent and must work on a true *NIX (like linux).
#define Cpt_EINTR_RETRY(res,op)  while (((res=op) == -1) && (errno == EINTR)) { ; /* NOP */ }


// Same as Cpt_EINTR_RETRY, but for cases when the return value is not
// integer, but some pointer, and where failure is signalled through
// NULL
#define Cpt_EINTR_RETRY_PTR(resPtr,op)  while (((resPtr=op) == NULL) && (errno == EINTR)) { ; /* NOP */ }


// TODO make this platform independent macro
#define Cpt_PATH_SEPARATOR '\\'


namespace Cpt
{

    /**
     * Gets the parent of a child.
     *
     * @param parent the place to copy the parent
     * @param bufSize the number of bytes available for the parent,
     *        including the terminating zero
     * @param child the child
     *
     * @returns 0 if successful, -1 if not (too small buffer)
     */
    int getparent(char       * parent, 
                  size_t       bufSize,
                  const char * child);


    /**
     * Creates directories, recursively, if necessary.
     *
     * @param path the path to the directory to create4
     * @param mod the mode of the creation
     * @returns 0 on success, -1 otherwise
     */
    int mkdirs(const char* path, int mod);


    /**
     * Checks if a given file (or directory) could be opened for reading.
     *
     * @param path the path to the file to test
     *
     * @return true if it could be opened for reading
     */
    bool isreadable(const char * path);



    /**
     * Checks if a given path is a valid, regular file.
     *
     * @param path the path to check
     *
     * @return true if it is a regular file, false otherwise (does not
     * exist or not a regular file)
     */
    bool isfile(const char * path);
    

    /**
     * @param path the path to the directory to test
     *
     * @returns true if the directory path could be opened as a
     * directory, false otherwise.
     */
    bool directoryexists(const char * path);


    
    /**
     * Removes a file or directory (with all its contents
     * recursively).
     *
     * @param path path to remove. If it points to the directory, the
     * directory itself will be removed too. See removeunder().
     *
     * @returns 0 on success and -1 on error
     */
    int removeall(const char * path);


    /**
     * Removes all the files under a directory.
     *
     * @param path the path to the directory under wich everything
     * must be removed - the directory itself is not removed. If path
     * happens to point to a file, it will not be removed. See
     * removeall().
     *
     * @returns 0 on success and -1 on error
     */
    int removeunder(const char * path);


    /**
     * Obtains the size of a file.
     *
     * @param path the path to the file
     *
     * @returns the size of the file. -1 is returned on error.
     */
    off_t filesize(const char * path);
    

    /**
     * Obtains the size of a file.
     *
     * @param the file descriptor to an open file
     *
     * @returns the size of the file. -1 is returned on error.
     */
    off_t filesize(int fileDesc);


    
    /**
     * Returns when the file was last modified or 0 if some error occurred.
     */
    time_t filemodified(const char * path);


    
    enum CopyFileRetCode
        {
            CPT_CPF_OK              = 0,
            CPT_CPF_DST_OPEN_ERROR  = -1,
            CPT_CPF_SRC_OPEN_ERROR  = -2,
            CPT_CPF_DST_WRITE_ERROR = -3,
            CPT_CPF_SRC_READ_ERROR  = -4
        };
    /**
     * Copies the content of one file to another file.
     *
     * @param dstPath the path to copy to
     *
     * @param srcPath the path to copy from
     *
     * @return 0 if successful and negative number if failed. For
     * detailed error codes see enum CopyFileRetCode.
     */
    int copyFile(const char * dstPath,
                 const char * srcPath);



    enum CopyRetCode
        {
            CPT_CP_OK                = 0,
            CPT_CP_DST_OPEN_ERROR    = -1,
            CPT_CP_SRC_OPEN_ERROR    = -2,
            CPT_CP_DST_WRITE_ERROR   = -3,
            CPT_CP_SRC_READ_ERROR    = -4,
            CPT_CP_INVALID_ARG_ERROR = -5,
            CPT_CP_DIR_CREATE_ERROR  = -6
        };
    /**
     * Recursively copies the content of a file / directory from one
     * path to another.
     *
     * @param dstPath the path to copy to
     *
     * @param srcPath the path to copy from
     *
     * @param includeSrcDir whether to copy the src directory itself
     * to dst (inclusive) or only its contents (exclusive), obviously
     * makes sense only if src is a directory (and not a file).
     *
     * @return 0 if successful and negative number if failed. For
     * detailed error codes see enum CopyRetCode.
     */
    int copy(const char * dstPath,
             const char * srcPath,
             bool         includeSrcDir);



    class IFileVisitor
    {
    public:
        //
        // public operators
        //

        /**
         * Called when a file path is encountered during
         * traversal. Whether that file can be used/opened, is another
         * question.
         *
         * @param path the path to the file
         *
         * @returns true if traversal should go on, false otherwise
         *
         * NOTE: this method is allowed to throw anything.
         */
        virtual bool visitFile(const char * path) = 0;


        /**
         * Called when a directory path is encountered during
         * traversal - before traversing its children (but after the
         * directory stream has been opened).
         *
         * @param path the path to the file
         *
         * @returns true if traversal should go on, false otherwise
         *
         * NOTE: this method is allowed to throw anything.
         */
        virtual bool visitDirPre(const char * path) = 0;


        /**
         * Called when a directory path is encountered during
         * traversal - after traversing its children and after the
         * directory stream has been closed.
         *
         * @param path the path to the file
         *
         * @returns true if traversal should go on, false otherwise
         *
         * NOTE: this method is allowed to throw anything.
         */
        virtual bool visitDirPost(const char * path) = 0;


        virtual ~IFileVisitor() = 0;
    };


    /**
     * Traverses a file system (depth-first descending) from a point.
     *
     * @param path the path to start descending from
     *
     * @param visitor to visit directories and files.
     */
    void traverse(const char   * path,
                  IFileVisitor * visitor);
    

    /**
     * Appends a path fragment to a path, making sure that there is a
     * folder separator in between.
     *
     * @param path the path to append to - modified
     *
     * @param fragment the path part to append, with a folder
     * separator in between if it is not a suffix of path already.
     */
    void pathappend(std::string & path,
                    const char  * fragment);


    /**
     * A sentry class for DIR* types - so you don't have to call
     * closedir explicitly yourself.
     */
    class DIRSentry
    {
    private:
        //
        // private members
        //
        DIR * d_;

    public:
        /**
         * Constructs this sentry with the valid DIR* instance
         * obtained through opendir() call.
         */
        DIRSentry(DIR * d);


        /**
         * Destroys this sentry, and along with it the closedir
         * function is called for the stored DIR* instance.
         */
        ~DIRSentry();
    };
    


    /**
     * Class to guard open file descriptors against not being closed.
     */
    class FileDescSentry
    {
    private:
        //
        // private members
        //
        int    * fileDesc_;

        
    public:
        //
        // public operators
        //
        /**
         * Constructs this sentry with a pointer to an open file
         * descriptor. Unless release is called, the destructor of
         * this instance will (a) close the file descriptor and (b)
         * set it to -1.
         *
         * @param fd the pointer to the open file descriptor. May be
         * NULL in which case nothing is done.
         */
        FileDescSentry(int * fd);


        /**
         * Releases the file descrpitor guarded by this instance -
         * when this will be destructed, it will do nothing, once this
         * call has been made.
         */
        void release();


        /**
         * If this instance still holds a valid pointer to an integer
         * that is not -1, it will call close on the indicated file
         * descriptor and set the pointed variable to -1.
         */
        ~FileDescSentry();
    };



}

#endif
