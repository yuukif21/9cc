#!/bin/bash
try(){
	expected="$1"
	input="$2"

	./9cc "$input" > tmp.s
	gcc -o tmp tmp.s
	./tmp
	actual="$?"

	if [ "$actual" = "$expected" ]; then
		echo "$input => $actual"
	else
		echo "$expected expectes, but got $actual"
		exit 1
	fi
}

try 0 0
try 42 42
try 21 '5+20-4'
try 41 " 12 + 34 - 5 "
try 47 "5+6*7"
try 27  "30 - 6 /2"
try 4 "(3+ 5)/2"
try 8 "-7+5*3"
try 1 "3*2==7-1"
try 0 "4==5"
try 1 "3!=1"
try 0 "15/3!=5"
try 1 "3<=5"
try 0 "5*2<=6-4"
try 1 "2<5"
try 0 "3*6<10-7"
try 1 "5>1"
try 0 "-7+9>=42"
echo OK
