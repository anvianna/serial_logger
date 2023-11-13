import subprocess
import time
import math
import datetime
import threading
import os
import sys
import select

end_thread = False
now = datetime.datetime.now()
port = input("Enter serial port(e.g. ,/dev/ttyUSB0):")
#print(port)
file_out = open('Log_'+ now.strftime("%d-%m-%Y_%H-%M-%S")+'.txt','w+')
#file = open("test.txt","wb")
#rfile = open("test.txt",'rb')

file_out.write("\r\nPort:"+port+"\r\n\r\n")



def init_picocom(serial_port):
    global end_thread
    global file_out
    tic = time.perf_counter()
    pico_cmd = f"picocom -b 115200 {serial_port}"
    print(pico_cmd)
    pico_process = subprocess.Popen(pico_cmd,shell=True,stdout=subprocess.PIPE,stdin=subprocess.PIPE, text= True)
    poller = select.poll()
    poller.register(pico_process.stdout, select.POLLIN)
    while not end_thread:
        if(pico_process.poll() == None):
            reply = pico_process.stdout.readline()
            if(len(reply) > 2):
                sys.stdout.write("\r\nReply:"+reply.rstrip()+"\r\n")
                file_out.write("\"" + datetime.datetime.now().strftime("%d/%m/%Y %H-%M-%S-%f")+"\":"+reply.rstrip()+"\r\n")
        ###### nao funciona ############
        #if(time.perf_counter() - tic > 5):
        #    tic = time.perf_counter()
            #if(pico_process.poll() == None):
            #    str = "nodes"+"\r\n"
            #    print(str)
            #    pico_process.stdin.write(str)
            #    pico_process.stdin.flush()
        ###################
        time.sleep(0.01)
        #if(math.floor(time.time())%30 == 0):
            #print("writing\n")vooi
            #pico_process.stdin.write("nodes\r\n")
            #pico_process.stdin.flush()

    pico_process.terminate()
    pico_process.wait()

def stop_thread():
    global end_thread
    end_thread = True

pico_thread = threading.Thread(target=init_picocom,args=(port,))
#pico_thread = threading.Thread(target=alt_init_pipocom,args=(port,))
pico_thread.daemon = True
pico_thread.start()

input("Press Enter to terminate the thread.")
stop_thread()  # Set the flag to terminate the thread

# Join the picocom thread to wait for it to terminate
pico_thread.join()
file_out.close()
print("Thread terminated.")