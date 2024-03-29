/*
 * (c)COPYRIGHT
 * ALL RIGHT RESERVED
 *
 * FileName : w5500.c
  * -----------------------------------------------------------------
 */
//#include <stdio.h>
#include <string.h>
#include "config.h"
#include "SPI2.h"
#include "w5500.h"
#include "socket.h"

#ifdef __DEF_IINCHIP_PPP__
   #include "md5.h"
#endif

static uint_8 I_STATUS[MAX_SOCK_NUM];
static uint_16 SSIZE[MAX_SOCK_NUM]; /**< Max Tx buffer size by each channel */
static uint_16 RSIZE[MAX_SOCK_NUM]; /**< Max Rx buffer size by each channel */

uint_8 getISR(uint_8 s)
{
  return I_STATUS[s];
}
void putISR(uint_8 s, uint_8 val)
{
   I_STATUS[s] = val;
}

uint_16 getIINCHIP_RxMAX(uint_8 s)
{
   return RSIZE[s];
}
uint_16 getIINCHIP_TxMAX(uint_8 s)
{
   return SSIZE[s];
}
void IINCHIP_CSoff(void)
{
  WIZ_CS(0);
}
void IINCHIP_CSon(void)
{
   WIZ_CS(1);
}
uint_8  IINCHIP_SpiSendData(uint_8 dat)
{
   return(SPI2_SendByte(dat));
}

void IINCHIP_WRITE( uint_32 addrbsb,  uint_8 data)
{
   IINCHIP_ISR_DISABLE();                        // Interrupt Service Routine Disable
   IINCHIP_CSoff();                              // CS=0, SPI start
   SPI_send1(SPI_2, (addrbsb & 0x00FF0000)>>16);
   SPI_send1(SPI_2, (addrbsb & 0x0000FF00)>>8);
   SPI_send1(SPI_2, (addrbsb & 0x000000F8)+4);
//   IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);// Address byte 1
//   IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);// Address byte 2
//   IINCHIP_SpiSendData( (addrbsb & 0x000000F8) + 4);    // Data write command and Write data length 1
   SPI_send1(SPI_2, data);
//   IINCHIP_SpiSendData(data);                    // Data write (write 1byte data)
   IINCHIP_CSon();                               // CS=1,  SPI end
   IINCHIP_ISR_ENABLE();                         // Interrupt Service Routine Enable
}

uint_8 IINCHIP_READ(uint_32 addrbsb)
{
   uint_8 data = 0;
   IINCHIP_ISR_DISABLE();                        // Interrupt Service Routine Disable
   IINCHIP_CSoff();                              // CS=0, SPI start
   SPI_send1(SPI_2, (addrbsb & 0x00FF0000)>>16);
   SPI_send1(SPI_2, (addrbsb & 0x0000FF00)>>8);
   SPI_send1(SPI_2, (addrbsb & 0x000000F8));
//   IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);// Address byte 1
//   IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);// Address byte 2
//   IINCHIP_SpiSendData( (addrbsb & 0x000000F8))    ;// Data read command and Read data length 1
   data = IINCHIP_SpiSendData(0x00);
//   data = IINCHIP_SpiSendData(0x00);             // Data read (read 1byte data)
   IINCHIP_CSon();                               // CS=1,  SPI end
   IINCHIP_ISR_ENABLE();                         // Interrupt Service Routine Enable
   return data;    
}

uint_16 wiz_write_buf(uint_32 addrbsb,uint_8* buf,uint_16 len)
{
   uint_16 idx = 0;
   if(len == 0) printf("Unexpected2 length 0\r\n");

//   IINCHIP_ISR_DISABLE();
	DISABLE_INTERRUPTS;
   IINCHIP_CSoff();                              // CS=0, SPI start
   SPI_send1(SPI_2, (addrbsb & 0x00FF0000)>>16);
   SPI_send1(SPI_2, (addrbsb & 0x0000FF00)>>8);
   SPI_send1(SPI_2, (addrbsb & 0x000000F8)+4);
//   IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);// Address byte 1
//   IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);// Address byte 2
//   IINCHIP_SpiSendData( (addrbsb & 0x000000F8) + 4);    // Data write command and Write data length 1
   for(idx = 0; idx < len; idx++)                // Write data in loop
   {
//     IINCHIP_SpiSendData(buf[idx]);
	   SPI_send1(SPI_2, buf[idx]);
   }
   IINCHIP_CSon();
   ENABLE_INTERRUPTS;// CS=1, SPI end
//   IINCHIP_ISR_ENABLE();                         // Interrupt Service Routine Enable

   return len;  
}

