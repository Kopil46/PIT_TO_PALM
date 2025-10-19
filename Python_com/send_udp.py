# usage:
#   python3 send_udp.py 192.168.4.1 ON
#   python3 send_udp.py 192.168.4.1 OFF
#   python3 send_udp.py 192.168.4.1 TOGGLE
#   python3 send_udp.py 192.168.4.1 PULSE:500

import socket, sys
PORT = 5005

if len(sys.argv) < 3:
    print("Usage: python3 send_udp.py <ESP_IP> <CMD>\n  CMD: ON | OFF | TOGGLE | PULSE:<ms>")
    raise SystemExit

ip = sys.argv[1]
cmd = sys.argv[2].encode()

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.settimeout(2.0)
s.sendto(cmd, (ip, PORT))
print("Sent:", cmd.decode())

try:
    data, addr = s.recvfrom(1024)
    print("Reply from", addr, "->", data.decode(errors="ignore"))
except socket.timeout:
    print("No reply (timeout)")
