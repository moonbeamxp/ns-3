
rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.mcd src/ndnSIM/model/cs/content-store-impl.h

echo "test the MCD strategy!"
./waf --run ndn-cache-mcd $1

rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.cachetag src/ndnSIM/model/cs/content-store-impl.h

echo "test the Probcache strategy!"
./waf --run ndn-cache-probcache $1

echo "test the Popcache strategy!"
./waf --run ndn-cache-popcache $1

echo "test the Popcachemid strategy!"
./waf --run ndn-cache-popcachemid $1

echo "test the PopcacheOnce strategy!"
./waf --run ndn-cache-popcacheonce $1

rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.ori src/ndnSIM/model/cs/content-store-impl.h

echo "test the LCE strategy!"
./waf --run ndn-cache-lce $1

echo "test the LCD strategy!"
./waf --run ndn-cache-lcd $1

echo "test the Prob 0.3 strategy!"
./waf --run ndn-cache-prob0.3 $1

echo "test the Prob 0.7 strategy!"
./waf --run ndn-cache-prob0.7 $1

