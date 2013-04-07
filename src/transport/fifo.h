typedef char CLSVBUFF[200];

int create_channel(void *);

int receive_packet(CLSVBUFF *, int, void *);

int send_packet(CLSVBUFF *, int, void *);
