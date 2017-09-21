sed -i '10d' scratch/*.cc
sed -i "9a #define Rate\t\t\"$1\"" scratch/*.cc
echo -------------------------------- Request Rate is set to $1 --------------------------------
