#ifndef HEADERS_H 
#define HEADERS_H

unsigned int conversor_8bit_int(uint8_t valor);
uint8_t conversor_int_8bit(int valor);
void TWI_inic(void);
void ligar_TWI(void);
void parar_TWI(void);
void envia_TWI(uint8_t data);
uint8_t receb_TWI_ACK(int ackBit);
void USART_Inic(unsigned int ubrr);
void USART_enviaCaractere(unsigned char ch);
void USART_enviaString(char *s);
char USART_receberCaractere(void);
void inicADC(void);
uint16_t lerADC(uint8_t porta);
uint16_t contagemADC_FR(void);
uint16_t contagemADC_humid(void);
void mostrarLeituraADC(uint16_t valorADC);
void mostrarFreqResp(float valorFR,float tempo);
void oledEnviaComando(uint8_t command);
void oledEnviarDados(uint8_t data);
void oledLimpar(void);
void oledInic(void);
void oledImprimir(char *str, uint8_t x, uint8_t y);
void mostrarDados(void);
unsigned int RTC_Temperatura();
void RTC_ler_relogio();
void RTC_Editar_relogio(int _segundos,int _minutos,int _horas,int _diasemana,int _dia,int _mes,int _ano);
void esperarMaximo(void);
void esperarMinimo(void);
ISR (TIMER0_COMPA_vect);
ISR(TIMER1_COMPA_vect);
int main(void);
void init(void);
void verificarWIFI(void);

#endif