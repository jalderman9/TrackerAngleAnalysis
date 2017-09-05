# Makefile for rt_watts_v1.05a

CC = gcc
CFLAGS = -Wall


all : 
	$(CC) main.c tracking_algorithm.c solarpos.c -lm -o tracker_calc

clean : 
	rm -f tracker_calc *.o *.csv
