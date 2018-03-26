# coding=utf-8
##########################################################################
# Project : To Compute per­hop Traceroute times given a tcpdump text trace
# Date : 02/03/2017
# Name: Chirag Jain
# Course: Computer Networks(CSCI 6760)
#########################################################################

import sys
import re
from datetime import datetime

# Taking Tcpdump file as a command line argument
data = open(sys.argv[1]).read().split('\n')
idList = []

#Method to remove duplicate values
def remove_duplicates(li):
    my_set = set()
    res = []
    for e in li:
        if e not in my_set:
            res.append(e)
            my_set.add(e)
    return res

#Making list of IDs
for i, line in enumerate(data):
    if (re.search('id\s\d+', line)):
        strTTLnID = re.search('id\s\d+', line).group()
        idList.append(strTTLnID)

idList = remove_duplicates(idList)

#Initializing Lists
ttlList = []
ipList = []
rttList = []

#Iterating over ID list
for string in idList:
    for j, lines in enumerate(data):
        if string in lines:
            if re.search('\d{2}:\d{2}:\d{2}.\d{6}', lines) or re.search('\d{10}.\d{6}', lines):
                if re.search('proto\s\w{3}\s', lines):
                    if re.search('ttl\s\d+', lines):
                        if re.search('none', lines):
                            if re.search('\d{2}:\d{2}:\d{2}.\d{6}', lines):
                                strTimestamp1 = re.search('\d{2}:\d{2}:\d{2}.\d{6}', lines).group()
                                date1 = datetime.strptime(strTimestamp1, "%H:%M:%S.%f")
                            else:
                                strTimestamp1 = float(re.search('\d{10}.\d{6}', lines).group())
                                date1 = datetime.fromtimestamp(strTimestamp1).strftime("%H:%M:%S.%f")
                                date1 = datetime.strptime(date1, "%H:%M:%S.%f")

                            ttl = re.search('ttl\s\d+', lines).group()
                            ttlList.append(ttl)

                            ipList.append("*")
                            rttList.append("*")

            else:
                if re.search('\d+.\d+.\d+.\d+', data[j - 1]):
                    ip = re.search('\d+.\d+.\d+.\d+', data[j - 1]).group()
                    ipList.pop()
                    ipList.append(ip)

                if re.search('\d{2}:\d{2}:\d{2}.\d{6}', data[j - 2]):
                    strTimestamp2 = re.search('\d{2}:\d{2}:\d{2}.\d{6}', data[j - 2]).group()
                    date2 = datetime.strptime(strTimestamp2, "%H:%M:%S.%f")
                else:
                    strTimestamp2 = float(re.search('\d{10}.\d{6}', data[j - 2]).group())
                    date2 = datetime.fromtimestamp(strTimestamp2).strftime("%H:%M:%S.%f")
                    date2 = datetime.strptime(date2, "%H:%M:%S.%f")

                time = (date2 - date1).total_seconds() * 1000
                rtt = str(time) + " ms"
                if rtt:
                    rttList.pop()
                    rttList.append(rtt)

for i in range(len(ttlList)):
    if i + 2 < len(ttlList):
        if (ttlList[i] == ttlList[i + 1]) and (ttlList[i] == ttlList[i + 2]):
            print ttlList[i]
            if ipList[i] == "*" and rttList[i] == "*":
                print ipList[i] + " " + rttList[i] + " " + rttList[i + 1] + " " + rttList[i + 2] + '\n'
            elif (ipList[i] == ipList[i + 1] and (ipList[i] == ipList[i + 2])):
                print ipList[i] + '\n' + rttList[i] + '\n' + rttList[i + 1] + '\n' + rttList[i + 2] + '\n'
            elif ipList[i] == ipList[i + 1]:
                print ipList[i] + '\n' + rttList[i] + " " + rttList[i + 1] + '\n'
                print ipList[i+2] + '\n' + rttList[i + 2]
            elif ipList[i+1] == ipList[i + 2]:
                print ipList[i] + '\n' + rttList[i+1]
                print ipList[i+1] + '\n' + rttList[i+1] + " " + rttList[i + 2] + '\n'
            elif ipList[i] == ipList[i + 2]:
                print ipList[i + 2] + '\n' + rttList[i] + " " + rttList[i + 2] + '\n'
                print ipList[i + 1] + '\n' + rttList[i + 1]
            else:
                print ipList[i]
                print rttList[i] + '\n' + rttList[i + 1] + '\n' + rttList[i + 2] + '\n'
        else:
            continue


