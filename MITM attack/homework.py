from bitstring import Bits, BitArray, BitStream, pack
from itertools import product
from threading import Thread, Lock
from queue import Empty, Queue
import time
#это подстановка 1>4>5>2>3>7>15>8>0>9>6>11>13>12>10>14
S = {
    "0000": "1001",#0 > 9
    "0001": "0100",#1 > 4
    "0010": "0011",#2 > 3
    "0011": "0111",#3 > 7
    "0100": "0101",#4 > 5
    "0101": "0010",#5 > 2
    "0110": "1011",#6 > 11
    "0111": "1111",#7 > 15
    "1000": "0000",#8 > 0
    "1001": "0110",#9 > 6
    "1010": "1110",#10 > 14
    "1011": "1101",#11 > 13
    "1100": "1010",#12 > 10
    "1101": "1100",#13 > 12
    "1110": "0001",#14 > 1
    "1111": "1000",#15 > 8
}
rS = {
    "1001": "0000",#9 > 0
    "0100": "0001",#4 > 1
    "0011": "0010",#3 > 2
    "0111": "0011",#
    "0101": "0100",
    "0010": "0101",
    "1011": "0110",
    "1111": "0111",
    "0000": "1000",
    "0110": "1001",
    "1110": "1010",
    "1101": "1011",
    "1010": "1100",
    "1100": "1101",
    "0001": "1110",
    "1000": "1111",
}
#encrypt func
def encrypt(text,key, count):
    if(count == 1):
        res = text ^ key[:16]
        part1 = S[res.bin[:4]]
        part2 = S[res.bin[4:8]]
        part3 = S[res.bin[8:12]]
        part4 = S[res.bin[12:]]
        text = BitArray(bin = part1+part3+part2+part4)
        #print('1:',text)
    if(count == 2):
        res = text ^ key[:16]
        part1 = S[res.bin[:4]]
        part2 = S[res.bin[4:8]]
        part3 = S[res.bin[8:12]]
        part4 = S[res.bin[12:]]
        text = BitArray(bin = part1+part3+part2+part4)
        #print('1:',text.bin)
        res = text ^ key[16:32]
        part1 = S[res.bin[:4]]
        part2 = S[res.bin[4:8]]
        part3 = S[res.bin[8:12]]
        part4 = S[res.bin[12:]]
        text = BitArray(bin = part1+part3+part2+part4)
        #print('2:',text.bin)
    """if(count == 3):
        res = text ^ key[:16]
        part1 = S[res.bin[:4]]
        part2 = S[res.bin[4:8]]
        part3 = S[res.bin[8:12]]
        part4 = S[res.bin[12:]]
        text = BitArray(bin = part1+part3+part2+part4)
        #print('1:',text.bin)
        res = text ^ key[16:32]
        part1 = S[res.bin[:4]]
        part2 = S[res.bin[4:8]]
        part3 = S[res.bin[8:12]]
        part4 = S[res.bin[12:]]
        text = BitArray(bin = part1+part3+part2+part4)
        #print('2:',text.bin)
        res = text ^ key[32:48]
        part1 = S[res.bin[:4]]
        part2 = S[res.bin[4:8]]
        part3 = S[res.bin[8:12]]
        part4 = S[res.bin[12:]]
        text = BitArray(bin = part1+part3+part2+part4)
        #print('3:',text.bin)
    if(count == 4):
        res = text ^ key[:16]
        part1 = S[res.bin[:4]]
        part2 = S[res.bin[4:8]]
        part3 = S[res.bin[8:12]]
        part4 = S[res.bin[12:]]
        text = BitArray(bin = part1+part3+part2+part4)
        #print('1:',text.bin)
        res = text ^ key[16:32]
        part1 = S[res.bin[:4]]
        part2 = S[res.bin[4:8]]
        part3 = S[res.bin[8:12]]
        part4 = S[res.bin[12:]]
        text = BitArray(bin = part1+part3+part2+part4)
        #print('2:',text.bin)
        res = text ^ key[32:48]
        part1 = S[res.bin[:4]]
        part2 = S[res.bin[4:8]]
        part3 = S[res.bin[8:12]]
        part4 = S[res.bin[12:]]
        text = BitArray(bin = part1+part3+part2+part4)
        #print('3:',text.bin)
        res = text ^ key[48:64]
        part1 = S[res.bin[:4]]
        part2 = S[res.bin[4:8]]
        part3 = S[res.bin[8:12]]
        part4 = S[res.bin[12:]]
        text = BitArray(bin = part1+part3+part2+part4)
        #print('4:',text.bin)"""
    return text
