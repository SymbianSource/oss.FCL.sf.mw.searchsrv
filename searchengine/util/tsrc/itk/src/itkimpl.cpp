/*
* Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
* This component and the accompanying materials are made available
* under the terms of "Eclipse Public License v1.0"
* which accompanies this distribution, and is available
* at the URL "http://www.eclipse.org/legal/epl-v10.html".
*
* Initial Contributors:
* Nokia Corporation - initial contribution.
*
* Contributors:
*
* Description: 
*
*/

#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <fstream>

#include "cpixfstools.h"

#include "itktesters.h"
#include "itktestmgr.h"
#include "itkimpl.h"

namespace
{
    // suffix for 'in' (input to be redirected) files
    const char IN_SFX[] = "_in.txt";

    // suffixes for 'res' / 'exp' (RESult and EXPected) files for
    // stdandard OUTput and ERRor.
    const char RES_OUT_SFX[] = "_res_out.txt";
    const char RES_ERR_SFX[] = "_res_err.txt";
    const char EXP_OUT_SFX[] = "_exp_out.txt";
    const char EXP_ERR_SFX[] = "_exp_err.txt";
}



namespace Itk
{
    namespace Impl
    {


        /****
         * InputRedirector
         */
        InputRedirector::InputRedirector(const std::string & defFilesBasePath)
            : duplicatedStdInFD_(-1),
              inFileFD_(-1)
        {
            using namespace std;
            using namespace Cpt;
            
            string
                inFilePath = defFilesBasePath;
            inFilePath += IN_SFX;
            
            if (isreadable(inFilePath.c_str()))
                {
                    int
                        res;

                    duplicatedStdInFD_ = dup(STDIN_FILENO);
                    if (duplicatedStdInFD_ == -1)
                        {
                            throw IOCaptureExc("Can't dup(stdin)");
                        }

                    Cpt_EINTR_RETRY(inFileFD_,
                                    open(inFilePath.c_str(),O_RDONLY));
                    if (inFileFD_ == -1)
                        {
                            Cpt_EINTR_RETRY(res,close(duplicatedStdInFD_));
                            throw IOCaptureExc(inFilePath.c_str());
                        }
                    int
                        newStdIn = dup2(inFileFD_,
                                        STDIN_FILENO);
                    if (newStdIn == -1)
                        {
                            Cpt_EINTR_RETRY(res,close(inFileFD_));
                            Cpt_EINTR_RETRY(res,close(duplicatedStdInFD_));
                            throw IOCaptureExc("Can't dup2(infile,stdin)");
                        }
                }
        }


        InputRedirector::~InputRedirector()
        {
            if (duplicatedStdInFD_ != -1)
                {
                    int
                        fd = dup2(duplicatedStdInFD_,
                                  STDIN_FILENO);
                    // we can't possibly do anything sensible about these
                    // failures here, but they must not go unnoticed
                    assert(fd != -1);

                    int
                        res;
                    Cpt_EINTR_RETRY(res,close(inFileFD_));
                    Cpt_EINTR_RETRY(res,close(duplicatedStdInFD_));
                }
        }



        /****
         * OutputRedirector
         */
        OutputRedirector::OutputRedirector(const std::string & defFilesBasePath,
                                           TestMgr           * testMgr)
            : duplicatedStdOutFD_(-1),
              outFileFD_(-1),
              stdErr_(testMgr == NULL ? true : false),
              testMgr_(testMgr)
        {
            // flushing possible STD IO stream buffers before swapping
            int
                res = fflush(stdErr_ ? stderr : stdout);
            assert(res != EOF);

            using namespace std;
            
            string
                outFilePath = defFilesBasePath;
            outFilePath += (stdErr_ ? RES_ERR_SFX : RES_OUT_SFX);
            
            int
                stdFD = stdErr_ ? STDERR_FILENO : STDOUT_FILENO;

            duplicatedStdOutFD_ = dup(stdFD);
            if (duplicatedStdOutFD_ == -1)
                {
                    const char
                        * msg = stdErr_ ? "Can't dup(stdErr)"
                        : "Can't dup(stdout)";

                    throw IOCaptureExc(msg);
                }
            Cpt_EINTR_RETRY(outFileFD_,
                            open(outFilePath.c_str(),
                                 O_CREAT | O_TRUNC | O_WRONLY,
                                 0666));
            if (outFileFD_ == -1)
                {
                    Cpt_EINTR_RETRY(res,close(duplicatedStdOutFD_));
                    throw IOCaptureExc(outFilePath.c_str());
                }
            int
                newStdOut = dup2(outFileFD_,
                                 stdFD);
            if (newStdOut == -1)
                {
                    Cpt_EINTR_RETRY(res,close(outFileFD_));
                    Cpt_EINTR_RETRY(res,close(duplicatedStdOutFD_));
                    throw IOCaptureExc("Can't dup2(outfile,stdout)");
                }
            
            if (!stdErr_)
                {
                    testMgr_->setDbgConsoleFd(duplicatedStdOutFD_);
                }
        }


