sed -i '11d' scratch/*.cc
sed -i "10a #define FreshTime\t$1" scratch/*.cc
echo -------------------------------- FreshTime is set to $1 --------------------------------
