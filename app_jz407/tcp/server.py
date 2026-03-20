#!/usr/bin/env python3
import socket
import sys

SERVER_IP = "0.0.0.0"
SERVER_PORT = 1234
MESSAGE = b"Hello from pc!\n"

def start_tcp_server():
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    
    try:
        server_socket.bind((SERVER_IP, SERVER_PORT))
        server_socket.listen(1)
        server_socket.settimeout(1.0)
        print(f"[*] Server listening on {SERVER_IP}:{SERVER_PORT}")
        print("[*] Press Ctrl+C to stop the server.")
    except Exception as e:
        print(f"[!] Error binding to port: {e}")
        return

    try:
        while True:
            print("[*] Waiting for client to connect...")
            try:
                client_socket, client_address = server_socket.accept()
            except socket.timeout:
                continue

            try:
                print(f"[+] Connection established from {client_address}")
                client_socket.sendall(MESSAGE)
                print(f"[->] Sent: {MESSAGE.decode().strip()}")

                while True:
                    data = client_socket.recv(1024)
                    if not data:
                        print("[-] Client disconnected.")
                        break
                    print(f"[<-] Received: {data.decode().strip()}")
                    
            except Exception as e:
                print(f"[!] Connection error: {e}")
            finally:
                client_socket.close()
                
    except KeyboardInterrupt:
        print("\n[!] Server stopping per user request...")
    finally:
        server_socket.close()
        print("[*] Server socket closed. Goodbye.")

if __name__ == "__main__":
    start_tcp_server()