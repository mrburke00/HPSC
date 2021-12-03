#!/usr/bin/python
import os
import time


if __name__ == "__main__":

    time.sleep(1)

    count = 0
    while(True):
        count += 1
        os.system('clear')
        print
        print '------------------------------------------------------------------'
        print 'Checking job with squeue, count = ',count
        print '------------------------------------------------------------------'
        print
        command = "squeue -u scru5660"
        os.system(command)
        time.sleep(1)
