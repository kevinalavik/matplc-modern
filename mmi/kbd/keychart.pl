#!/usr/bin/perl

open DK, "dumpkeys |" or die "can't spawn dumpkeys";

while (<DK>) {
  ($code,$key)=/^keycode\s*(\d+)\s*=\s*(\S+)/ or next;
  if ($key =~ /^.$/) {
    $chars{$key}=$code;
  } else {
    $names{$key}=$code;
  }
}

$col = 0;
foreach ((map {[$_,$chars{$_}]} sort keys %chars),
         (map {[$_,$names{$_}]} sort keys %names)) {
  ($key, $code) = @$_;


  $key = $key . " "x(6-length $key);

  $res = $key . " " . sprintf "%3d",$code;

  $res = $res . " "x(16- (length $res)%16);

  $col += length $res;
  if ($col>80) {
    $col = length $res;
    print "\n";
  } elsif ($col == 80) {
    $col = 0;
    $res =~ s/\s*$/\n/;
  }

  print "$res";
}
print "\n" unless $col == 0;
