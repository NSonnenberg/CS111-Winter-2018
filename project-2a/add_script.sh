
statement="Running add-yield-none threads="

echo "Running add-none threads=10"
./lab2_add --threads=10 --iterations=100 > lab2_add.csv
./lab2_add --threads=10 --iterations=1000 >> lab2_add.csv
./lab2_add --threads=10 --iterations=10000 >> lab2_add.csv
./lab2_add --threads=10 --iterations=100000 >> lab2_add.csv

echo "Creating data for lab2_add-2.png"
for threads in {2..12..2}
do
	echo $statement$threads
	./lab2_add --threads=$threads --iterations=10 --yield >> lab2_add.csv
	./lab2_add --threads=$threads --iterations=20 --yield >> lab2_add.csv
	./lab2_add --threads=$threads --iterations=40 --yield >> lab2_add.csv
	./lab2_add --threads=$threads --iterations=80 --yield >> lab2_add.csv
	iterations=100
	while [ $iterations -le 100000 ]
	do
		./lab2_add --threads=$threads --iterations=$iterations --yield >> lab2_add.csv
		((iterations *= 10))
	done
done

echo "Creating data for lab2_add-3.png"
echo "Running add-none threads=1"
iterations=10
while [ $iterations -le 100000 ]
do
	./lab2_add --threads=1 --iterations=$iterations >> lab2_add.csv
	((iterations *= 10))
done

echo "Creating data for lab2_add-4.png"
echo "Running mutex test"
for threads in {2..12..2}
do
	./lab2_add --threads=$threads --iterations=10000 --yield --sync=m >> lab2_add.csv
done

echo "Running spin lock test"
for threads in {2..12..2}
do
	./lab2_add --threads=$threads --iterations=1000 --yield --sync=s >> lab2_add.csv
done

echo "Running compare_and_swap test"
for threads in {2..12..2}
do
	./lab2_add --threads=$threads --iterations=10000 --yield --sync=c >> lab2_add.csv
done

echo "Creating data for lab2_add-5.png"
echo "Running none test"
./lab2_add --threads=1 --iterations=10000 >> lab2_add.csv
for threads in {2..12..2}
do
	./lab2_add --threads=$threads --iterations=10000 >> lab2_add.csv
done

echo "Running mutex test"
./lab2_add --threads=1 --iterations=10000 --sync=m >> lab2_add.csv
for threads in {2..12..2}
do
	./lab2_add --threads=$threads --iterations=10000 --sync=m >> lab2_add.csv
done

echo "Running spin lock test"
./lab2_add --threads=1 --iterations=10000 --sync=s >> lab2_add.csv
for threads in {2..12..2}
do
	./lab2_add --threads=$threads --iterations=10000 --sync=s >> lab2_add.csv
done

echo "Running compare_and_swap test"
./lab2_add --threads=1 --iterations=10000 --sync=c >> lab2_add.csv
for threads in {2..12..2}
do
	./lab2_add --threads=$threads --iterations=10000 --sync=c >> lab2_add.csv
done

