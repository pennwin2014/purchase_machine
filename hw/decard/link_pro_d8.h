

extern int  Send_NAK(int idComDev,unsigned char F_pcb,unsigned int Len,unsigned char *SendData,
             unsigned int *rlen,unsigned char *RecvData,unsigned char timeout);
             
extern int  Send_ACK(int idComDev,unsigned char F_pcb,unsigned int Len,unsigned char *SendData,
             unsigned int *rlen,unsigned char *RecvData,unsigned char timeout);
             
extern int  Send_WTX(int idComDev,unsigned int Len,unsigned char *SendData,
                   unsigned int *rlen,unsigned char *RecvData,unsigned char timeout)
                   
extern int my_pro_commandlink(int idComDev,unsigned int SLen,
                            unsigned char *SendBuffer,unsigned int *rlen,
                          unsigned char *DataBuffer,unsigned char timeout,
                          unsigned int  PlotLen);
                          
                          