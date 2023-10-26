import subprocess
import time
import datetime
import threading

end_thread = False
now = datetime.datetime.now()
file = open('Log_'+ now.strftime("%Y:%m:%d:%H:%M:%S")+'_.txt','w')
def init_picocom(serial_port):
    global end_thread
    global file
    pico_cmd = f"picocom -b 115200 {serial_port}"
    print(pico_cmd)

    pico_process = subprocess.Popen(pico_cmd,shell=True,stdout=subprocess.PIPE, stdin=subprocess.PIPE, text= True)
    while not end_thread:
        reply = pico_process.stdout.readline()
        if(len(reply) != 0):
            print("NODE:",reply)
        file.write("\"" + datetime.datetime.now().strftime("%d/%m/%Y %H:%M:%S")+"\""+":"+reply)

        time.sleep(0.01)

    pico_process.terminate()
    pico_process.wait()

def stop_thread():
    global end_thread
    end_thread = True

port = input("Enter serial port(e.g. ,/dev/ttyUSB0):")
print(port)
pico_thread = threading.Thread(target=init_picocom,args=(port,))
pico_thread.daemon = True
pico_thread.start()

input("Press Enter to terminate the thread.")
stop_thread()  # Set the flag to terminate the thread

# Join the picocom thread to wait for it to terminate
pico_thread.join()

print("Thread terminated.")