uint_16 wiz_read_buf(uint_32 addrbsb, uint_8* buf,uint_16 len)
{
  uint_16 idx = 0;
  if(len == 0)
  {
    printf("Unexpected2 length 0\r\n");
  }

  IINCHIP_ISR_DISABLE();
  //SPI MODE I/F
  IINCHIP_CSoff();                                  // CS=0, SPI start
  SPI_send1(SPI_2, (addrbsb & 0x00FF0000)>>16);
  SPI_send1(SPI_2, (addrbsb & 0x0000FF00)>>8);
  SPI_send1(SPI_2, (addrbsb & 0x000000F8));
//  IINCHIP_SpiSendData( (addrbsb & 0x00FF0000)>>16);// Address byte 1
//  IINCHIP_SpiSendData( (addrbsb & 0x0000FF00)>> 8);// Address byte 2
//  IINCHIP_SpiSendData( (addrbsb & 0x000000F8));    // Data write command and Write data length 1
  for(idx = 0; idx < len; idx++)                    // Write data in loop
  {
	  DISABLE_INTERRUPTS;
    buf[idx] = IINCHIP_SpiSendData(0x00);
    ENABLE_INTERRUPTS;
//	  buf[idx] = SPI_send1(SPI_2, 0x00);
//	  SPI_send1(SPI_2, 0x00);
//	  delay_us(1000000);
  }
  IINCHIP_CSon();                                   // CS=1, SPI end
  IINCHIP_ISR_ENABLE();                             // Interrupt Service Routine Enable
  
  return len;
}


/**
@brief  This function is for resetting of the iinchip. Initializes the iinchip to work in whether DIRECT or INDIRECT mode
*/
void iinchip_init(void)
{
  setMR( MR_RST );
#ifdef __DEF_IINCHIP_DBG__
  printf("MR value is %02x \r\n",IINCHIP_READ_COMMON(MR));
#endif
}

/**
@brief  This function set the transmit & receive buffer size as per the channels is used
Note for TMSR and RMSR bits are as follows\n
bit 1-0 : memory size of channel #0 \n
bit 3-2 : memory size of channel #1 \n
bit 5-4 : memory size of channel #2 \n
bit 7-6 : memory size of channel #3 \n
bit 9-8 : memory size of channel #4 \n
bit 11-10 : memory size of channel #5 \n
bit 12-12 : memory size of channel #6 \n
bit 15-14 : memory size of channel #7 \n
Maximum memory size for Tx, Rx in the W5500 is 16K Bytes,\n
In the range of 16KBytes, the memory size could be allocated dynamically by each channel.\n
Be attentive to sum of memory size shouldn't exceed 8Kbytes\n
and to data transmission and receiption from non-allocated channel may cause some problems.\n
If the 16KBytes memory is already  assigned to centain channel, \n
other 3 channels couldn't be used, for there's no available memory.\n
If two 4KBytes memory are assigned to two each channels, \n
other 2 channels couldn't be used, for there's no available memory.\n
*/
void sysinit( uint_8 * tx_size, uint_8 * rx_size  )
{
  int_16 i;
  int_16 ssum,rsum;
#ifdef __DEF_IINCHIP_DBG__
  printf("sysinit()\r\n");
#endif
  ssum = 0;
  rsum = 0;

  for (i = 0 ; i < MAX_SOCK_NUM; i++)       // Set the size, masking and base address of Tx & Rx memory by each channel
  {
          IINCHIP_WRITE( (Sn_TXMEM_SIZE(i)), tx_size[i]);
          IINCHIP_WRITE( (Sn_RXMEM_SIZE(i)), rx_size[i]);
          
#ifdef __DEF_IINCHIP_DBG__
         printf("tx_size[%d]: %d, Sn_TXMEM_SIZE = %d\r\n",i, tx_size[i], IINCHIP_READ(Sn_TXMEM_SIZE(i)));
         printf("rx_size[%d]: %d, Sn_RXMEM_SIZE = %d\r\n",i, rx_size[i], IINCHIP_READ(Sn_RXMEM_SIZE(i)));
#endif
    SSIZE[i] = (int_16)(0);
    RSIZE[i] = (int_16)(0);


    if (ssum <= 16384)
    {
         switch( tx_size[i] )
      {
      case 1:
        SSIZE[i] = (int_16)(1024);
        break;
      case 2:
        SSIZE[i] = (int_16)(2048);
        break;
      case 4:
        SSIZE[i] = (int_16)(4096);
        break;
      case 8:
        SSIZE[i] = (int_16)(8192);
        break;
      case 16:
        SSIZE[i] = (int_16)(16384);
      break;
      default :
        RSIZE[i] = (int_16)(2048);
        break;
      }
    }

   if (rsum <= 16384)
    {
         switch( rx_size[i] )
      {
      case 1:
        RSIZE[i] = (int_16)(1024);
        break;
      case 2:
        RSIZE[i] = (int_16)(2048);
        break;
      case 4:
        RSIZE[i] = (int_16)(4096);
        break;
      case 8:
        RSIZE[i] = (int_16)(8192);
        break;
      case 16:
        RSIZE[i] = (int_16)(16384);
        break;
      default :
        RSIZE[i] = (int_16)(2048);
        break;
      }
    }
    ssum += SSIZE[i];
    rsum += RSIZE[i];

  }
}

