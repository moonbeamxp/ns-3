
rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.cachetag src/ndnSIM/model/cs/content-store-impl.h

echo "-------------------------------- test the Popcachestat strategy! --------------------------------"
./waf --run ndn-cache-popcachestat

echo "-------------------------------- test the Popcachestat-with-Lb strategy! --------------------------------"
./waf --run ndn-cache-popcachestat-with-lb

echo "-------------------------------- test the Popcachestat-with-Fifo strategy! --------------------------------"
./waf --run ndn-cache-popcachestat-with-fifo

echo "-------------------------------- test the Popcachestat-with-Random strategy! --------------------------------"
./waf --run ndn-cache-popcachestat-with-random

echo "-------------------------------- test the Popcachestat-with-Lfu strategy! --------------------------------"
./waf --run ndn-cache-popcachestat-with-lfu

rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.ori src/ndnSIM/model/cs/content-store-impl.h

