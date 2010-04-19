/*------------------------------------------------------------------------------
* Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies). 
* 
* Distributable under the terms of either the Apache License (Version 2.0) or 
* the GNU Lesser General Public License, as specified in the COPYING file.
------------------------------------------------------------------------------*/

#include <fcntl.h>
#include <stdio.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

#include <string>

#include "cpixfstools.h"
#include "cpixstrtools.h"


namespace
{
    const char DIR_SEPARATOR = '/';
}



namespace Cpt
{

    int getparent(char       * parent, 
                  size_t       bufSize,
                  const char * child)
    {
        size_t
            len = strlen(child);
	if (child[len-1] == '\\' ||
            child[len-1] == '/') {
            len--;
	}

	size_t
            i = len-1;

	while (i >= 0) {
            char c = child[i];
            if (c == '\\' ||
                c == '/') {
                // found parent path 
                break;
            }
            i--;
	}
        
	if (i+1 >= FILENAME_MAX 
            || i+1 >= bufSize
            || i < 0) {
            return -1;
	}
	
        memcpy(parent, child, i);
        parent[i] = 0;
        return 0;
    }

    

    bool directoryexists(const char * path)
    {
        bool
            rv = false;

	DIR
            * d = NULL;

        Cpt_EINTR_RETRY_PTR(d, opendir(path));

	if (d) {
            int
                result;
            Cpt_EINTR_RETRY(result, closedir(d));
            rv = true;
	} 

        return rv;
    }


    /**
     * This class can be used in two distinct modes:
     *
     *  (a) removes everything, including the directory where
     *      the traversal was started (default constructor)
     *
     *  (b) removes only stuff under the start directory (constructor
     *      with path argument to start directory).
     */
    class Remover : public IFileVisitor
    {
        int              success_;
        std::string      start_;
    public:

        Remover()
            : success_(0)
        {
            ;
        }


        Remover(const char * start)
            : success_(0),
              start_(start)
        {
            ;
        }


        int success() const
        {
            return success_;
        }


        virtual bool visitFile(const char * path)
        {
            // 'remove' works for files and directories
            Cpt_EINTR_RETRY(success_,remove(path));

            return success_ == 0;
        }


        virtual bool visitDirPre(const char * /*path*/)
        {
            return true;
        }


        virtual bool visitDirPost(const char * path)
        {
            bool
                rv = true;

            if (start_.length() > 0
                && start_ != path)
                {
                    rv = visitFile(path);
                }

            return rv;
        }
        
    };



    int removeall(const char * path)
    {
        // this instance will remove anything
        Remover
            remover;

        traverse(path,
                 &remover);

        return remover.success();
    }


    int removeunder(const char * path)
    {
        // this instance will remove everything except the thing with
        // path 'path', which happens to be the directory where the
        // traversal starts
        Remover
            remover(path);

        traverse(path,
                 &remover);

        return remover.success();
    }


    
    int mkdirs(const char* path, int mod)
    {
        if (directoryexists(path))
            {
                return 0;
            }

        char 
            parent[FILENAME_MAX];

        if (getparent(parent, sizeof(parent), path) >= 0) {
            // make the parent
            mkdirs(parent, mod); 
        } 
        
        return mkdir(path, mod);
    }


    
    bool isreadable(const char * path)
    {
        int
            fd;
        Cpt_EINTR_RETRY(fd,
                        open(path,
                             O_RDONLY));
        bool
            rv = (fd != -1);
        
        if (rv)
            {
                int
                    result;
                Cpt_EINTR_RETRY(result,
                                close(fd));
            }

        return rv;
    }



    bool isfile(const char * path)
    {
        bool
            rv = false;

        struct stat 
            buf;

        int
            res = lstat(path,
                        &buf);

        if (res == 0)
            {
                rv = static_cast<bool>(S_ISREG(buf.st_mode));
            }

        return rv;
    }



    off_t filesize(const char * path)
    {
        off_t
            rv = 0;
        
        int
            res;
        struct stat
            buf;
        
        res = stat(path,
                   &buf);
        
        if (res == 0)
            {
                rv = buf.st_size;
            }
        
        return rv;
    }



    off_t filesize(int fileDesc)
    {
        off_t
            rv = 0;
        
        int
            res;
        struct stat buf;
        
        res = fstat(fileDesc,
                    &buf);
        
        if (res == 0)
            {
                rv = buf.st_size;
            }
        
        return rv;
    }



