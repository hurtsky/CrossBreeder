./dnsmasq -d --all-servers -C /dev/null -n --user=root -x /dev/dnsmasq.pid $LOCALDNS -a 127.0.0.3 -z -p 10053 -c 0 -N -h 
