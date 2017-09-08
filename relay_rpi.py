#coding=utf-8
import sys,time,threading,RPi.GPIO
try:
    data=[(0.007,0.01) for i in range(5)]
    limits=[0,80,120,160,190,256]
    limitss=[[0,0.01],[0.01,0.006],[0.01,0.004],[0.01,0.003],[0.01,0.003]]
    RPi.GPIO.setmode(RPi.GPIO.BOARD)

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


    pins=[29,31,33,35,37]
    for i in pins:
        RPi.GPIO.setup(i, RPi.GPIO.OUT)

    tasks = [threading.Thread(target=work,args=(i,)) for i in range(5)]
    for i in tasks:
        i.start()
        
    sys.stderr.write('123')
    while True:
        i=sys.stdin.readline()
        i=i.rstrip(';\n \t');
        sys.stderr.write('\n'+i+'aaaa\n')
        if i=='':
            time.sleep(0.2)
            continue
        if len(i.split(';'))!=5:
            time.sleep(0.2)
            continue
        now = list(map(int,i.split(';')))
        sys.stderr.write(repr(now)+'aaaa\n')
        for j in range(5):
            data[j]=trans(now[j])

except KeyboardInterrupt:
    RPi.GPIO.cleanup()