    time_t filemodified(const char * path)
    {
        time_t
            rv = 0;
        
        int
            res;
        struct stat buf;
        
        res = stat(path,
                   &buf);
        
        if (res == 0)
            {
                rv = buf.st_mtime;
            }
        
        return rv;
    }
    


    int copyFile(const char * dstPath,
                 const char * srcPath)
    {
        int
            rv = CPT_CPF_OK;

        ssize_t
            res;
        int
            dstFD;
        Cpt_EINTR_RETRY(dstFD,open(dstPath,
                                   O_WRONLY | O_TRUNC | O_CREAT,
                                   0666));
        if (dstFD == -1)
            {
                rv = CPT_CPF_DST_OPEN_ERROR;
                return rv;
            }
            
        int
            srcFD;
        Cpt_EINTR_RETRY(srcFD,open(srcPath,
                                   O_RDONLY));
        if (srcFD == -1)
            {
                Cpt_EINTR_RETRY(res,close(dstFD));

                rv = CPT_CPF_SRC_OPEN_ERROR;
                return rv;
            }

        char
            buffer[256];
        ssize_t
            readC = 0;
        while (true)
            {
                Cpt_EINTR_RETRY(readC,read(srcFD, 
                                           buffer, 
                                           sizeof(buffer)
                                           /sizeof(char)));
                if (readC == -1)
                    {
                        rv = CPT_CPF_SRC_READ_ERROR;
                        break;
                    }
                else if (readC == 0)
                    {
                        // end of file
                        break;
                    }
                else
                    {
                        ssize_t
                            writtenC = 0;

                        while (writtenC < readC)
                            {
                                Cpt_EINTR_RETRY(res,write(dstFD,
                                                          buffer + writtenC,
                                                          readC - writtenC));
                                if (res == -1)
                                    {
                                        rv = CPT_CPF_DST_WRITE_ERROR;
                                        break;
                                    }
                                else
                                    {
                                        writtenC += res;
                                    }
                            }
                    }
            }

        Cpt_EINTR_RETRY(res,close(srcFD));
        Cpt_EINTR_RETRY(res,close(dstFD));

        return rv;
    }



    /**
     * A file visitor that can copy files and directories.
     */
    class Copier : public IFileVisitor
    {
        int          result_;
        bool         inclusive_;
        
        std::string  srcBasePath_;
        std::string  dstBasePath_;

    public:
        Copier(const char * dstPath,
               const char * srcPath,
               bool         inclusive)
            : result_(CPT_CP_INVALID_ARG_ERROR),
              inclusive_(inclusive)
        {
            // NOTE: result_ is initialized to invalid argument error

            size_t
                pathSize = std::max(strlen(dstPath),
                                      strlen(srcPath)) + 1;
            auto_array<char>
                parentPath(new char[pathSize]);

            if (isfile(srcPath) && isreadable(srcPath))
                {
                    int
                        result = getparent(parentPath.get(),
                                           pathSize,
                                           srcPath);
                    
                    if (result == 0)
                        {
                            srcBasePath_ = parentPath.get();
                            
                            if (isfile(dstPath) && isreadable(srcPath))
                                {
                                    result = getparent(parentPath.get(),
                                                       pathSize,
                                                       dstPath);
                                    if (result == 0)
                                        {
                                            dstBasePath_ = parentPath.get();
                                            result_ = CPT_CP_OK;
                                        }
                                }
                            else
                                {
                                    dstBasePath_ = dstPath;
                                    result_ = CPT_CP_OK;
                                }
                        }
                }
            else if (directoryexists(srcPath))
                {
                    if (inclusive)
                        {
                            // If inclusiveness is required, then the
                            // src base path must be the parent path
                            // of the src path given here, so that the
                            // src directory gets explicitly copied
                            // too to dst.
                            //
                            // NOTE: siblings of src dir do not get
                            // copied, since traversal starts at
                            // srcPath, it's just what we set as base.
                            int
                                result = getparent(parentPath.get(),
                                                   pathSize,
                                                   srcPath);
                            if (result == 0)
                                {
                                    srcBasePath_ = parentPath.get();
                                }
                            else
                                {
                                    // must not proceed
                                    return;
                                }
                        }
                    else
                        {
                            // exclusive 
                            srcBasePath_ = srcPath;
                        }

                    if (!isfile(dstPath))
                        {
                            dstBasePath_ = dstPath;
                            result_ = CPT_CP_OK;
                        }
                    // else: insensible input: copying dir to file
                }
        }


