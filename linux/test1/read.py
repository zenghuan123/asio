import signal
import sys 
def main():
	a = 1
	while a:
		a = sys.stdin.read(1)
		print ("a %s",a)


if __name__ == '__main__':
	print("start")
	main()
	print("end")