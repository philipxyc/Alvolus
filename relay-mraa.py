import sys,time,threading,mraa

data=[(0.007,0.01) for i in range(5)]
limits=[0,80,120,160,190,256]
limitss=[[0,0.01],[0.01,0.006],[0.01,0.004],[0.01,0.003],[0.01,0.003]]

def work(motor):
	while True:
		pins[motor].write(1)
		time.sleep(data[motor][0])
		pins[motor].write(0)
		time.sleep(data[motor][1])

def trans(a):
	i=0
	while not (limits[i+1]>a and limits[i]<=a):
		i+=1
	return tuple(limitss[i])

pins = [mraa.Gpio(i) for i in range(2,7)]
for i in pins:
	i.dir(mraa.DIR_OUT)

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
        time.sleep(0.2)

