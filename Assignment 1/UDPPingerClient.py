from socket import *
import time
import sys
from func_timeout import func_set_timeout, FunctionTimedOut

serverName = sys.argv[1] # first arguement
serverPort = 12000 # server port
message = "Hello!" # Input
num = 10

@func_set_timeout(1)
def receive(clientSocket):
    return clientSocket.recvfrom(2048) # wait receive

M, m, S, loss = 0, 10000000, 0, 0
for i in range(1,num+1):
    clientSocket = socket(AF_INET, SOCK_DGRAM) # IPv4 UDP
    clientSocket.sendto(message.encode(), (serverName, serverPort)) # send carrying address
    start = time.time()
    try:
        modifiedMessage, Add = receive(clientSocket) # wait receive
    except FunctionTimedOut:
        loss += 1
    end = time.time()
    during = min(end-start,1)
    M = max(M, during)
    m = min(m, during)
    S = S + during
    if during < 1:
        print("Ping {:2d} {:.2f}ms".format(i,during*1000))
    else:
        print("Ping {:2d} Request timed out".format(i))
    clientSocket.close()
print("loss rate:{:.1f}%, max/min/avg:{:.2f}/{:.2f}/{:.2f} ms".format(loss/num*100, M*1000, m*1000, S/num*1000))
