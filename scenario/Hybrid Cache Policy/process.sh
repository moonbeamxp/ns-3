sed -i '60d' scratch/*.cc
sed -i "59a \ \ int times=$1;" scratch/process.cc
echo -------------------------------- Running times is set to $1 --------------------------------