// added

/**
@brief  This function sets up gateway IP address.
*/
void setGAR(
  uint_8 * addr  /**< a pointer to a 4 -byte array responsible to set the Gateway IP address. */
  )
{
    wiz_write_buf(GAR0, addr, 4);
}
void getGWIP(uint_8 * addr)
{
    wiz_read_buf(GAR0, addr, 4);
}

/**
@brief  It sets up SubnetMask address
*/
void setSUBR(uint_8 * addr)
{   
    wiz_write_buf(SUBR0, addr, 4);
}
/**
@brief  This function sets up MAC address.
*/
void setSHAR(
  uint_8 * addr  /**< a pointer to a 6 -byte array responsible to set the MAC address. */
  )
{
  wiz_write_buf(SHAR0, addr, 6);  
}

/**
@brief  This function sets up Source IP address.
*/
void setSIPR(
  uint_8 * addr  /**< a pointer to a 4 -byte array responsible to set the Source IP address. */
  )
{
    wiz_write_buf(SIPR0, addr, 4);  
}

/**
@brief  This function sets up Source IP address.
*/
void getGAR(uint_8 * addr)
{
    wiz_read_buf(GAR0, addr, 4);
}
void getSUBR(uint_8 * addr)
{
    wiz_read_buf(SUBR0, addr, 4);
}
void getSHAR(uint_8 * addr)
{
    wiz_read_buf(SHAR0, addr, 6);
}
void getSIPR(uint_8 * addr)
{
    wiz_read_buf(SIPR0, addr, 4);
}

void getVERSIONR(uint_8 * addr)
{
    wiz_read_buf(VERSIONR, addr, 4);
}

void setMR(uint_8 val)
{
  IINCHIP_WRITE(MR,val);
}

void setSIMR(uint_8 val)
{
  IINCHIP_WRITE(SIMR,val);
}

/**
@brief  This function gets Interrupt register in common register.
 */
uint_8 getIR( void )
{
   return IINCHIP_READ(IR);
}

/**
@brief  This function sets up Retransmission time.

If there is no response from the peer or delay in response then retransmission
will be there as per RTR (Retry Time-value Register)setting
*/
void setRTR(uint_16 timeout)
{
  IINCHIP_WRITE(RTR0,(uint_8)((timeout & 0xff00) >> 8));
  IINCHIP_WRITE(RTR1,(uint_8)(timeout & 0x00ff));
}

/**
@brief  This function set the number of Retransmission.

If there is no response from the peer or delay in response then recorded time
as per RTR & RCR register seeting then time out will occur.
*/
void setRCR(uint_8 retry)
{
  IINCHIP_WRITE(WIZ_RCR,retry);
}

/**
@brief  This function set the interrupt mask Enable/Disable appropriate Interrupt. ('1' : interrupt enable)

If any bit in IMR is set as '0' then there is not interrupt signal though the bit is
set in IR register.
*/
void clearIR(uint_8 mask)
{
  IINCHIP_WRITE(IR, ~mask | getIR() ); // must be setted 0x10.
}

/**
@brief  This sets the maximum segment size of TCP in Active Mode), while in Passive Mode this is set by peer
*/
void setSn_MSS(SOCKET s, uint_16 Sn_MSSR)
{
  IINCHIP_WRITE( Sn_MSSR0(s), (uint_8)((Sn_MSSR & 0xff00) >> 8));
  IINCHIP_WRITE( Sn_MSSR1(s), (uint_8)(Sn_MSSR & 0x00ff));
}

