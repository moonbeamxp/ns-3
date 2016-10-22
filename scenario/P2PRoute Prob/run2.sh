./init.sh

# Run BestRoute Stratagy

./cs.sh 100
./bestroute.sh

./cs.sh 120
./bestroute.sh

./cs.sh 140
./bestroute.sh

#./cs.sh 160
#./bestroute.sh

./cs.sh 180
./bestroute.sh

./cs.sh 200
./bestroute.sh

./cs.sh 220
./bestroute.sh

# Run P2PRoute Stratagy

./cs.sh 250
./freshtime.sh 2.5
./p2proute.sh

./cs.sh 300
./freshtime.sh 3.0
./p2proute.sh

./cs.sh 350
./freshtime.sh 3.5
./p2proute.sh

#./cs.sh 400
#./freshtime.sh 4.0
#./p2proute.sh

./cs.sh 450
./freshtime.sh 4.5
./p2proute.sh

./cs.sh 500
./freshtime.sh 5.0
./p2proute.sh

./cs.sh 550
./freshtime.sh 5.5
./p2proute.sh
