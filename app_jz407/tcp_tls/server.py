#!/usr/bin/env python3
import socket
import ssl
import threading
import sys

# Configuración
LISTEN_ADDR = '0.0.0.0'
LISTEN_PORT = 1234
SERVER_CERT = 'crt/server.crt'
SERVER_KEY = 'crt/server.key'

def handle_client(conn, addr):
    print(f"[+] Cliente conectado: {addr}")
    try:
        # Hilo para enviar "Hello" cada segundo
        def send_periodic():
            try:
                while True:
                    conn.sendall(b"Hello from host\n")
                    threading.Event().wait(1) # Dormir 1s de forma interrumpible
            except:
                pass
        
        threading.Thread(target=send_periodic, daemon=True).start()

        # Recibir datos de la placa
        while True:
            data = conn.recv(1024)
            if not data:
                break
            # Imprimimos lo que llega de la placa (tus mensajes de 10ms)
            print(f"[{addr}] -> {data.decode().strip()}")
            
    except Exception as e:
        print(f"[-] Error con {addr}: {e}")
    finally:
        conn.close()
        print(f"[-] Conexión terminada con {addr}")

# 1. Configurar SSL
context = ssl.create_default_context(ssl.Purpose.CLIENT_AUTH)
context.load_cert_chain(certfile=SERVER_CERT, keyfile=SERVER_KEY)

# 2. Crear Socket con opciones de re-uso
bind_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
bind_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
bind_socket.bind((LISTEN_ADDR, LISTEN_PORT))
bind_socket.listen(5)

# 3. Timeout para que el hilo principal no se bloquee y capture el Ctrl+C
bind_socket.settimeout(1.0)

print(f"[*] Servidor TLS activo en puerto {LISTEN_PORT}...")
print("[*] Presiona Ctrl+C para salir.")

try:
    while True:
        try:
            newsocket, fromaddr = bind_socket.accept()
            conn = context.wrap_socket(newsocket, server_side=True)
            threading.Thread(target=handle_client, args=(conn, fromaddr), daemon=True).start()
        except socket.timeout:
            continue # Permite volver a chequear si hubo un Ctrl+C
except KeyboardInterrupt:
    print("\n[!] Cerrando servidor por solicitud del usuario...")
finally:
    bind_socket.close()
    print("[*] Puerto liberado. Adiós.")
    sys.exit(0)