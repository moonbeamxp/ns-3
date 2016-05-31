
rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.mcd src/ndnSIM/model/cs/content-store-impl.h

echo "-------------------------------- test the MCD strategy! --------------------------------"
./waf --run ndn-cache-mcd >>result/cs-replace-MCD-$1.txt

rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.cachetag src/ndnSIM/model/cs/content-store-impl.h

echo "-------------------------------- test the Probcache strategy! --------------------------------"
./waf --run ndn-cache-probcache >>result/cs-replace-Probcache-$1.txt

echo "-------------------------------- test the Popcache strategy! --------------------------------"
./waf --run ndn-cache-popcache >>result/cs-replace-Popcache-$1.txt

echo "-------------------------------- test the Popcachemid strategy! --------------------------------"
./waf --run ndn-cache-popcachemid >>result/cs-replace-Popcachemid-$1.txt

echo "-------------------------------- test the PopcacheOnce strategy! --------------------------------"
./waf --run ndn-cache-popcacheonce >>result/cs-replace-Popcacheonce-$1.txt

echo "-------------------------------- test the PopcacheRevise strategy! --------------------------------"
./waf --run ndn-cache-popcacherevise >>result/cs-replace-Popcacherevise-$1.txt

rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.ori src/ndnSIM/model/cs/content-store-impl.h

echo "-------------------------------- test the LCE strategy! --------------------------------"
./waf --run ndn-cache-lce >>result/cs-replace-LCE-$1.txt

echo "-------------------------------- test the LCD strategy! --------------------------------"
./waf --run ndn-cache-lcd >>result/cs-replace-LCD-$1.txt

echo "-------------------------------- test the Prob 0.3 strategy! --------------------------------"
./waf --run ndn-cache-prob0.3 >>result/cs-replace-Prob0.3-$1.txt

echo "-------------------------------- test the Prob 0.7 strategy! --------------------------------"
./waf --run ndn-cache-prob0.7 >>result/cs-replace-Prob0.7-$1.txt

