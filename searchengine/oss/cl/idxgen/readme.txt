

  Index Generator Utility (using clucene, natively)
  =================================================

  Not a symbian project, meant to be used on a UNIX type environment
  (can be cygwin too). Goal: to be able to generate indexes out of
  corpus files in a fast execution environment.

  
  Requires only clucene which is included in this directory as a
  source tarball (as well as "make" and "g++").

  Installation
  ------------

    1 untar clucene source tarball (Download it from http://sourceforge.net/projects/clucene/ )

    2 "configure", "make", "make test" the clucene stuff in the
      directory that was created in the previous step

    3 go to idxgen directory, and "make" it



  Usage
  -----

    The executable does not accept parameters: you just set the values
    programmatically, rebuild and run it.

    It takes corpus from the corpus subdirectory. It can take two
    types of corpus: file (one line is one short file), and POI (one
    line is one record of POI data).

    It produces the index in the idx directory.

    It produces the snapshots in the snapshots directory.

    There is a lot of diagnostical printf statement, some of them may
    not be commented out - see to them.

