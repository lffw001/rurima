#!/bin/sh
# Fix DNS.
rm /etc/resolv.conf
echo "nameserver 1.1.1.1" > /etc/resolv.conf
exit 0