sed -i '8d' scratch/*.cc
sed -i "7a #define Capacity\t\"$1\"" scratch/*.cc
echo -------------------------------- Contents Capacity is set to $1 --------------------------------
