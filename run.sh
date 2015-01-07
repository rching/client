echo "Savepath: " $1 "   monitor meta path: "$2 "    uuid: "$3
rm -rf $1
rm -rf $2
mkdir -p $1
mkdir -p $2
./ihog $1 $2 $3 &
./client -s $1 -m $2