#decrypt func
def decrypt(text,key):
    part1 = rS[text.bin[:4]]
    part2 = rS[text.bin[4:8]]
    part3 = rS[text.bin[8:12]]
    part4 = rS[text.bin[12:]]
    text = BitArray(bin = part1+part3+part2+part4)
    res = text ^ key[16:32]
    #print('4:',res)
    """part1 = rS[res.bin[:4]]
    part2 = rS[res.bin[4:8]]
    part3 = rS[res.bin[8:12]]
    part4 = rS[res.bin[12:]]
    text = BitArray(bin = part1+part3+part2+part4)
    res = text ^ key[:16]"""
    #print('3:',res.bin)
    return res
#mitm func
def gen_key():
    #lock.acquire()
    k = []             
    str_key = ''
    tmp_ot = BitArray(0)
    first_key_2 = BitArray(bin = '0000000000000000')
    for key_part_1 in product(range(2),repeat=16):
        for i in range(len (key_part_1)):
            k.append(key_part_1[i])
            str_key+=str(k[i])    
        first_key_1 = BitArray(bin = str_key)
        tmp_ot.append(check(first_key_1+first_key_2,0)+first_key_1)
        f1.put(tmp_ot[:32])
        tmp = BitArray(0)
        tmp = check_res(f1.get(),f2.get())
        if (tmp != 1):
            print(tmp)
        #lock.release    
        time.sleep(1)
def gen_key2():
    #lock.acquire()
    str_key = ''
    k = []
    tmp_ot = BitArray(0)
    first_key_1 = BitArray(bin = '0000000000000000')    
    for key_part_2 in product(range(2),repeat=16):
        for m in range(len (key_part_2)):
            k.append(key_part_2[m])
            str_key+=str(k[m]) 
        first_key_2 = BitArray(bin = str_key)
        tmp_ot.append(check2(first_key_1+first_key_2,0)+first_key_2)
        f2.put(tmp_ot[:32])
        tmp = BitArray(0)
        tmp = check_res(f1.get(),f2.get())
        if (tmp != 1):
            print(tmp)
        #lock.release()
        time.sleep(1)                  
def check(key,count):
     tmp_e = encrypt(array_ot[count:count+16],key,1)
     return tmp_e
def check2(key,count):
     tmp_d = decrypt(array_ct[count:count+16],key)
     return tmp_d
def check_res(tmp1,tmp2):
    return tmp1[:16] == tmp2[:16] if tmp1[16:32]+tmp2[16:32] else 1            
lock = Lock()                  
#open text from string
ot = BitArray(bin = '1100101001010011')
#if block OT < 16
while len(ot) < 16:
    ot.prepend('0b0')
array_ot = BitArray()
array_ot.append(ot)
#print('Arr Ot',array_ot)
#init cipher text
ct = BitArray()
#key 64b
key = BitArray(bin = '0000000000000001 0000000000000001')
array_keys = BitArray()
print('OT:',ot)
print('Key:',key)
f1 = Queue
f2 = Queue
#get CT
ct = encrypt(ot,key,2)
array_ct =BitArray()
array_ct.append(ct)
#print('Arr Ct',array_ct)
def gen_ct():
    for tmp_ot in product(range(2),repeat=16):
        array_ot.append(tmp_ot)
        tmp_ct = encrypt(tmp_ot,key,2)
        array_ct.append(tmp_ct)
        if(tmp_ot == (0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0)):
            return 1
gen_ct()
print('OT/CT pair =',int(len(array_ot)/16-1))
print('Start generate keys')
#find Key
k1_thread = Thread(target=gen_key)
k2_thread = Thread(target=gen_key2)

start = time.time()
k1_thread.start()
k2_thread.start()
#k1_thread.join()
#k2_thread.join()
#check(key,0)
end = time.time()
print('Key found in',end - start,'sec')
