# 📁 TFTP (Trivial File Transfer Protocol)

## 📌 Description

TFTP is a simple client-server file transfer project developed in C using UDP sockets. It allows a client and server to transfer files using the TFTP protocol.

## ✨ Features

* 📤 Supports file upload
* 📥 Supports file download
* 🌐 Client-server communication using UDP
* 📖 Supports Read Request (RRQ)
* 📝 Supports Write Request (WRQ)
* 📦 Handles DATA and ACK packets
* ⚠️ Basic error handling
* 🔄 Timeout and retransmission support

## 🛠️ Technologies Used

* C Programming
* Linux
* UDP Sockets
* Computer Networks
* TFTP Protocol

## 📂 Project Structure

* `tftp.h` – Common definitions, constants, and packet structures
* `tftp.c` – Common TFTP functions
* `tftp_client.h` – Client-specific declarations
* `tftp_client.c` – Implements the TFTP client
* `tftp_server.c` – Implements the TFTP server

## 📡 Protocol Details

* **Port:** 6969
* **Buffer Size:** 516 bytes
* **Timeout:** 5 seconds
* **RRQ** – Read Request
* **WRQ** – Write Request
* **DATA** – Transfers file data
* **ACK** – Acknowledges received data
* **ERROR** – Reports errors

## ⚙️ How to Compile

Compile the server:

```bash
gcc tftp_server.c tftp.c -o tftp_server
```

Compile the client:

```bash
gcc tftp_client.c tftp.c -o tftp_client
```

## ▶️ How to Run

Start the server:

```bash
./tftp_server
```

Open another terminal and run the client:

```bash
./tftp_client
```

## 🎯 Learning Outcome

This project helped in understanding UDP socket programming, client-server communication, packet handling, file transfer, timeout and retransmission mechanisms, and the working of the TFTP protocol.

