#!/usr/bin/perl -w

if (!defined($ARGV[0]) || $ARGV[0] < 1){
    print "Invalid number of variables $#ARGV\n";
    exit 1;
}

my $numVars = $ARGV[0];
my $numTerms = 2**$numVars;
#print "NumVars: $numVars, NumTerms: $numTerms\n";
print "$numVars\n";

my $outF2;
if ($#ARGV >= 1){
    $outF2 = $ARGV[1];
    open OUT2, ">", $outF2.".csv";
    my $z = 'A';
    foreach $i(0..$numVars-1){
        print OUT2 $z++.",";
    }
    print OUT2 ",F0\n";
}

my $n = 0;
my $out="";
foreach $i(0..$numTerms-1){
    my $r = int(rand(2));
    if ($r == 1){
        $n++;
        $r = int(rand(10));
        my $val = '1';
        if ($r == 0){
            $val = 'd';
        }
        # print this term
        my $bin = makebin($i,$numVars);
        $out = $out.$bin." $val\n";
        if (defined($outF2)){
            $bin =~ s/(.)/$1,/g;
            print OUT2 $bin.",".($val eq 'd'?'X':'1')."\n";
        }
    } else {
        # $out = $out.makebin($i,$numVars)." 0\n";
        if (defined($outF2)){
            my $bin = makebin($i,$numVars);
            $bin =~ s/(.)/$1,/g;
            print OUT2 $bin.",0\n";
        }
    }
}

print "$n\n$out";

sub makebin {
    my $val = shift;
    my $numVars = shift;
    my $i = 2**($numVars);
    my $str="";

    while($i > 0){
        $i = int($i/2);
        #print "val:$val numVars:$numVars i:$i str:$str\n";
        if ($i != 0){
            if ($val >= $i){
                $val -= $i;
                $str = $str.'1';
            } else {
                $str = $str.'0';
            }
        }
    }
    return $str;
}
