#include "main.h"

#define UART_BUF_SIZE 128
//
#define USART_BAUDRATE 9600 // Desired Baud Rate
#define BAUD_PRESCALER (((F_CPU / (USART_BAUDRATE * 16UL))) - 1)
#define ASYNCHRONOUS (0<<UMSEL00) // USART Mode Selection
#define DISABLED    (0<<UPM00)
#define EVEN_PARITY (2<<UPM00)
#define ODD_PARITY  (3<<UPM00)
#define PARITY_MODE  DISABLED // USART Parity Bit Selection
#define ONE_BIT (0<<USBS0)
#define TWO_BIT (1<<USBS0)
#define STOP_BIT ONE_BIT      // USART Stop Bit Selection
#define FIVE_BIT  (0<<UCSZ00)
#define SIX_BIT   (1<<UCSZ00)
#define SEVEN_BIT (2<<UCSZ00)
#define EIGHT_BIT (3<<UCSZ00)
#define DATA_BIT   EIGHT_BIT  // USART Data Bit Selection

static char uart_in_buf[UART_BUF_SIZE];
volatile unsigned char head;
volatile unsigned char tail;

void init_uart()
{
	UBRR0H = BAUD_PRESCALER >> 8;
	UBRR0L = BAUD_PRESCALER;

	// Set Frame Format
	UCSR0C = ASYNCHRONOUS | PARITY_MODE | STOP_BIT | DATA_BIT;
    /* Enable USART receiver and transmitter and receive complete interrupt */
    UCSR0B = _BV(RXCIE0)|(1<<RXEN0)|(1<<TXEN0);

	head = tail = 0;
}

//*******************************************************************************************************************

// USART Receiver interrupt service routine
ISR(USART_RX_vect)
{
	char data;
    cli();
	data=UDR0;
	if((UCSR0A & ((1 << FE0) | (1 << DOR0))) == 0)
	//if ((UCSR0A & ((1<<FE0) | (1<<DOR0)))==0)
	{
		uart_in_buf[head] = data;
		head = (head+1)&(UART_BUF_SIZE-1);
	}
	sei();
}



//*******************************************************************************************************************

char get_byte_from_queue(void)
{
	char data;
	data = uart_in_buf[tail];
	tail = (tail+1)&(UART_BUF_SIZE-1);
	return data;
}

//*******************************************************************************************************************
uint8_t is_queue_not_empty(void)
 {
	if(head!=tail)
		return TRUE;
	else
		return FALSE;
}

//*******************************************************************************************************************

void uart_send_buf(char* buf, char len)
{
	while(len--)
	{
		while( (UCSR0A & (1<<UDRE0)) == 0 ); // ждем готовности буферного регистра
		UDR0 = *(buf++);
	}
}

//*******************************************************************************************************************

void uart_send_str(char *str)
{
	while(*str)
	{
		while( (UCSR0A & (1<<UDRE0)) == 0 ); // ждем готовности буферного регистра
		UDR0 = *str++;
	}
}

//*******************************************************************************************************************

void uart_send_str_p(const char *progmem_s)
{
	register char c;
    uart_flush();
	while ((c = pgm_read_byte(progmem_s++))) {
		uart_send_byte(c);
	}
}
//*******************************************************************************************************************

void uart_send_byte(char data)
{
	while( (UCSR0A & (1<<UDRE0)) == 0 ){}; // ждем готовности буферного регистра
	UDR0 = data;
}

//*******************************************************************************************************************

void reset_uart_queue(void)
{
	tail = head = 0;
}

void uart_flush(void)
{
	unsigned char dummy;
	uint32_t time_stamp = get_time_s() + 3;
	while ( UCSR0A & (1<<RXC0) )
	{
        if(get_time_s() > time_stamp)
            return;
		dummy = UDR0;
		dummy = dummy;
	}
}

//void uart_send_word(uint16_t f_data)
//{
//    char buffer [12];
//    utoa(f_data, buffer, 10);
//    uart_send_str(buffer);
//}
//
//void uart_send_int(uint8_t f_data)
//{
//	int x,y,i;
//	unsigned char c[10];
//	y=f_data;
//	if(y==0)
//	{
//		uart_send_byte(0x30);
//	}
//	else
//	{
//		for(i=0;y>0;i++)
//		{
//			x=y%10;
//			y=y/10;
//			c[i]=x+'0';
//		}
//
//		i--;
//		for(;i>=0;i--)
//		{
//			uart_send_byte(c[i]);
//			_delay_ms(10);
//		}
//	}
//}
//
//void uart_send_float(float f_data)
//{
//	unsigned char f_buf;
//	unsigned int temp;
//	temp=(int)f_data;
//	uart_send_int(temp);
//	f_data=(f_data-temp);
//	if(f_data>0)
//	{
//		uart_send_byte('.');
//		int i;
//		for(i=0;i<4;i++)
//		{
//			f_data=f_data*10;
//			temp=(int)f_data;
//			temp=(temp%10);
//			if(temp==0)
//			{
//				uart_send_byte(0x30);
//			}
//			else
//			{
//				f_buf=temp+'0';
//				uart_send_byte(f_buf);
//			}
//		}
//	}
//}







