import subprocess
import time
import math
import datetime
import threading

end_thread = False
now = datetime.datetime.now()
port = input("Enter serial port(e.g. ,/dev/ttyUSB0):")
print(port)
file = open('Log_'+ now.strftime("%d-%m-%Y_%H-%M-%S")+'.txt','w')
file.write("\r\nPort:"+port+"\r\n\r\n")

def init_picocom(serial_port):
    global end_thread
    global file
    pico_cmd = f"picocom -b 115200 {serial_port}"
    print(pico_cmd)

    pico_process = subprocess.Popen(pico_cmd,shell=True,stdout=subprocess.PIPE, stdin=subprocess.PIPE, text= True)
    while not end_thread:
        reply = pico_process.stdout.readline()
        if(len(reply) > 2):
            print("NODE:",reply)
            file.write("\"" + datetime.datetime.now().strftime("%d/%m/%Y %H:%M:%S")+"\""+":"+reply+"\r\n")

        time.sleep(0.01)
        if(math.floor(time.time())%30 == 0):
            pico_process.stdin.write("nodes\n")

    pico_process.terminate()
    pico_process.wait()

def stop_thread():
    global end_thread
    end_thread = True

pico_thread = threading.Thread(target=init_picocom,args=(port,))
pico_thread.daemon = True
pico_thread.start()

input("Press Enter to terminate the thread.")
stop_thread()  # Set the flag to terminate the thread

# Join the picocom thread to wait for it to terminate
pico_thread.join()

print("Thread terminated.")