if (!$ARGV[0]) {
	printf("ERROR: Target (udeb/urel) needs to be specified.\n"); 
	printf("Use either 'makeme udeb' or 'makeme urel'\n"); 
	exit(0); 
}

if (!$ARGV[1]) {
	printf("ERROR: EPOCROOT needs to be specified as a second parameter.\n"); 
	exit(0); 
}


print "Creating package file...\n";
open(SOURCE, "<CPiXSearch.pkg.source"); 
open(TARGET, ">CPiXSearch.pkg"); 

$version = `svnversion`; 
$version =~ s/\d+://;
$version =~ s/M//;

$target = $ARGV[0];
$epocroot = $ARGV[1];


while ($line = <SOURCE>) {
	$line =~ s/\$\(SVN\)/$version/;
	$line =~ s/\$\(TARGET\)/$target/; 
	$line =~ s/\$\(EPOCROOT\)/$epocroot/; 
	print TARGET $line;
}
close(TARGET); 
close(SOURCE); 
print "DONE.\n";
