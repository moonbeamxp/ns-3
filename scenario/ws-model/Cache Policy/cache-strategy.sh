
rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.cachetag src/ndnSIM/model/cs/content-store-impl.h

echo "-------------------------------- test the PopcacheRevise strategy! --------------------------------"
./waf --run ndn-cache-popcacherevise >>result/cs-replace-Popcacherevise-$1.txt

echo "-------------------------------- test the PopcacheRevise-with-Lb strategy! --------------------------------"
./waf --run ndn-cache-popcacherevise-with-lb >>result/cs-replace-Popcacherevise-with-Lb-$1.txt

echo "-------------------------------- test the Popcachestat strategy! --------------------------------"
./waf --run ndn-cache-popcachestat >>result/cs-replace-Popcachestat-$1.txt

echo "-------------------------------- test the Popcachestat-with-Lb strategy! --------------------------------"
./waf --run ndn-cache-popcachestat-with-lb >>result/cs-replace-Popcachestat-with-Lb-$1.txt

rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.ori src/ndnSIM/model/cs/content-store-impl.h

echo "-------------------------------- test the LCE strategy! --------------------------------"
./waf --run ndn-cache-lce >>result/cs-replace-LCE-$1.txt

