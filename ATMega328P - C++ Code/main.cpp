#ifndef F_CPU
#define F_CPU 16000000UL	//Definição da Frequência de Funcionamento do uProcessador -> 16MHz
#endif
//-------------------------------------------------------------------------------------------------------------------------------------
//----------------------------------------------- Definições de Bibliotecas-----------------------------------------------------
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/eeprom.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <util/twi.h>
#include <string.h>
#include "headers.h"
#include "fonte_5x7.h"

#define BAUD 9600
#define UBBRR_US F_CPU/16/BAUD-1
#define I2C_FREQ 100000UL

#define OLED_LARGURA  128
#define OLED_ALTURA 32
#define ENDER_OLED 0x3C				// Endereço I2C do display OLED

#define ENDERECO_RTC_LEITURA 0xD1	// Endereço I2C do RTC (leitura)
#define ENDERECO_RTC_ESCRITA 0xD0	// Endereço I2C do RTC (escrita)

#define ADC_FR 0x01
#define ADC_HUMID 0x00
#define LED_LEIT PORTB0
#define BOTAO_TOUCH PORTD2

typedef struct rtc_dados		//Estrutura referente a informações provenientes do RTC
{
	int segundos;
	int minutos;
	int horas;
	int diasemana;
	int dia;
	int mes;
	int ano;
	int temperatura;
} RTC_Dados;
const char* dias_Semana[7]={"Seg","Ter","Qua","Qui","Sex","Sab","Dom"};
RTC_Dados c_dados;
	
int FResp, flagCont, flagValorADC, flagLigarLeitura, flagPrimeiro;			//definição de flags

volatile long int contador0;
volatile uint16_t valorADC;
char buffer[10], relogio[10], data[20], temperatura[20], humidade[20];

char StringRecebida[64];
int StringID = 0;

//-------------------------------------------------------------------------------------------------------------------------------------
// ----------------------------------------------------- Funções de Conversão ---------------------------------------------------------
unsigned int conversor_8bit_int(uint8_t valor)			//Conversor de variavel de 8 bits para inteiro
{
	int valor_conv;
	valor_conv = ((valor>>4)*10)+(valor & 0xF);
	return valor_conv;
}
uint8_t conversor_int_8bit(int valor)					//Conversor de inteiro para variavel de 8 bits
{
	int valor_conv;
	valor_conv = (valor%10)|((valor/10)<<4);
	return valor_conv;
}
//-------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------I2C/TWI-------------------------------------------------------------------
void TWI_inic(void)														//Função de Inicialização de Comunicação I2C
{
	TWBR = ((F_CPU/I2C_FREQ) - 16) / 2;
}
void ligar_TWI(void)													//Ligar Comunicação I2C
{
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
}
void parar_TWI(void)													//Parar Comunicação I2C
{
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);
	while (TWCR & (1 << TWSTO));
}
void envia_TWI(uint8_t data)											//Enviar Dados Por I2C
{
	TWDR = data;
	TWCR=(1<<TWINT)|(1<<TWEN);
	while (!(TWCR & (1<<TWINT)));
}
uint8_t receb_TWI_ACK(int ackBit)										//Recebe dados Por I2C sem avançar posição no registo
{
	TWCR = (1<< TWINT) | (1<<TWEN) | (ackBit<<TWEA);
	while (!(TWCR & (1<<TWINT)));
	return TWDR;
}
//-------------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------- Comunicação USART------------------------------------------------------------
void USART_Inic(unsigned int ubrr)										//Função de Inicialização de Comunicação USART
{
	UBRR0H = (unsigned char)(ubrr >> 8);
	UBRR0L = (unsigned char)ubrr;
	UCSR0B = (1<<RXEN0)|(1<<TXEN0);
	UCSR0C = (1<<USBS0)|(1<<UCSZ00)|(1<<UCSZ01);
}
void USART_enviaCaractere(unsigned char ch)								//Função de envio de caracter por USART
{
	while (!(UCSR0A & (1<<UDRE0)));
	UDR0 = ch;
}
void USART_enviaString(char *s)											//Envio de conjuntos de caracteres por USART
{
	unsigned int i=0;
	while (s[i] != '\x0')
	{
		USART_enviaCaractere(s[i++]);
	};
	USART_enviaCaractere('\x0');
}
char USART_receberCaractere(void)										//Receber caracteres por USART
{
	while ( !(UCSR0A & (1<<RXC0)) );
	return UDR0;
}