void setSn_TTL(SOCKET s, uint_8 ttl)
{    
   IINCHIP_WRITE( Sn_TTL(s) , ttl);
}



/**
@brief  get socket interrupt status

These below functions are used to read the Interrupt & Soket Status register
*/
uint_8 getSn_IR(SOCKET s)
{
   return IINCHIP_READ(Sn_IR(s));
}


/**
@brief   get socket status
*/
uint_8 getSn_SR(SOCKET s)
{
   return IINCHIP_READ(Sn_SR(s));
}


/**
@brief  get socket TX free buf size

This gives free buffer size of transmit buffer. This is the data size that user can transmit.
User shuold check this value first and control the size of transmitting data
*/
uint_16 getSn_TX_FSR(SOCKET s)
{
  uint_16 val=0,val1=0;
  do
  {
    val1 = IINCHIP_READ(Sn_TX_FSR0(s));
    val1 = (val1 << 8) + IINCHIP_READ(Sn_TX_FSR1(s));
      if (val1 != 0)
    {
        val = IINCHIP_READ(Sn_TX_FSR0(s));
        val = (val << 8) + IINCHIP_READ(Sn_TX_FSR1(s));
    }
  } while (val != val1);
   return val;
}


/**
@brief   get socket RX recv buf size

This gives size of received data in receive buffer.
*/
uint_16 getSn_RX_RSR(SOCKET s)
{
  uint_16 val=0,val1=0;
  do
  {
    val1 = IINCHIP_READ(Sn_RX_RSR0(s));
    val1 = (val1 << 8) + IINCHIP_READ(Sn_RX_RSR1(s));
    if(val1 != 0)
    {
        val = IINCHIP_READ(Sn_RX_RSR0(s));
        val = (val << 8) + IINCHIP_READ(Sn_RX_RSR1(s));
    }
  } while (val != val1);
   return val;
}


/**
@brief   This function is being called by send() and sendto() function also.

This function read the Tx write pointer register and after copy the data in buffer update the Tx write pointer
register. User should read upper byte first and lower byte later to get proper value.
*/
void send_data_processing(SOCKET s, uint_8 *data, uint_16 len)
{
  if(len == 0)
  {
    printf("CH: %d Unexpected1 length 0\r\n", s);
    return;
  }

  uint_16 ptr = 0;
  uint_32 addrbsb = 0;
  ptr = IINCHIP_READ( Sn_TX_WR0(s) );
  ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ(Sn_TX_WR1(s));

  addrbsb = ((uint_32)ptr<<8) + (s<<5) + 0x10;
  wiz_write_buf(addrbsb, data, len);
  
  ptr += len;
  IINCHIP_WRITE( Sn_TX_WR0(s) ,(uint_8)((ptr & 0xff00) >> 8));
  IINCHIP_WRITE( Sn_TX_WR1(s),(uint_8)(ptr & 0x00ff));
}


/**
@brief  This function is being called by recv() also.

This function read the Rx read pointer register
and after copy the data from receive buffer update the Rx write pointer register.
User should read upper byte first and lower byte later to get proper value.
*/
void recv_data_processing(SOCKET s, uint_8 *data, uint_16 len)
{
  uint_16 ptr = 0;
  uint_32 addrbsb = 0;
  
  if(len == 0)
  {
    printf("CH: %d Unexpected2 length 0\r\n", s);
    return;
  }

  ptr = IINCHIP_READ( Sn_RX_RD0(s) );
  ptr = ((ptr & 0x00ff) << 8) + IINCHIP_READ( Sn_RX_RD1(s) );

  addrbsb = ((uint_32)ptr<<8) + (s<<5) + 0x18;
  wiz_read_buf(addrbsb, data, len);
  ptr += len;

  IINCHIP_WRITE( Sn_RX_RD0(s), (uint_8)((ptr & 0xff00) >> 8));
  IINCHIP_WRITE( Sn_RX_RD1(s), (uint_8)(ptr & 0x00ff));
}

void setSn_IR(uint_8 s, uint_8 val)
{
    IINCHIP_WRITE(Sn_IR(s), val);
}

void setSn_IMR(uint_8 s, uint_8 val)
{
    IINCHIP_WRITE(Sn_IMR(s), val);
}


