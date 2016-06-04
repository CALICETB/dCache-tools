# README file for dCache-tools package

This package has been created by E.Brianne, DESY Hamburg during the Summer 2015.
dCache-tools needs dcap/gfal and grid-tools (EMI) to be installed (lcg-tools).
This packages uploads directly to the needed folder on the dCache useful during Testbeams.

## Requirements
* Grid tools (gfal-utils, voms proxy)
* g++ 4.7 minimum

## Installation

Get the git repository 
<pre>
git clone https://github.com/CALICETB/dCache-tools.git
</pre>
Modify the script start_copy.sh with the needed variables.
Start the copy to the dCache 
<pre>
./start_copy.sh
tail -f copy_log.txt
</pre>