void USART_receberString(void) {
	char receivedChar;
	
	while (1) {
		receivedChar = USART_receberCaractere();
		
		if (receivedChar == '\n' || receivedChar == '\0') {
			StringRecebida[StringID] = '\0';
			break;
			} else {
			StringRecebida[StringID] = receivedChar;
			StringID++;
			
			if (StringID >= 64 - 1) {
				StringRecebida[64 - 1] = '\0';
				break;
			}
		}
	}
}
//-------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------- ADC ---------------------------------------------------------------------------
void inicADC(void) {
	ADMUX |= (1 << REFS0);										// Tensão de Referencia AREF
	ADCSRA |= (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);		// Prescaler = 128
	ADCSRA |= (1 << ADEN);										// Habilitar o ADC
}
uint16_t lerADC(uint8_t porta) {
	ADMUX = (ADMUX & 0xF0) | (porta & 0x0F);					// Selecionar PC1 como entrada ADC
	ADCSRA |= (1 << ADSC);										// Iniciar a conversão ADC
	while (ADCSRA & (1 << ADSC));								// Aguardar a conclusão da conversão
	return ADC;
}
uint16_t contagemADC_FR(void){
	uint16_t valorADC_FR = 0;
	for(int i = 0; i<32;i++){
		valorADC_FR += lerADC(ADC_FR);
	}
	valorADC_FR = ((valorADC_FR/64)-120);
	if(valorADC_FR <= 0 || valorADC_FR >= 300){
		valorADC_FR = 0;
	}
	return (valorADC_FR);
}
uint16_t contagemADC_humid(void){
	uint16_t valorADC_Humid = 0;
	for(int i = 0; i<32;i++){
		valorADC_Humid += lerADC(ADC_HUMID);
	}
	return (valorADC_Humid/32);
}
//-------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------- Funções Display OLED ------------------------------------------------------------
void oledEnviaComando(uint8_t command) {
	ligar_TWI();
	envia_TWI(ENDER_OLED << 1); // Endereço do display OLED (shifted left 1 bit para adicionar o bit R/W = 0)
	envia_TWI(0x00);			// Co = 0 para comandos
	envia_TWI(command);
	parar_TWI();
}
void oledEnviarDados(uint8_t data) {
	ligar_TWI();
	envia_TWI(ENDER_OLED << 1); // Endereço do display OLED (shifted left 1 bit para adicionar o bit R/W = 0)
	envia_TWI(0x40);			// Co = 1 para dados
	envia_TWI(data);
	parar_TWI();
}

