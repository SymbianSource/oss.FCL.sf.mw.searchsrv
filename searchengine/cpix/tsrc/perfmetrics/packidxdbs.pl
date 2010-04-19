#
# Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
# All rights reserved.
# This component and the accompanying materials are made available
# under the terms of "Eclipse Public License v1.0"
# which accompanies this distribution, and is available
# at the URL "http://www.eclipse.org/legal/epl-v10.html".
#
# Initial Contributors:
# Nokia Corporation - initial contribution.
#
# Contributors:
#
# Description: 
#
#!/usr/bin/perl -w

# This Perl program
#
#  o parses a directory
#  o creates a package definition file for whatever it finds there
#  o creates the package
#  o and signs it
#
# NOTE: this program does not take arguments, you must set the
# variables below.


############################################
#
#   Configuration Variables
#

# Input directory (use path separator at the end)
my $SrcDir = "m:/epoc32/winscw/c/Data/perfmetrics/000/";

# package definition file path
my $PkgDefFile = "idxdb.pkg";

# destination installation directory (use path separator at the end)
my $DstDir = "c:/Data/indexing/indexdb/file/";

# Certificate to sign with
my $CertificateFile = "m:/rd.crt";

# Key to sign with
my $KeyFile = "m:/rd.key";




############################################
#
#   Global Variables (don't touch)
#

# List of files to generate package for
my @SrcFiles = ();


############################################
#
#   Body of program
#


# Lists (not recursively) file names from source directory ($srcDir)
# to %srcFiles
sub readSrcFiles()
{
    local $globbedSrcDir = $SrcDir . "*";
    local @srcFiles = glob $globbedSrcDir;
    foreach $file (@srcFiles) {
        push @SrcFiles, substr($file, length($SrcDir))
    }

    foreach $file (@SrcFiles) {
        print "TODO ### $file\n";
    }
}


sub createPkgDefFile()
{
    open(PKGDEF, ">$PkgDefFile") or die "Can't open $PkgDefFile for writing: $!\n";

    print PKGDEF <<EOS;
; Installation file for test corpus
;
; This is an auto-generated PKG file by Carbide.
; This file uses variables specific to Carbide builds that will not work
; on command-line builds. If you want to use this generated PKG file from the
; command-line tools you will need to modify the variables with the appropriate
; values: $(EPOCROOT), $(PLATFORM), $(TARGET)
;
;Languages
&EN

;
; UID is the app\'s UID
;
#{"testcorpus"},(0xEC2D35B4),1,0,0

;Localised Vendor name
%{"Vendor-EN"}

;Unique Vendor name
:"Vendor"

; Supports Series 60 v 3.0
[0x101F7961], 0, 0, 0, {"S60ProductID"}

; Files to install
; Symbols set up for the source location are Carbide.c++ specific symbols
EOS

    foreach $file (@SrcFiles) {
        print PKGDEF "\"$SrcDir$file\"-\"$DstDir$file\"\n";
    }

    close(PKGDEF);
}


sub makeAndSignSis()
{
    $sisFile = $PkgDefFile;
    $sisFile =~ s/.pkg/.sis/;

    system ("makesis", "$PkgDefFile", "$sisFile");

    $sisxFile = $PkgDefFile;
    $sisxFile =~ s/.pkg/.sisx/;

    system ("signsis", "-v", "$sisFile", "$sisxFile", "$CertificateFile", "$KeyFile");
}

############################################
#
#   Main
#
readSrcFiles();
createPkgDefFile();
makeAndSignSis();

