# rm src/ndnSIM/model/cs/content-store-impl.h
# cp src/ndnSIM/model/cs/content-store-impl.h.mcd src/ndnSIM/model/cs/content-store-impl.h

# echo "-------------------------------- test the BestRoute-MCD strategy! --------------------------------"
# ./waf --run ndn-bestroute-MCD

rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.cachetag src/ndnSIM/model/cs/content-store-impl.h

echo "-------------------------------- test the BestRoute-Probcache strategy! --------------------------------"
./waf --run ndn-bestroute-Probcache

# echo "-------------------------------- test the BestRoute-MBP strategy! --------------------------------"
# ./waf --run ndn-bestroute-MBP

rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.lcu src/ndnSIM/model/cs/content-store-impl.h

# echo "-------------------------------- test the BestRoute-Prob0.5 strategy! --------------------------------"
# ./waf --run ndn-bestroute-Prob0.5

echo "-------------------------------- test the BestRoute-LCD strategy! --------------------------------"
./waf --run ndn-bestroute-LCD

echo "-------------------------------- test the BestRoute-LCE strategy! --------------------------------"
./waf --run ndn-bestroute-LCE
