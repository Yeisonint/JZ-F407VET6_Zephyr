# ZEPHYR TCP TLS CLIENT

## SERVER FIREWALL SETUP
Enable the communication port on your host machine:

```bash
sudo ufw allow 1234
```

## CERTIFICATE GENERATION (OPENSSL)
Run these commands in your server directory to create the trust chain:

```bash
# Generate CA Key and Certificate
openssl req -x509 -sha256 -nodes -days 365 -newkey rsa:2048 -keyout crt/ca.key -out crt/ca.crt -subj "/CN=ServerZephyrCA"

# Generate Server Private Key
openssl genrsa -out crt/server.key 2048

# Generate Certificate Signing Request (CSR), remember change IP
openssl req -new -key crt/server.key -out crt/server.csr -subj "/CN=192.168.8.220"

# Sign Server Certificate with CA
openssl x509 -req -in crt/server.csr -CA crt/ca.crt -CAkey crt/ca.key -CAcreateserial -out crt/server.crt -days 365 -sha256
```

# FLASHING THE CERTIFICATE (MCUMGR)
Install mcumgr:

```bash
sudo apt install golang -y
go install github.com/apache/mynewt-mcumgr-cli/mcumgr@latest
```

Upload the Root CA to the board's LittleFS partition so it can verify the server:

```bash
mcumgr --conntype serial --connstring="dev=/dev/ttyACM0,baud=115200" fs upload crt/ca.crt /lfs:/ca.crt
```

# VERIFICATION

On Zephyr Shell: Run 'fs ls lfs:/' to confirm 'ca.crt' exists.

On Host: Run the Python server script ensuring it points to 'crt/server.crt' and 'crt/server.key'.

```bash
cd app_jz407/tcp_tls
./server.py
```