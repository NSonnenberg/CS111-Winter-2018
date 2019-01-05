./lab2_list --sync=m --iterations=1000 --threads=1 > lab2b_list.csv
./lab2_list --sync=m --iterations=1000 --threads=2 >> lab2b_list.csv
./lab2_list --sync=m --iterations=1000 --threads=4 >> lab2b_list.csv
./lab2_list --sync=m --iterations=1000 --threads=8 >> lab2b_list.csv
./lab2_list --sync=m --iterations=1000 --threads=12 >> lab2b_list.csv
./lab2_list --sync=m --iterations=1000 --threads=16 >> lab2b_list.csv
./lab2_list --sync=m --iterations=1000 --threads=24 >> lab2b_list.csv

./lab2_list --sync=s --iterations=1000 --threads=1 >> lab2b_list.csv
./lab2_list --sync=s --iterations=1000 --threads=2 >> lab2b_list.csv
./lab2_list --sync=s --iterations=1000 --threads=4 >> lab2b_list.csv
./lab2_list --sync=s --iterations=1000 --threads=8 >> lab2b_list.csv
./lab2_list --sync=s --iterations=1000 --threads=12 >> lab2b_list.csv
./lab2_list --sync=s --iterations=1000 --threads=16 >> lab2b_list.csv
./lab2_list --sync=s --iterations=1000 --threads=24 >> lab2b_list.csv

./lab2_list --sync=m --iterations=1000 --threads=1 > lab2b_list.csv
./lab2_list --sync=m --iterations=1000 --threads=2 >> lab2b_list.csv
./lab2_list --sync=m --iterations=1000 --threads=4 >> lab2b_list.csv
./lab2_list --sync=m --iterations=1000 --threads=8 >> lab2b_list.csv
./lab2_list --sync=m --iterations=1000 --threads=12 >> lab2b_list.csv
./lab2_list --sync=m --iterations=1000 --threads=16 >> lab2b_list.csv
./lab2_list --sync=m --iterations=1000 --threads=24 >> lab2b_list.csv

./lab2_list --yield=id --threads=1 --iterations=1 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=1 --iterations=2 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=1 --iterations=4 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=1 --iterations=8 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=1 --iterations=16 --lists=4 >> lab2b_list.csv

./lab2_list --yield=id --threads=4 --iterations=1 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=4 --iterations=2 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=4 --iterations=4 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=4 --iterations=8 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=4 --iterations=16 --lists=4 >> lab2b_list.csv

./lab2_list --yield=id --threads=8 --iterations=1 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=8 --iterations=2 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=8 --iterations=4 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=8 --iterations=8 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=8 --iterations=16 --lists=4 >> lab2b_list.csv

./lab2_list --yield=id --threads=12 --iterations=1 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=12 --iterations=2 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=12 --iterations=4 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=12 --iterations=8 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=12 --iterations=16 --lists=4 >> lab2b_list.csv

./lab2_list --yield=id --threads=16 --iterations=1 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=16 --iterations=2 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=16 --iterations=4 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=16 --iterations=8 --lists=4 >> lab2b_list.csv
./lab2_list --yield=id --threads=16 --iterations=16 --lists=4 >> lab2b_list.csv

./lab2_list --yield=id --threads=1 --iterations=10 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=1 --iterations=20 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=1 --iterations=40 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=1 --iterations=80 --lists=4 --sync=s >> lab2b_list.csv

./lab2_list --yield=id --threads=4 --iterations=10 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=4 --iterations=20 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=4 --iterations=40 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=4 --iterations=80 --lists=4 --sync=s >> lab2b_list.csv

./lab2_list --yield=id --threads=8 --iterations=10 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=8 --iterations=20 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=8 --iterations=40 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=8 --iterations=80 --lists=4 --sync=s >> lab2b_list.csv

./lab2_list --yield=id --threads=12 --iterations=10 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=12 --iterations=20 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=12 --iterations=40 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=12 --iterations=80 --lists=4 --sync=s >> lab2b_list.csv

./lab2_list --yield=id --threads=16 --iterations=10 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=16 --iterations=20 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=16 --iterations=40 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=16 --iterations=80 --lists=4 --sync=s >> lab2b_list.csv

./lab2_list --yield=id --threads=1 --iterations=10 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=1 --iterations=20 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=1 --iterations=40 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=1 --iterations=80 --lists=4 --sync=m >> lab2b_list.csv

./lab2_list --yield=id --threads=4 --iterations=10 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=4 --iterations=20 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=4 --iterations=40 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=4 --iterations=80 --lists=4 --sync=m >> lab2b_list.csv

./lab2_list --yield=id --threads=8 --iterations=10 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=8 --iterations=20 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=8 --iterations=40 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=8 --iterations=80 --lists=4 --sync=m >> lab2b_list.csv

./lab2_list --yield=id --threads=12 --iterations=10 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=12 --iterations=20 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=12 --iterations=40 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=12 --iterations=80 --lists=4 --sync=m >> lab2b_list.csv

./lab2_list --yield=id --threads=16 --iterations=10 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=16 --iterations=20 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=16 --iterations=40 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=16 --iterations=80 --lists=4 --sync=m >> lab2b_list.csv

./lab2_list --yield=id --threads=1 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=1 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=1 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=1 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv

./lab2_list --yield=id --threads=2 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=2 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=2 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=2 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv

./lab2_list --yield=id --threads=4 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=4 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=4 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=4 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv

./lab2_list --yield=id --threads=8 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=8 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=8 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=8 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv

./lab2_list --yield=id --threads=12 --iterations=1000 --lists=1 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=12 --iterations=1000 --lists=4 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=12 --iterations=1000 --lists=8 --sync=m >> lab2b_list.csv
./lab2_list --yield=id --threads=12 --iterations=1000 --lists=16 --sync=m >> lab2b_list.csv

./lab2_list --yield=id --threads=1 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=1 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=1 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=1 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv

./lab2_list --yield=id --threads=2 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=2 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=2 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=2 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv

./lab2_list --yield=id --threads=4 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=4 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=4 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=4 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv

./lab2_list --yield=id --threads=8 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=8 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=8 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=8 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv

./lab2_list --yield=id --threads=12 --iterations=1000 --lists=1 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=12 --iterations=1000 --lists=4 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=12 --iterations=1000 --lists=8 --sync=s >> lab2b_list.csv
./lab2_list --yield=id --threads=12 --iterations=1000 --lists=16 --sync=s >> lab2b_list.csv