void oledLimpar(void) {
	for (uint8_t page = 0; page < 4; page++) {
		oledEnviaComando(0xB0 + page); // Define the current page
		oledEnviaComando(0x00); // Define the column address as 0
		oledEnviaComando(0x10); // Define the column address (bits 7-4) as 0

		for (uint8_t column = 0; column < 128; column++) {
			oledEnviarDados(0x00); // Send the value 0x00 (all pixels turned off) for each byte
		}
	}
} 
void oledInic(void) {
	oledEnviaComando(0xAE); // Desligar Display
	oledEnviaComando(0xD5); // Set Display Clock Divide Ratio/Oscillator Frequency
	oledEnviaComando(0x80); // Valor por defeito
	oledEnviaComando(0xA8); // Multiplex
	oledEnviaComando(0x1F); // 32 pixeis
	oledEnviaComando(0xD3); // Display Offset
	oledEnviaComando(0x00); // Sem Offset
	oledEnviaComando(0x40); // Linha Inicial
	oledEnviaComando(0x8D); // Set Charge Pump
	oledEnviaComando(0x14); // Enable charge pump
	oledEnviaComando(0x20); // Modo de Endereço da Memoria
	oledEnviaComando(0x00); // Endereço Horizontal
	oledEnviaComando(0xA1); // Set Segment Re-map (column address 127 is mapped to SEG0)
	oledEnviaComando(0xC8); // Set COM Output Scan Direction (remapped mode)
	oledEnviaComando(0xDA); // Set COM Pins Hardware Configuration
	oledEnviaComando(0x02); // Alternative COM pin configuration, disable COM left/right remap
	oledEnviaComando(0x81); // Set Contrast Control
	oledEnviaComando(0xCF); // Default value
	oledEnviaComando(0xD9); // Set Pre-charge Period
	oledEnviaComando(0xF1); // Default value
	oledEnviaComando(0xDB); // Set VCOMH Deselect Level
	oledEnviaComando(0x40); // Default value
	oledEnviaComando(0xA4); // Entire Display ON (resume to RAM content display)
	oledEnviaComando(0xA6); // Set Normal/Inverse Display (not inverted)
	oledLimpar();     // Clear the display with black color
	oledEnviaComando(0xAF); // Display ON
}
void oledImprimir(char *str, uint8_t x, uint8_t y) {
	uint8_t page = y / 8;
	uint8_t shift = y % 8;

	oledEnviaComando(0xB0 + page); // Set the current page
	oledEnviaComando(0x00 + (x & 0x0F)); // Set the column address (bits 3-0)
	oledEnviaComando(0x10 + ((x >> 4) & 0x0F)); // Set the column address (bits 7-4)

	while (*str) {
		uint8_t c = *str - 0x20; // Convert the ASCII character to an index in the fonte_5x7 array
		for (uint8_t i = 0; i < 5; i++) {
			uint8_t data = fonte_5x7[c][i]; // Read the pixel data for the current column of the character
			if (shift > 0) {
				data = (data << shift) | (fonte_5x7[c][i] >> (8 - shift));
			}
			oledEnviarDados(data); // Send the pixel data to the OLED display
		}
		oledEnviarDados(0x00); // Send an empty column of pixels between characters
		str++;
		x += 6;
		if (x >= OLED_LARGURA) {
			break;
		}
	}
}
void mostrarDados(void){
	if(c_dados.segundos%2 == 0){
		sprintf(relogio,"%02u : %02u",c_dados.horas,c_dados.minutos);
	}
	else{
		sprintf(relogio,"%02u   %02u",c_dados.horas,c_dados.minutos);
	}
	sprintf(data,"%s, %02u / %02u / %02u",dias_Semana[c_dados.diasemana-1],c_dados.dia,c_dados.mes,2000+(c_dados.ano));
	sprintf(humidade,"Humidade: %2u%%",contagemADC_humid()/10);
	sprintf(temperatura,"Temperatura: %u' C",c_dados.temperatura);
	
	oledImprimir(relogio, (OLED_LARGURA/2)-((sizeof(relogio)*5)/2), 0);
	oledImprimir(data, (OLED_LARGURA/2)-((sizeof(data)*5)/2), 8);
	oledImprimir(humidade, (OLED_LARGURA/2)-((sizeof(humidade)*5)/2), 16);
	oledImprimir(temperatura, (OLED_LARGURA/2)-((sizeof(temperatura)*5)/2), 24);
}
//-------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------------------------------- RTC ----------------------------------------------------
unsigned int RTC_Temperatura()										//Leitura de temperatura no modulo RTC
{
	int temperatura;
	ligar_TWI();
	envia_TWI(ENDERECO_RTC_ESCRITA);								//Indica o modo de escrita, para indicar em qual posição do registo do RTC, deverá ser iniciada a leitura.
	envia_TWI(0x11);												//Posição de registo referente à temperatura adquirida pelo módulo.
	ligar_TWI();
	envia_TWI(ENDERECO_RTC_LEITURA);
	temperatura = conversor_8bit_int(receb_TWI_ACK(0));					//Atribui à variável o valor inteiro convertido (de 8 bits), a leitura do registo e pára o avanço de posições de registo.
	return temperatura;													//Retorna o valor lido
}
void RTC_ler_relogio()												//Função referente à aquisição da data e hora, presentes no modulo RTC
{
	ligar_TWI();
	envia_TWI(ENDERECO_RTC_ESCRITA);								//Indica o modo de escrita, para indicar em qual posição do registo do RTC, deverá ser iniciada a leitura.
	envia_TWI(0x00);												//Posição inicial do registo
	ligar_TWI();
	envia_TWI(ENDERECO_RTC_LEITURA);								//Indica o modo de leitura
	c_dados.segundos = conversor_8bit_int(receb_TWI_ACK(1));		//ATribui à variável o valor inteiro convertido de 8 bits, a leitura do registo e avança uma posição de registo.
	c_dados.minutos = conversor_8bit_int(receb_TWI_ACK(1));			//...
	c_dados.horas = conversor_8bit_int(receb_TWI_ACK(1));			//...
	c_dados.diasemana = conversor_8bit_int(receb_TWI_ACK(1));		//...
	c_dados.dia = conversor_8bit_int(receb_TWI_ACK(1));				//...
	c_dados.mes = conversor_8bit_int(receb_TWI_ACK(1));				//...
	c_dados.ano = conversor_8bit_int(receb_TWI_ACK(0));
	c_dados.temperatura = RTC_Temperatura();				
	parar_TWI();													//Termina comunicação.
}
void RTC_Editar_relogio(int _segundos,int _minutos,int _horas,int _diasemana,int _dia,int _mes,int _ano) {			//Função referente à atribuição da data e hora no modulo RTC
	ligar_TWI();
	envia_TWI(ENDERECO_RTC_ESCRITA);								//Modo de escrita
	envia_TWI(0x00);												//Posição inicial
	envia_TWI(conversor_int_8bit(_segundos));						//Envia o valor 8 bits convertido de inteiro.
	envia_TWI(conversor_int_8bit(_minutos));						//Neste modo automaticamente é avançada uma posição do registo.
	envia_TWI(conversor_int_8bit(_horas));							//...
	envia_TWI(conversor_int_8bit(_diasemana));						//...
	envia_TWI(conversor_int_8bit(_dia));							//...
	envia_TWI(conversor_int_8bit(_mes));							//...
	envia_TWI(conversor_int_8bit(_ano));							//...
	parar_TWI();													//Termina comunicação.
}
//-------------------------------------------------------------------------------------------------------------------------------------
//---------------------------------------------------- Funções de Interrupções --------------------------------------------------------
ISR(USART_RX_vect) {							//Interrupção referente à receção de dados por USART
	char dados = UDR0;							//Quando o Wi-Fi está ligado, o microcontrolador ativa interrupção e coloca a flag a '1' para permitir leituras de valores do termístor
	if(dados == '{'){				
		flagLigarLeitura = 1;
	}
	if(dados == '}'){
		flagLigarLeitura = 0;
	}
	if(dados == '*'){
		USART_receberString();
		oledImprimir(StringRecebida,0,0);
		_delay_ms(1000);
		//RTC_Editar_relogio(30,58,11,2,5,9,23);
	}
}
ISR(TIMER0_COMPA_vect){						//Interrupção Referente ao Contador (incrementa a cada 1 ms)
	contador0++;
	if(contador0 >= 10000 && flagCont == 1){ //Ao fim de 10 segundos, se o ecra estiver a mostrar informações, limpar
		flagCont = 0;
	}
	if(contador0 >= 300000){				//300 segundos
		contador0 = 0;						//Ao fim de 100 segundos reinicia contador, para evitar overflow
	}
	if(contador0%100 == 0){					//A cada 100ms realiza contagem de ADC do Termístor
		flagValorADC = 1;
	}
}
ISR(INT0_vect) {							//Interrupção referente ao sensor touch que quando pressionado, ativa a flag e liga o ecrã
	contador0 = 0;
	flagCont = 1;
}
void aqueleswitch(void){
	switch(flagCont){
		case 1:{
			RTC_ler_relogio();
			mostrarDados();
			break;
		}
		case 2:{
			oledInic();
			flagCont = 0;
			break;
		}
		default: break;
	}
}
//------------------------------------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------------------------------------