        /**
         * Whether to copy the src directory itself to dst (inclusive)
         * or only its contents (exclusive).
         */
        void setInclusive(bool inclusive)
        {
            inclusive_ = inclusive;
        }


        int result() const
        {
            return result_;
        }


        virtual bool visitFile(const char * path)
        {
            std::string
                file(getDst(path));

            result_ = copyFile(file.c_str(),
                               path);

            return result_ == CPT_CP_OK;
        }


        virtual bool visitDirPre(const char * path)
        {
            std::string
                dir(getDst(path));

            int
                result = mkdirs(dir.c_str(),
                                0700);

            if (result != 0)
                {
                    result_ = CPT_CP_DIR_CREATE_ERROR;
                }

            return result_ == CPT_CP_OK;
        }


        virtual bool visitDirPost(const char * /* path */)
        {
            return result_ == CPT_CP_OK;
        }

    private:

        /**
         * path must be a child-or-same as srcPath given to
         * constructor. This method computes the destination path that
         * corresponds to the path argument - taking into account
         * srcBasePath_ and dstBasePath_.
         */
        std::string getDst(const char * path)
        {
            using namespace std;

            string
                diff(path + srcBasePath_.length());
            if (diff[0] == '\\' || diff[0] == '/')
                {
                    diff = diff.substr(1);
                }

            string
                rv(dstBasePath_);
            if (rv[rv.length() - 1] != '\\'
                && rv[rv.length() - 1] != '/')
                {
                    rv += Cpt_PATH_SEPARATOR;
                }
            rv += diff;

            return rv;
        }
    };



    int copy(const char * dstPath,
             const char * srcPath,
             bool         includeSrcDir)
    {
        Copier
            copier(dstPath,
                   srcPath,
                   includeSrcDir);

        if (copier.result() == CPT_CP_OK)
            {
                traverse(srcPath,
                         &copier);
            }

        return copier.result();
    }



    IFileVisitor::~IFileVisitor()
    {
        ;
    }


    DIRSentry::DIRSentry(DIR * d)
        : d_(d)
    {
        ;
    }


    DIRSentry::~DIRSentry()
    {
        int
            result;

        Cpt_EINTR_RETRY(result, closedir(d_));
    }



    bool traverse_(const char   * path,
                   IFileVisitor * visitor)
    {
        bool
            goOn = true;

        DIR
            * d;
        Cpt_EINTR_RETRY_PTR(d, opendir(path));
        
        dirent
            * dir;

        if (d != NULL)
            {
                {
                    DIRSentry
                        dirSentry(d);

                    goOn = visitor->visitDirPre(path);

                    while (goOn)
                        {
                            Cpt_EINTR_RETRY_PTR(dir, readdir(d));

                            if (dir == NULL)
                                break;

                            if (strcmp(".", dir->d_name) == 0
                                || strcmp("..", dir->d_name) == 0)
                                continue;

                            std::string
                                child(path);
                            child += DIR_SEPARATOR;
                            child += dir->d_name;
                        
                            goOn = traverse_(child.c_str(),
                                             visitor);
                        }
                    // no need to call closedir - done by dirSentry
                }

                goOn = visitor->visitDirPost(path);
            }
        else
            {
                goOn = visitor->visitFile(path);
            }

        return goOn;
    }



    void traverse(const char   * path,
                  IFileVisitor * visitor)
    {
        traverse_(path,
                  visitor);
    }



    void pathappend(std::string & path,
                    const char  * fragment)
    {
        // TODO platform specific
        if (path[path.length()-1] != '\\')
            {
                path += '\\';
            }
        path += fragment;
    }



    FileDescSentry::FileDescSentry(int * fd)
        : fileDesc_(fd)
    {
        ;
    }
    
    
    void FileDescSentry::release()
    {
        fileDesc_ = NULL;
    }
    
    
    FileDescSentry::~FileDescSentry()
    {
        if (fileDesc_ != NULL
            && *fileDesc_ != -1)
            {
                int
                    result;

                Cpt_EINTR_RETRY(result,
                                close(*fileDesc_));
            }
    }
    
    
    FileSentry::FileSentry(FILE * file)
    : file_( file )
    {
		;
    }
	
	
	void FileSentry::release()
	{
		file_ = NULL; 
	}

	FileSentry::~FileSentry()
	{
		if ( file_ != NULL )
		{
			int result;
			
			Cpt_EINTR_RETRY(result, 
							fclose(file)); 
		}
	}

}
