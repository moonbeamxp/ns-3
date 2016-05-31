rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.ori src/ndnSIM/model/cs/content-store-impl.h

rm src/ndnSIM/model/fw/p2p-route-index.cc
cp src/ndnSIM/model/fw/p2p-route-index.cc.1hop src/ndnSIM/model/fw/p2p-route-index.cc

echo "-------------------------------- test the P2PRoute-Single strategy! --------------------------------"
./waf --run ndn-p2proute-Single

echo "-------------------------------- test the P2PRoute-Prob strategy! --------------------------------"
./waf --run ndn-p2proute-Prob
