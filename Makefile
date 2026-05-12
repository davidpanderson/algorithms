all: num gucc

num: num.cpp
	g++ -O4 num.cpp -o num
gucc: gucc.cpp
	g++ -O4 gucc.cpp -o gucc

