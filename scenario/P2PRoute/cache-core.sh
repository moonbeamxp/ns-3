rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.ori src/ndnSIM/model/cs/content-store-impl.h

echo "-------------------------------- test the BestRoute-Core strategy! --------------------------------"
./waf --run ndn-bestroute-core >>result/cs-replace-BestRoute-Core-$1.txt