        OutputRedirector::~OutputRedirector()
        {
            // flushing possible STD IO stream buffers before swapping
            int
                res = fflush(stdErr_ ? stderr : stdout);
            assert(res != EOF);

            int
                stdFD = stdErr_ ? STDERR_FILENO : STDOUT_FILENO;
            
            if (!stdErr_)
                {
                    testMgr_->setDbgConsoleFd(stdFD);
                }

            int
                fd = dup2(duplicatedStdOutFD_,
                          stdFD);
            // we can't possibly do anything sensible about these
            // failures here, but they must not go unnoticed
            assert(fd != -1);

            Cpt_EINTR_RETRY(res,close(outFileFD_));
            Cpt_EINTR_RETRY(res,close(duplicatedStdOutFD_));
        }


        /****
         * IOCaptureEvaluator
         */
        void IOCaptureEvaluator::evaluate()
        {
            evaluate(false); // std out
            evaluate(true);  // std err
        }

        
        IOCaptureEvaluator
        ::IOCaptureEvaluator(const std::string  & defFileBasePath,
                             TestMgr            * testMgr,
                             const std::string  & lenience)
            : defFileBasePath_(defFileBasePath),
              testMgr_(testMgr),
              lenience_(lenience)
        {
            ;
        }
        
        void IOCaptureEvaluator::copyContent(const char * dstPath,
                                             const char * srcPath)
        {
            int
                result = Cpt::copyFile(dstPath,
                                       srcPath);

            switch (result)
                {
                case Cpt::CPT_CPF_OK:
                    testMgr_->ioCaptureDefined(dstPath,
                                               "IO test case defined.");
                    break;
                case Cpt::CPT_CPF_DST_OPEN_ERROR:
                    testMgr_->ioCaptureError(dstPath,
                                             "Could not open for writing");
                    break;
                case Cpt::CPT_CPF_SRC_OPEN_ERROR:
                    testMgr_->ioCaptureError(srcPath,
                                             "Could not open for reading");
                    break;
                case Cpt::CPT_CPF_DST_WRITE_ERROR:
                    testMgr_->ioCaptureError(dstPath,
                                             "Writing failed");
                    break;
                case Cpt::CPT_CPF_SRC_READ_ERROR:
                    testMgr_->ioCaptureError(srcPath,
                                             "Reading failed");
                    break;
                };
        }


        void IOCaptureEvaluator::compareContent(const char * expPath,
                                                const char * resPath)
        {
            using namespace std;

            FILE*
                expFs;
            expFs = fopen(expPath, "r");
            if (!expFs)
                {
                    testMgr_->ioCaptureError(expPath,
                                             "Could not open for reading");
                    return;
                }
			Cpt::FileSentry expFsSentry( expFs ); 


            FILE*
                resFs;
            resFs = fopen(resPath, "r");
            if (!resFs)
                {
                    testMgr_->ioCaptureError(resPath,
                                             "Could not open for reading");
                    return;
                }
			Cpt::FileSentry resFsSentry( resFs ); 

            size_t
                firstDifferingLine = -1,
                currentLine = 1;
            string
                expLine,
                resLine;
            bool
                lenient = lenience_.length() > 0;

            while (firstDifferingLine == -1 && Cpt::fgetline(expFs, expLine))
                {
                    if (!Cpt::fgetline(resFs, resLine))
                        {
                            firstDifferingLine = currentLine;
                        }
                    else if (expLine != resLine)
                        {
                            if (!(lenient 
                                  && expLine.find(lenience_) != string::npos))
                                {
                                    firstDifferingLine = currentLine;
                                }
                        }

                    ++currentLine;
                }

            if (firstDifferingLine == -1)
                {
                    // If the contents were identical so far, then we
                    // have terminated the previous loop with
                    // exhausting the entire EXPected content. At this
                    // point, therefore, nothing should be left from
                    // RESult content, either.
                    if (Cpt::fgetline(resFs, resLine))
                        {
                            firstDifferingLine = currentLine;
                        }
                }

            bool
                succeeded = firstDifferingLine == -1;

            testMgr_->expecting(succeeded,
                                "(IO Capture)",
                                expPath,
                                succeeded ? 0 : firstDifferingLine,
                                "IO Capture test.");
        }
        

        void IOCaptureEvaluator::evaluate(bool stdErr)
        {
            using namespace std;
            using namespace Cpt;

            string
                resPath = defFileBasePath_,
                expPath = defFileBasePath_;

            resPath += (stdErr ? RES_ERR_SFX : RES_OUT_SFX);
            expPath += (stdErr ? EXP_ERR_SFX : EXP_OUT_SFX);

            bool
                redirectOnly = (lenience_ == SuiteTester::REDIRECT_ONLY);
            
            if (!isreadable(resPath.c_str()))
                {
                    testMgr_->ioCaptureError(resPath.c_str(),
                                             "Cannot open RESult file.");
                }
            else if (isreadable(expPath.c_str()))
                {
                    if (!redirectOnly)
                        {
                            compareContent(expPath.c_str(),
                                           resPath.c_str());
                        }
                }
            else if (stdErr
                     && !isreadable(expPath.c_str()))
                {
                    bool
                        succeeded = filesize(resPath.c_str()) == 0;

                    // if we are doing std err, and if there is no
                    // EXPected output then we fail or succeed by
                    // looking already at the size of the RESulting
                    // error
                    testMgr_->expecting(succeeded,
                                        "(IO Capture)",
                                        expPath.c_str(),
                                        0,
                                        "IO Capture test.");
                }
            else
                {
                    if (!redirectOnly)
                        {
                            copyContent(expPath.c_str(),
                                        resPath.c_str());
                        }
                }
        }



    }
}
