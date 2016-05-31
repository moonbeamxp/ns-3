./init.sh

# Run BestRoute Stratagy

./cs.sh 160

./alpha.sh 0.7
./bestroute.sh

./alpha.sh 0.8
./bestroute.sh

./alpha.sh 0.9
./bestroute.sh

./alpha.sh 1.0
./bestroute.sh

./alpha.sh 1.1
./bestroute.sh

./alpha.sh 1.2
./bestroute.sh

./alpha.sh 1.3
./bestroute.sh

# Run P2PRoute Stratagy

./cs.sh 400

./alpha.sh 0.7
./p2proute.sh

./alpha.sh 0.8
./p2proute.sh

./alpha.sh 0.9
./p2proute.sh

./alpha.sh 1.0
./p2proute.sh

./alpha.sh 1.1
./p2proute.sh

./alpha.sh 1.2
./p2proute.sh

./alpha.sh 1.3
./p2proute.sh
