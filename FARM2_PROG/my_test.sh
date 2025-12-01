echo test consegna progetto "./farm -n 4 -q 4 file1.dat file2.dat file3.dat file4.dat file5.dat -d testdir"
./farm -n 4 -q 4 file1.dat file2.dat file3.dat file4.dat file5.dat -d testdir &
pid=$! 
wait $pid
echo "Numero file nel thread pool: "
cat nworkeratexit.txt
echo

echo test consegna progetto "./farm -n 8 -q 4 -t 200 file*"
./farm -n 8 -q 4 -t 200 file* &
sleep 1
pkill -int farm
pid=$! 
wait $pid
echo "Numero file nel thread pool: "
cat nworkeratexit.txt
echo

echo test6 "./farm -n 1 -d testdir -q 1 -t 1000 file*"
./farm -n 1 -d testdir -q 1 -t 1000 file*  &
pid=$! 
sleep 2
echo  SIGUSR1 after 2 sec
pkill -USR1 farm
sleep 3
echo  SIGUSR1 after 5 sec
pkill -USR1 farm 
sleep 6 
echo  SIGUSR1 after 11 sec and then await for termination 
pkill -USR1 farm
wait $pid
echo "Numero file nel thread pool: "
cat nworkeratexit.txt
echo


echo test7 "./farm -n 1 -d testdir -q 1 -t 1000 file*"
./farm -n 1 -d testdir -q 1 -t 1000 file*  &
pid=$! 
sleep 7
echo SIGTERM after 7 sec
pkill -TERM farm
wait $pid
echo "Numero file nel thread pool: "
cat nworkeratexit.txt
echo

echo test8 "./farm -n 1 -d testdir -q 1 -t 1000 file*"
./farm -n 1 -d testdir -q 1 -t 1000 file* &
pid=$!
sleep 5
echo SIGINT after 5 sec
pkill -INT farm
wait $pid
echo "Numero file nel thread pool: "
cat nworkeratexit.txt
echo

echo test9 "./farm -n 1 -d testdir -q 1 -t 1000 file*"
./farm -n 1 -d testdir -q 1 -t 1000 file*  &
pid=$! 
sleep 7
echo SIGHUP after 7 sec
pkill -HUP farm
wait $pid
echo "Numero file nel thread pool: "
cat nworkeratexit.txt
echo

echo test10 "./farm -n 1 -d testdir -q 1 -t 1000 file*"
./farm -n 1 -d testdir -q 1 -t 1000 file*  &
pid=$! 
sleep 7
echo SIGUSR1 after 7 sec
pkill -usr1 farm
sleep 3
echo SIGQUIT after 10 sec
pkill -QUIT farm
wait $pid
echo "Numero file nel thread pool: "
cat nworkeratexit.txt
echo


echo test11 "./farm -n 1 -d testdir -q 1 -t 1000 file*"
./farm -n 1 -d testdir -q 1 -t 1000 file*  &
pid=$! 
sleep 5
echo due sigusr1 in sequenza dopo 5 secondi  
pkill -usr1 farm
pkill -usr1 farm
wait $pid
echo "Numero file nel thread pool: "
cat nworkeratexit.txt
echo


echo test12 "./farm -n 4 -d testdir -q 8 -t 2000 file*"
./farm -n 4 -d testdir -q 8 -t 2000 file*  &
pid=$! 
echo  SIGUSR2 after 2 sec
pkill -USR2 farm
sleep 3
echo tre sigusr2 in sequenza dopo 5 secondi  
pkill -usr2 farm
pkill -usr2 farm
pkill -usr2 farm
wait $pid
echo "Numero file nel thread pool: "
cat nworkeratexit.txt
echo

echo test13 "./farm -n 0 -d testdir -q 8  file*"
./farm -n 0 -d testdir -q 8  file*  &
pid=$! 
wait $pid
echo "Numero file nel thread pool: "
cat nworkeratexit.txt
echo

echo test14 "./farm -n 1 -d testdir -q 1  file*"
./farm -n 1 -d testdir -q 1  file*  &
pid=$! 
wait $pid
echo "Numero file nel thread pool: "
cat nworkeratexit.txt
echo

echo test15 "./farm -n 4 -d testdir -q 8"
./farm -n 4 -d testdir -q 8  &
pid=$! 
echo  SIGUSR1 after 1 sec
pkill -USR1 farm
sleep 1
echo  SIGUSR1 after 2 sec
pkill -USR1 farm 
sleep 1 
echo  SIGUSR2 after 3 sec and then await for termination 
pkill -USR2 farm
wait $pid
echo "Numero file nel thread pool: "
cat nworkeratexit.txt
echo

echo test16 "./farm -n 4  -q 8 file*"
./farm -n 4  -q 8 file*  &
pid=$! 
echo  SIGUSR1 after 1 sec
pkill -USR1 farm
sleep 1
echo  SIGUSR1 after 2 sec
pkill -USR1 farm 
sleep 1 
echo  SIGHUP after 3 sec and then await for termination 
pkill -HUP farm
wait $pid
echo "Numero file nel thread pool: "
cat nworkeratexit.txt
echo

echo test17 "./farm  -n 8 -q 1 -t 200 file* -d testdir"
./farm  -n 8 -q 1 -t 200 file* -d testdir &
pid=$! 
wait $pid
echo "Numero file nel thread pool: "
cat nworkeratexit.txt
echo

echo test18 "./farm -n 1 -d testdir -q 1 -t 1000 file*"
./farm -n 1 -d testdir -q 1 -t 1000 file*  &
pid=$! 
sleep 2
echo SIGTERM after 2 sec
pkill -TERM farm
wait $pid
echo "Numero file nel thread pool: "
cat nworkeratexit.txt
echo
