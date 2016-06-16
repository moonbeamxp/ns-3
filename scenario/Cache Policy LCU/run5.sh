echo Start!

./process.sh 10000
./waf --run=process
./waf --run=process >>Performance.txt
./waf --run=process >>Performance.txt
./waf --run=process >>Performance.txt

./process.sh 100000
./waf --run=process
./waf --run=process >>Performance.txt
./waf --run=process >>Performance.txt
./waf --run=process >>Performance.txt

./process.sh 1000000
./waf --run=process
./waf --run=process >>Performance.txt
./waf --run=process >>Performance.txt
./waf --run=process >>Performance.txt

./process.sh 4000000
./waf --run=process
./waf --run=process >>Performance.txt
./waf --run=process >>Performance.txt
./waf --run=process >>Performance.txt

./process.sh 7000000
./waf --run=process
./waf --run=process >>Performance.txt
./waf --run=process >>Performance.txt
./waf --run=process >>Performance.txt

./process.sh 10000000
./waf --run=process
./waf --run=process >>Performance.txt
./waf --run=process >>Performance.txt
./waf --run=process >>Performance.txt

echo End!