int main(void){
	init();
	while(1){
		aqueleswitch();
		if(flagValorADC == 1 && flagLigarLeitura == 1){
			valorADC = contagemADC_FR();
			if(valorADC <= 20){
				flagValorADC = 0;
				if(flagPrimeiro == 1){
					sprintf(buffer,"<");
					USART_enviaString(buffer);
					flagPrimeiro = 0;
				}
			}
			else{
				PORTB |= (1 << LED_LEIT);
				memset(buffer,0,sizeof(buffer));
				if(flagPrimeiro == 0){
					sprintf(buffer,">");
					USART_enviaString(buffer);
					sprintf(buffer,"%u ",valorADC);
					USART_enviaString(buffer);
					flagPrimeiro = 1;
					}else{
					sprintf(buffer,"%u ",valorADC);
					USART_enviaString(buffer);
				}
				flagValorADC = 0;
			}
		}
		PORTB &= ~(1 << LED_LEIT);
	}
}
void init(void){
	cli();
	USART_Inic(UBBRR_US);
	TWI_inic();
	oledInic();
	inicADC();
	
	DDRB |= (1 << LED_LEIT);
	DDRD &= ~(1 << BOTAO_TOUCH);
	PORTD |= (1 << BOTAO_TOUCH);		//Ativar Pull-Up Interno PD2
	
	EICRA |= (1 << ISC01);
	EICRA &= ~(1 << ISC00);
	EIMSK |= (1 << INT0);
	
	UCSR0B |= (1 << RXCIE0);
	
	TCCR0A |= (1 << WGM01);		// Timer0								
	TCCR0B |= (1 << CS00) | (1 << CS01);						
	OCR0A = 249;												
	TIMSK0 |= (1 << OCIE0A);
	
	contador0 = flagCont = flagLigarLeitura = flagPrimeiro = 0;
	
	RTC_ler_relogio();	
	sei();
}