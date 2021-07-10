

pipe=/tmp/merge_instance_$$
mkfifo $pipe

on_exit()
{
	rm -rf $pipe
}

trap on_exit EXIT

# python read.py<$pipe &>python.log &
# pyhton_pid=$!
echo $pipe
./linux_demo $pipe

wait $pyhton_pid

echo "finish"




