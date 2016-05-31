./init.sh

rm src/ndnSIM/model/cs/content-store-impl.h
cp src/ndnSIM/model/cs/content-store-impl.h.ori src/ndnSIM/model/cs/content-store-impl.h

echo "-------------------------------- ALL CACHE --------------------------------"
time ./waf --run ndn-bestroute-LCE-allcache

echo "-------------------------------- CORE CACHE --------------------------------"
time ./waf --run ndn-bestroute-LCE-corecache

echo "-------------------------------- EDGE CACHE --------------------------------"
time ./waf --run ndn-bestroute-LCE-edgecache

echo "-------------------------------- NO CACHE --------------------------------"
time ./waf --run ndn-bestroute-LCE
