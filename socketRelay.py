# -*- coding: utf-8 -*-  
import urllib3
import logging
import time
import sys,time,threading,RPi.GPIO
logging.getLogger('socketIO-client').setLevel(logging.DEBUG)
logging.captureWarnings(True)

from socketIO_client import SocketIO, LoggingNamespace
urllib3.disable_warnings()

deamonTime=time.time()
data=[(0.007,0.01) for i in range(5)]
postdata=data
limits=[0,80,120,160,190,256]
limitss=[[0,0.01],[0.01,0.006],[0.01,0.004],[0.01,0.003],[0.01,0.003]]
RPi.GPIO.setmode(RPi.GPIO.BOARD)

def motorDeamon():
    global postdata,data,deamonTime
    while True:
        time.sleep(0.1)
        RPi.GPIO.input(29)
        if postdata!=data:
            deamonTime=time.time()
            postdata=data
        else:
            if time.time()-deamonTime>3:
                data=[(0,0.01) for i in range(5)]

def work(motor):
    while True:
        RPi.GPIO.output(pins[motor], 1)
        time.sleep(data[motor][0])
        RPi.GPIO.output(pins[motor], 0)
        time.sleep(data[motor][1])

def trans(a):
    i=0
    while not (limits[i+1]>a and limits[i]<=a):
        i+=1
    return tuple(limitss[i])

def run(*arg):
    global data
    print(type(arg),str(arg))
    if(type(arg[0])==int and -1<arg[0]<5):
        data=[(0,0.01) for i in range(5)]
        data[arg[0]]=trans(255)
        print(data)
    print('in run')

def reset():
    global data
    data=[(0,0.01) for i in range(5)]
    print('-----reset')

try:
    pins=[29,31,33,35,37]
    for i in pins:
        RPi.GPIO.setup(i, RPi.GPIO.OUT)

    tasks = [threading.Thread(target=work,args=(i,)) for i in range(5)]
    for i in tasks:
        i.start()
    threading.Thread(target=motorDeamon).start()

    socketIO=SocketIO('http://139.198.188.89:32884', verify=False)

    print('start 1')
    socketIO.on('Alvolus', run)
    socketIO.on('reset', reset)
    socketIO.wait()

except KeyboardInterrupt:
    RPi.GPIO.cleanup()

