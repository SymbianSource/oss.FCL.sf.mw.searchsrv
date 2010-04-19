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
#ifndef ITK_ITKIMPL_H_
#define ITK_ITKIMPL_H_

#include <string>

#include "cpixfstools.h"

namespace Itk
{
    namespace Impl
    {


        /**
         * Tells if it could open the file.
         */
        bool FileExists(const char * path);


        /**
         *  Redirects the standard input, if necessary
         */
        class InputRedirector
        {
        private:
            int    duplicatedStdInFD_;
            int    inFileFD_;

        public:
            InputRedirector(const std::string & defFilesBasePath);
            ~InputRedirector();
        };



        /**
         *  Redirects the standard output, if necessary
         */
        class OutputRedirector
        {
        private:
            int       duplicatedStdOutFD_;
            int       outFileFD_;
            bool      stdErr_;
            TestMgr * testMgr_;

        public:
            /**
             * Creates an output redirector instance. The same class
             * is used for redirecting output and error, but there are
             * slight differences in how they work. See parameter
             * list.
             *
             * @param defFilesBasePath the path of the base of the
             * test definition files
             *
             * @param testMgr must NOT be null for stdout redirection
             * and must be NULL for redirecting stderr.
             */
            OutputRedirector(const std::string  & defFilesBasePath,
                             TestMgr            * testMgr);
            ~OutputRedirector();
        };


        /**
         * Evaluates the results of an IO capturing session based on the
         * defFileBasePath argument (ctor) and what it can find on the
         * filesystem around those paths.
         */
        class IOCaptureEvaluator
        {
        private:
            //
            // private members
            //
            const std::string    defFileBasePath_;
            TestMgr            * testMgr_;
            const std::string  & lenience_;


        public:
            //
            // public operators
            //
            void evaluate();


            //
            // lifetime mgmt
            //
            IOCaptureEvaluator(const std::string  & defFileBasePath,
                               TestMgr            * testMgr,
                               const std::string  & lenience);

        private:
            //
            // private methods
            //
            void copyContent(const char * dstPath,
                             const char * srcPath);
            void compareContent(const char * expPath,
                                const char * resPath);
            void evaluate(bool stdErr);
        };


        /**
         * Only requirement for testFunc is that it should support the
         * following syntax: testFunc(testMgr);
         **/
        template<typename TESTFUNC>
        void EvaluateIOCapture(const std::string  & defFilesBaseName,
                               TestMgr            * testMgr,
                               TESTFUNC             testFunc,
                               const std::string  & lenience)
        {
            using namespace std;
            using namespace Itk::Impl;

            string
                defFilesBasePath;
            testMgr->ioTestCasesDir(defFilesBasePath);
            // OBS defFilesBasePath += defFilesBaseName;
            Cpt::pathappend(defFilesBasePath,
                            defFilesBaseName.c_str());

            try
                {
                    InputRedirector
                        stdInRedirector(defFilesBasePath);
                    OutputRedirector
                        stdOutRedirector(defFilesBasePath,
                                         testMgr), // std out
                        stdErrRedirector(defFilesBasePath,
                                         NULL); // std err
                
                    testFunc(testMgr);
                }
            catch (IOCaptureExc & exc)
                {
                    testMgr->ioCaptureError("(?)",
                                            exc.what());
                }
            
            IOCaptureEvaluator
                evaluator(defFilesBasePath,
                          testMgr,
                          lenience);
            evaluator.evaluate();
        }


    }
}

#endif ITK_ITKIMPL_H_
