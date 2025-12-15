# Lora WWAN
I’m working on creating a LoRa-based WWAN (Wireless Wide Area Network).

### Hardware in use
- esp32
- EBYTE e220-400M33S Lora Module
- 3dbi antena

### Currently working on:
- Testing E220-400M33S driver and communication between two modules.

### To do:
- DB 
    - DB download
    - updates
- Add function for faulty transmission handling

### Currently finished implementing:
- Routing
    - Sequence number
    - Routing table
    - Hash function
    - Get neighbours
- Packet Processing
    - Packet buffering - received packets, sending packets from to_send queue 
    - Modify all the functions to enqueue packets instead of sending them directly
- Decentralized DHCP
    - Address assigning
- Transmit and Receive functions
    - Listen Before Talk
- ARP
- Main Loop
    - Setup - ISR -> Receive, initialize tables, etc.
    - Transmit and process task
- E220-400M33S Driver
    - IRQ Table
    - Basic functions
    - Allow custom settings
    - Transmit(), Receive IRQ callback, Channel scanning.
    - Error checking
    - Channel scanning