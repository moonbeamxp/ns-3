
rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.cachetag src/ndnSIM/model/cs/content-store-impl.h

echo "-------------------------------- test the Probcache strategy! --------------------------------"
./waf --run ndn-cache-Probcache >>result/cs-replace-Probcache-$1.txt

echo "-------------------------------- test the Hybrid strategy! --------------------------------"
./waf --run ndn-cache-HDP >>result/cs-replace-HDP-$1.txt

echo "-------------------------------- test the MBP strategy! --------------------------------"
./waf --run ndn-cache-MBP >>result/cs-replace-MBP-$1.txt

rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.lcu src/ndnSIM/model/cs/content-store-impl.h

echo "-------------------------------- test the LCE strategy! --------------------------------"
./waf --run ndn-cache-LCE >>result/cs-replace-LCE-$1.txt

echo "-------------------------------- test the LCD strategy! --------------------------------"
./waf --run ndn-cache-LCD >>result/cs-replace-LCD-$1.txt

echo "-------------------------------- test the MCD strategy! --------------------------------"
./waf --run ndn-cache-MCD >>result/cs-replace-MCD-$1.txt

echo "-------------------------------- test the Prob 0.3 strategy! --------------------------------"
./waf --run ndn-cache-Prob0.3 >>result/cs-replace-Prob0.3-$1.txt

echo "-------------------------------- test the Prob 0.7 strategy! --------------------------------"
./waf --run ndn-cache-Prob0.7 >>result/cs-replace-Prob0.7-$1.txt

echo "-------------------------------- test the Nocache strategy! --------------------------------"
./waf --run ndn-cache-Nocache >>result/cs-replace-Nocache-$1.txt

