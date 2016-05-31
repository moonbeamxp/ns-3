sed -i '7d' scratch/*.cc
sed -i "6a #define CS_Capacity\t\"$1\"" scratch/*.cc
echo -------------------------------- CS Capacity is set to $1 --------------------------------
