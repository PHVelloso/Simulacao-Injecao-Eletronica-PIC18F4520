//autor:Pedro Henrique Velloso da Silva
//Simulação de uma injeção eletrônica utilizando temperatura e rotação para tempo
//de injeção simulação feita no Proteus 

#include <pic18f4520.h> //o pic escolhido para este programa
#include <xc.h> //inclue a biblioteca XC8
#include <stdio.h>

#define _XTAL_FREQ 8000000 //define frequência para 8 MHz

#define RS LATBbits.LATB1 //pinos do LCD
#define EN LATBbits.LATB2
#define D4 LATDbits.LATD4
#define D5 LATDbits.LATD5
#define D6 LATDbits.LATD6
#define D7 LATDbits.LATD7

#pragma config WDT = OFF
#pragma config LVP = OFF
#pragma config PBADEN = OFF

unsigned int valor_adc; //guarda valor lido no adc
unsigned int temperatura; //guarda valor da temperatura calculada
unsigned char pulso_anterior = 0; //guarda estado anterior do RB0
unsigned long pulsos  = 0; //conta pulsos recebidos
unsigned int rpm = 0; //cria variável de rpm
unsigned int tempo_base; //cria variável de tempo base para o cálculo da injeção
unsigned int tempo_injecao; //o tempo de injeção que será executado
char texto1[16]; //texto para lcd linha 1, 16 caracteres
char texto2[16]; //texto para lcd linha 2, 16 caracteres

void ADC_init()
{

ADCON1 = 0b00001110; //configura ADCON1 e deixa AN0 como analógico e o restante digital
ADCON2 = 0b10101001;
// ADFM = 1 -> resultado justificado a direita
// ACQT = 010 -> tempo de aquisicao
// ADCS = 001 -> clock do ADC
ADCON0 = 0b00000001;
// Canal AN0 selecionado
// ADC ligado
}

unsigned int Ler_ADC()
   {
     GO_DONE = 1; //inicia conversão ADC
     while(GO_DONE); //espera conversão terminar
     return ((ADRESH << 8) + ADRESL); //junta ADRESH e ADRESL
   }

void LCD_Nibble(unsigned char nibble)
{
    D4 = (nibble >> 0) & 1;

    D5 = (nibble >> 1) & 1;

    D6 = (nibble >> 2) & 1;

    D7 = (nibble >> 3) & 1;

    EN = 1;

    __delay_us(50);

    EN = 0;
}

void LCD_Comando(unsigned char cmd)
{
    RS = 0;

    LCD_Nibble(cmd >> 4);

    LCD_Nibble(cmd & 0x0F);

    __delay_ms(2);
}
void LCD_Char(unsigned char data)
{
    RS = 1;

    LCD_Nibble(data >> 4);

    LCD_Nibble(data & 0x0F);

    __delay_us(20);
}

void LCD_String(const char *texto)
{
    while(*texto)
    {
        LCD_Char(*texto);

        texto++;
    }
}

void LCD_Init()
{
    TRISB1 = 0;

    TRISB2 = 0;

    TRISD4 = 0;
    TRISD5 = 0;
    TRISD6 = 0;
    TRISD7 = 0;

    __delay_ms(1);

    LCD_Nibble(0x03);

    LCD_Nibble(0x03);

    LCD_Nibble(0x03);

    LCD_Nibble(0x02);

    LCD_Comando(0x28);

    LCD_Comando(0x0C);

    LCD_Comando(0x06);

    //LCD_Comando(0x01);

    __delay_ms(1);
}
void LCD_Set_Cursor(unsigned char linha,
                    unsigned char coluna)
{
    unsigned char posicao;

    if(linha == 1)
    {
        posicao = 0x80 + coluna - 1;
    }
    else
    {
        posicao = 0xC0 + coluna - 1;
    }

    LCD_Comando(posicao);
}



void main (void)
{
TRISD = 0b00000000; //configura todas as portas em PORTD como saídas
TRISB0 = 1; //Deixa RB0 como entrada

ADC_init();			//inicializa ADC
LCD_Init();

while(1)
   {


valor_adc = Ler_ADC();      // Le potenciometro
// Formula da temperatura:
// 20 ate 110 graus

temperatura = 20 + ((unsigned long)valor_adc * 90 / 1023);
/* EXEMPLOS:
ADC = 0
temperatura = 20°C

ADC = 512
temperatura ? 65°C

ADC = 1023
temperatura = 110°C
*/

//------------------------Leitura do DCLOCK--------------------
  // Detecta borda de subida
        pulsos = 0;

// Conta pulsos durante 1 segundo
for(int i = 0; i < 100; i++)
      {
// Detecta borda de subida
if(PORTBbits.RB0 == 1 &&
pulso_anterior == 0)
	 {
pulsos++;
      }
pulso_anterior = PORTBbits.RB0;
__delay_ms(1);
	 }
rpm = pulsos * 600;


//================ TEMPO BASE ================

// Quanto mais frio, maior o tempo

tempo_base = 120 - temperatura;

//============== CORRECAO RPM ===============

tempo_injecao = tempo_base + (rpm / 100);

// Liga os 4 bicos
LATDbits.LATD0 = 1;
LATDbits.LATD1 = 1;
LATDbits.LATD2 = 1;
LATDbits.LATD3 = 1;

// Mantem ligados pelo tempo calculado
for(int i = 0; i < tempo_injecao; i++)
	    {
__delay_ms(1);
	    }

// Desliga os bicos
LATDbits.LATD0 = 0;
LATDbits.LATD1 = 0;
LATDbits.LATD2 = 0;
LATDbits.LATD3 = 0;
__delay_ms(1); 	// Pequeno atraso

LCD_Set_Cursor(1,1);
sprintf(texto1, "RPM:%4u ", rpm);
LCD_Set_Cursor(1,1);
LCD_String(texto1);

sprintf(texto2, "TEMP:%3uC ", temperatura);
LCD_Set_Cursor(2,1);
LCD_String(texto2);
   }

}
