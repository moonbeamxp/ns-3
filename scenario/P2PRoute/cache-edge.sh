rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.ori src/ndnSIM/model/cs/content-store-impl.h

echo "-------------------------------- test the P2PRoute strategy! --------------------------------"
./waf --run ndn-p2proute >>result/cs-replace-P2PRoute-$1.txt

echo "-------------------------------- test the BestRoute-Edge strategy! --------------------------------"
./waf --run ndn-bestroute-edge >>result/cs-replace-BestRoute-Edge-$1.txt
