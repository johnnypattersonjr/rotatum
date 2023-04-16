echo Changing to %5 ...
cd %5
echo Generating %3 and %4 with prefix %2.
%1 -o %3 %4 --defines -p %2 --header=%6
