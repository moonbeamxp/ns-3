
rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.cachetag src/ndnSIM/model/cs/content-store-impl.h

echo "-------------------------------- test the Probcache strategy! --------------------------------"
./waf --run ndn-cache-Probcache >>result/cs-replace-Probcache-$1.txt

# echo "-------------------------------- test the MBP-global strategy! --------------------------------"
# ./waf --run ndn-cache-MBP-global >>result/cs-replace-MBP-global-$1.txt

echo "-------------------------------- test the MBP-local strategy! --------------------------------"
./waf --run ndn-cache-MBP-local >>result/cs-replace-MBP-local-$1.txt

rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.lcu src/ndnSIM/model/cs/content-store-impl.h

echo "-------------------------------- test the LCE strategy! --------------------------------"
./waf --run ndn-cache-LCE >>result/cs-replace-LCE-$1.txt

echo "-------------------------------- test the LCD strategy! --------------------------------"
./waf --run ndn-cache-LCD >>result/cs-replace-LCD-$1.txt

echo "-------------------------------- test the MCD strategy! --------------------------------"
./waf --run ndn-cache-MCD >>result/cs-replace-MCD-$1.txt

echo "-------------------------------- test the Prob 0.5 strategy! --------------------------------"
./waf --run ndn-cache-Prob0.5 >>result/cs-replace-Prob0.5-$1.txt

echo "-------------------------------- test the LCU strategy! --------------------------------"
./waf --run ndn-cache-LCU >>result/cs-replace-LCU-$1.txt

echo "-------------------------------- test the Nocache strategy! --------------------------------"
./waf --run ndn-cache-Nocache >>result/cs-replace-Nocache-$1.txt

