sed -i '9d' scratch/*.cc
sed -i "8a #define Alpha\t\t\"$1\"" scratch/*.cc
echo -------------------------------- Alpha Value is set to $1 --------------------------------
