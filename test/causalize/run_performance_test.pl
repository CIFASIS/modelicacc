#!/usr/bin/perl

use strict;

use constant REPETITION => 2;
use constant MAX_N => 200;
use constant STEP => 100;

foreach my $original_filename ("OneDHeatTransferTI_FD.mo", "OneDHeatTransferTI_FD_loop.mo") {

	my $simple_strategy_times = {};
	my $tarjan_strategy_times = {};
	my $full_strategy_times = {};

	for (my $h = 0; $h < REPETITION; $h++) {
		for (my $i = 100; $i <= MAX_N; $i = $i + STEP) {
			my ($original_fh, $clone_fh);
					
			mkdir "/tmp/$i";
			
			my $clone_filename = "/tmp/$i/$original_filename";
			
			open($original_fh, "<", $original_filename)
		       or die "cannot open > $original_filename: $!";
			
			open($clone_fh, ">", $clone_filename)
		       or die "cannot open > $clone_filename: $!";

		    # Modify the cloned file with the correct N
		    while (my $line = <$original_fh>) {
	    		if ($line =~ /constant Integer N = (\d+);/) {
		    		$line = "  constant Integer N = $i;\n";
		    	}
		    	print $clone_fh $line;
		    }
		    
		    # Run the perfomance test
		    my $output = `performance_test $clone_filename 2>&1`;

		    print "Output for repetition $h and N $i:\n$output\n"; 
		    
		    # Get the times from the output
		    my ($sst, $tst, $fst);
		    if ($output =~ /Simple strategy: (\d\.\d+)	Tarjan strategy: (\d\.\d+)	Full Causalization strategy: (\d\.\d+)/) {
		    	$sst = $1;
		    	$tst = $2;
		    	$fst = $3;
		    } else {
		    	die "Unexpected output after processing file $clone_filename";
		    }

		    $simple_strategy_times->{$h}->{$i} = $sst;
		    $tarjan_strategy_times->{$h}->{$i} = $tst;
		    $full_strategy_times->{$h}->{$i} = $fst;
		    
		    close $clone_fh
		     	or die "cannot close $clone_filename: $!";
			close $original_fh
			    or die "cannot close $original_filename: $!";
		}
	}

	my $count = MAX_N / STEP;

	my @spiltted_original_name = split(/\./, $original_filename, 2);

	my $sst_data_fh;
	my $sst_data_filename = $spiltted_original_name[0] . "_simple.data";
	open($sst_data_fh, ">", $sst_data_filename)
	   or die "cannot open > $sst_data_filename: $!";

	print $sst_data_fh "0 0\n";

	for (my $i = 100; $i <= MAX_N; $i = $i + STEP) {
		my $sst_sum = 0;
		for (my $h = 0; $h < REPETITION; $h++) {
			my $time = $simple_strategy_times->{$h}->{$i};
			$sst_sum += $time;
		}
		my $avg = $sst_sum / $count;
		print $sst_data_fh "$i $avg\n";
	}

	my $tst_data_fh;
	my $tst_data_filename = $spiltted_original_name[0] . "_tarjan.data";
	open($tst_data_fh, ">", $tst_data_filename)
	   or die "cannot open > $tst_data_filename: $!";

	print $tst_data_fh "0 0\n";

	for (my $i = 100; $i <= MAX_N; $i = $i + STEP) {
		my $tst_sum = 0;
		for (my $h = 0; $h < REPETITION; $h++) {
			my $time = $tarjan_strategy_times->{$h}->{$i};
			$tst_sum += $time;
		}
		my $avg = $tst_sum / $count;
		print $tst_data_fh "$i $avg\n";
	}

	my $fst_data_fh;
	my $fst_data_filename = $spiltted_original_name[0] . "_full.data";
	open($fst_data_fh, ">", $fst_data_filename)
	   or die "cannot open > $fst_data_filename: $!";

	print $fst_data_fh "0 0\n";

	for (my $i = 100; $i <= MAX_N; $i = $i + STEP) {
		my $fst_sum = 0;
		for (my $h = 0; $h < REPETITION; $h++) {
			my $time = $full_strategy_times->{$h}->{$i};
			$fst_sum += $time;
		}
		my $avg = $fst_sum / $count;
		print $fst_data_fh "$i $avg\n";

	}
}


