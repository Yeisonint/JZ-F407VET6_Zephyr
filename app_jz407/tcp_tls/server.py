#!/usr/bin/env python3
import socket
import ssl
import threading
import sys

LISTEN_ADDR = '0.0.0.0'
LISTEN_PORT = 1234
SERVER_CERT = 'crt/server.crt'
SERVER_KEY = 'crt/server.key'

def handle_client(conn, addr):
    print(f"[+] Client connected: {addr}")
    try:
        def send_periodic():
            try:
                while True:
                    conn.sendall(b"Hello from host\n")
                    threading.Event().wait(1)
            except:
                pass
        
        threading.Thread(target=send_periodic, daemon=True).start()

        while True:
            data = conn.recv(1024)
            if not data:
                break
            print(f"[{addr}] -> {data.decode().strip()}")
            
    except Exception as e:
        print(f"[-] Error with {addr}: {e}")
    finally:
        conn.close()
        print(f"[-] Connection terminated with {addr}")

context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
context.load_cert_chain(certfile=SERVER_CERT, keyfile=SERVER_KEY)

bind_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
bind_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
bind_socket.bind((LISTEN_ADDR, LISTEN_PORT))
bind_socket.listen(5)

bind_socket.settimeout(1.0)

print(f"[*] TLS Server active on port {LISTEN_PORT}...")
print("[*] Press Ctrl+C to exit.")

try:
    while True:
        try:
            newsocket, fromaddr = bind_socket.accept()
            conn = context.wrap_socket(newsocket, server_side=True)
            threading.Thread(target=handle_client, args=(conn, fromaddr), daemon=True).start()
        except socket.timeout:
            continue 
except KeyboardInterrupt:
    print("\n[!] Closing server per user request...")
finally:
    bind_socket.close()
    print("[*] Port released. Goodbye.")
    sys.exit(0)