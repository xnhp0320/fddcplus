#!/usr/bin/python
import fileinput
import sys
import re
import os
import subprocess
import random
import copy

def permutations(dl):

    ret = []
    if len(dl) == 1:
        ret.append(copy.copy(dl))
        return ret

    for i in xrange(len(dl)):
        t = dl[0]
        dl[0] = dl[i]
        dl[i] = t
        rret = permutations(dl[1:])
        for x in rret:
            x.insert(0, dl[0])
        for x in rret:
            ret.append(x)

    return ret



if __name__ == "__main__":
    order_list = permutations([0,1,2,3,4])
    out = open(os.path.basename(sys.argv[1]), "w")
    #signal.signal(signal.SIGALRM, alarm_handler)
    #signal.alarm(30)

    for order in order_list:
        command = reduce(lambda x,y:x+y, map(lambda x: str(x), order))
        #print command
        print "test ", order
        #print command+" "+sys.argv[1]
        subprocess.call("./fastrr -o " + command +" -r "+ sys.argv[1], stdout=out, shell=True)




