#!/bin/sh

iptables -F
iptables -X
iptables -Z
for table in $(</proc/net/ip_tables_names)                                                         
do                                                                                                  
    iptables -t $table -F                                                                      
    iptables -t $table -X                                                                      
    iptables -t $table -Z                                                                      
done
iptables -P INPUT ACCEPT
iptables -P FORWARD ACCEPT
iptables -P OUTPUT ACCEPT
