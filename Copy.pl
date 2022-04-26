#! /usr/bin/perl

use Cwd;
$maindir = getcwd();

$sourcedir = "/Users/yjkim/workspace/git/kebi-22/LAMPS-HighEnergy";

system "cp $sourcedir/geant4/TB22K* .";
system "cp $sourcedir/macros_tpc/run_TB22Kmc.g4sim .";
#system "cp -r /Users/yjkim/workspace/git/kebi-22/source/* .";
system "cp $sourcedir/macros_tpc/kbpar_TB22K*.conf .";
