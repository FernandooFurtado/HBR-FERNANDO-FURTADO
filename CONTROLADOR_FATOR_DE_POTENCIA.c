#include <stdio.h>
#include <string.h>
#include <math.h> 
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/timer.h"
#include "hardware/adc.h"
#include "pico/binary_info.h"
#include "ssd1306.h"
#include "hardware/uart.h"

#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define BOT_1 5
#define BOT_2 6
#define BOT_3 22
#define CLK_4017 16
#define BANCO1 8
#define BANCO2 9
#define BANCO3 4
#define BANCO4 20
#define BANCO5 19

uint16_t P_APARENTE = 0;
uint16_t P_ATIVA = 0;  
uint16_t P_REATIVA = 0;
uint16_t I_TOTAL = 0;
uint16_t I_R = 0;
uint16_t I_S = 0;
uint16_t I_T = 0; 
uint16_t V_R = 0;
uint16_t V_S = 0;
uint16_t V_T = 0;
uint16_t V_fase = 0;
uint8_t FP_VAR = 0; 
float fp_min = 0.92;
float fp_calc;
float RAIZ_3;

uint8_t BANCO_CAP = 0;
uint8_t TIME_BANCO = 0;
uint16_t V_VAR = 0; 
uint8_t menu = 0;
uint8_t flag_1,flag_2,flag_3 = 0;
uint8_t i,j,k,l,y,m,n=0;
uint ADC_TENSAO1,ADC_TENSAO2,ADC_TENSAO3,ADC_TENSAO4,ADC_TENSAO5,ADC_TENSAO6;
uint v_tensao;
uint var_cap;
uint time_acionamento;

char mensagem_VR[8];
char mensagem_VS[8];
char mensagem_VT[8];
char mensagem_IR[8];
char mensagem_IS[8];
char mensagem_IT[8];
char mensagem_tensao[8];
char mensagem_tensao_fase[8];

void leitura_bot(void);
void clock_4017 (void);
void correcao_fp (void);

char *tela15[] = {
  "  CONTROLADOR  ",
  "  DE FOTOR DE  ",
  "   POTENCIA    ",
  "EMBARCATECH2025"};

char *tela1[] = {
  "   CONFIGURAR  ",
  "1ZFP           ",
  "2ZTENSAO       ",
  "3ZBANCO DE CAP."};

char *tela2[] = {
  "4ZTEMPO  DE    ",
  "ACIONAMENTO    ",
  "               ",
  "               "};

char *tela3[] = {
  "IRZ     VRZ    ",
  "ISZ     VSZ    ",
  "ITZ     VTZ    ",
  "FPZ092  VNZ    "};

char *tela4[] = {
  "BANCO1Z 3KVar  ",
  "BANCO2Z 12KVar ",
  "BANCO3Z 45KVar ",
  "BANCO3Z 45KVar "};     

char *tela5[] = {
  "     FP FIXO    ",
  "     MINIMO     ",
  "      092       ",
  "                "};

char *tela6[] = {
  "    TENSAO     ",
  "     220V      ",
  "               ",
  "      RST      "}; 

char *tela7[] = {
  "    TENSAO     ",
  "     380V      ",
  "               ",
  "      RST      "};

char *tela8[] = {
  " BANCOS DE CAP.",
  "               ",
  "   BANCO 1    ",
  "    3kVAR     "};

char *tela9[] = {
  " BANCOS DE CAP.",
  "               ",
  "   BANCO 2    ",
  "    3kVAR     "}; 

char *tela10[] = {
  " BANCOS DE CAP.",
  "               ",
  "   BANCO 3    ",
  "    3kVAR     "};  

char *tela11[] = {
  " TEMPO DE      ",
  " ACIONAMENTO   ",
  "               ",
  "  03 SEGUNDOS  "};

char *tela12[] = {
  " TEMPO DE      ",
  " ACIONAMENTO   ",
  "               ",
  "  05 SEGUNDOS  "};

char *tela13[] = {
  " TEMPO DE      ",
  " ACIONAMENTO   ",
  "               ",
  "  10 SEGUNDOS  "};  
  
 char *tela14[] = {
  "               ",
  "   CALIBRADO   ",
  "               ",
  "               "};

int main()
{
    stdio_init_all();
    gpio_init(BOT_1);
    gpio_set_dir(BOT_1, GPIO_IN);
    gpio_pull_up(BOT_1);

    gpio_init(BOT_2);
    gpio_set_dir(BOT_2, GPIO_IN);
    gpio_pull_up(BOT_2);

    gpio_init(BOT_3);
    gpio_set_dir(BOT_3, GPIO_IN);
    gpio_pull_up(BOT_3);

    gpio_init(CLK_4017);
    gpio_set_dir(CLK_4017, GPIO_OUT);

    gpio_init(CLK_4017);
    gpio_set_dir(BANCO1, GPIO_OUT);
    gpio_init(BANCO1);
    gpio_set_dir(BANCO2, GPIO_OUT);
    gpio_init(BANCO2);
    gpio_set_dir(BANCO3, GPIO_OUT);
    gpio_init(BANCO3);
    gpio_set_dir(BANCO4, GPIO_OUT);
    gpio_init(BANCO4);
    gpio_set_dir(BANCO5, GPIO_OUT);
    gpio_init(BANCO5);

    adc_init();
    adc_gpio_init(28);
    adc_select_input(2); 

    i2c_init(i2c1, SSD1306_I2C_CLK * 1000);
    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C);
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C);
    gpio_pull_up(I2C_SDA);
    gpio_pull_up(I2C_SCL);

    SSD1306_init();

 // Initialize render area for entire frame (SSD1306_WIDTH pixels by SSD1306_NUM_PAGES pages)
    struct render_area frame_area = {
        start_col : 0,
        end_col : SSD1306_WIDTH - 1,
        start_page : 0,
        end_page : SSD1306_NUM_PAGES - 1
    };

    calc_render_area_buflen(&frame_area);

    // zero the entire display
    uint8_t buf[SSD1306_BUF_LEN];
    memset(buf, 0, SSD1306_BUF_LEN);
    render(buf, &frame_area);

    SSD1306_scroll(true);
    sleep_ms(5000);
    SSD1306_scroll(false);   
    
     for (i = 0; i < count_of(tela15); i++)
      {
          WriteString(buf, 5, y, tela15[i]);
          y += 8;
      }              
      render(buf, &frame_area);
      i = 0;
        

    while(1)
    {
        leitura_bot();
        clock_4017();
        correcao_fp();
        switch (menu)
        {
            case 1 : for (i = 0; i < count_of(tela1); i++)
                    {
                        WriteString(buf, 5, y, tela1[i]);
                        y += 8;
                    }              
                    render(buf, &frame_area);
                    i = 0;
                    break;

            case 2 : for (j = 0; j < count_of(tela2); j++)
                    {
                        WriteString(buf, 5, y, tela2[j]);
                        y += 8;
                    }
                    render(buf, &frame_area);
                    j = 0;
                    break;
            case 3 : for ( l = 0; l < count_of(tela3); l++)
                    {
                        WriteString(buf, 5, y, tela3[l]);
                        y += 8;
                    }
                    l = 0;
                    sprintf (mensagem_IR,"%d",ADC_TENSAO1);
                    sprintf (mensagem_IS,"%d",ADC_TENSAO2);
                    sprintf (mensagem_IT,"%d",ADC_TENSAO3);
                    sprintf (mensagem_tensao,"%d",V_VAR);
                    sprintf (mensagem_tensao_fase,"%d",V_fase);
                    
                    WriteString(buf,25,5,mensagem_IR);
                    WriteString(buf,25,15,mensagem_IS);
                    WriteString(buf,25,20,mensagem_IT);

                    WriteString(buf,90,5,mensagem_tensao_fase);
                    WriteString(buf,90,15,mensagem_tensao_fase);
                    WriteString(buf,90,20,mensagem_tensao_fase);
                    WriteString(buf,90,24,mensagem_tensao);
                    render(buf, &frame_area);
                    break;   
            
            case 4 : for ( l = 0; l < count_of(tela4); l++)
                    {
                        WriteString(buf, 5, y, tela4[l]);
                        y += 8;
                    }
                    render(buf, &frame_area);
                    l = 0;
                    break;         

            case 5 : for (k = 0; k < count_of(tela5); k++)
                    {
                        WriteString(buf, 5, y, tela5[k]);
                        y += 8;
                    }
                    render(buf, &frame_area);
                    k = 0;
                    break;

            case 6 : for ( l = 0; l < count_of(tela6); l++)
                    {
                        WriteString(buf, 5, y, tela6[l]);
                        y += 8;
                    }
                    render(buf, &frame_area); 
                    l = 0;
                    break;
            case 7 : for (i = 0; i < count_of(tela7); i++)
                    {
                        WriteString(buf, 5, y, tela7[i]);
                        y += 8;
                    }              
                    render(buf, &frame_area);
                    i = 0;
                    break;

            case 8 : for (j = 0; j < count_of(tela8); j++)
                    {
                        WriteString(buf, 5, y, tela8[j]);
                        y += 8;
                    }
                    render(buf, &frame_area);
                    j = 0;
                    break; 

            case 9 : for (k = 0; k < count_of(tela9); k++)
                    {
                        WriteString(buf, 5, y, tela9[k]);
                        y += 8;
                    }
                    render(buf, &frame_area);
                    k = 0;
                    break;

            case 10 : for ( l = 0; l < count_of(tela10); l++)
                    {
                        WriteString(buf, 5, y, tela10[l]);
                        y += 8;
                    }
                    render(buf, &frame_area); 
                    l = 0;
                    break;
            case 11 : for ( l = 0; l < count_of(tela11); l++)
                    {
                        WriteString(buf, 5, y, tela11[l]);
                        y += 8;
                    }
                    render(buf, &frame_area); 
                    l = 0;
                    break;  
            
            case 12 : for ( l = 0; l < count_of(tela12); l++)
                    {
                        WriteString(buf, 5, y, tela12[l]);
                        y += 8;
                    }
                    render(buf, &frame_area);  
                    l = 0;
                    break;
            case 13 : for ( l = 0; l < count_of(tela13); l++)
                    {
                        WriteString(buf, 5, y, tela13[l]);
                        y += 8;
                    }
                    render(buf, &frame_area);
                    l = 0;
                    break;   
            
            case 14 : for ( l = 0; l < count_of(tela14); l++)
                    {
                        WriteString(buf, 5, y, tela14[l]);
                        y += 8;
                    }
                    render(buf, &frame_area);
                    l = 0;
                    break;
        } 
    }
    
}
///////////////////////////////////////////////////////////////////////////
void leitura_bot(void)
{
    if((gpio_get(BOT_1)==0) && ((flag_1 ==0)))                  // tratamento de deboucing  com flag
    {
       sleep_ms(80);  
      if(gpio_get(BOT_1)==0)                                    // tratamento de deboucing  com flag
      {
          flag_1=1;
          if((gpio_get(BOT_1)==0) && ((flag_1 ==1)))            // tratamento de deboucing  com flag
          {
            menu ++;
            if(menu > 4)
            {
               menu = 1;
            }
          }
      }
    }
    if((gpio_get(BOT_1)==1) && ((flag_1 ==1)))                  // baixando a flag do deboucing  com flag
    {
        flag_1=0;
    }
///////////////////////////////////////////////////////////////////////////
    if((gpio_get(BOT_2)==0) && ((flag_2 ==0)))                  // tratamento de deboucing  com flag
    {
       sleep_ms(80);  
      if(gpio_get(BOT_2)==0)                                    // tratamento de deboucing  com flag
      {
          flag_2=1;
          if((gpio_get(BOT_2)==0) && ((flag_2 ==1)))            // tratamento de deboucing  com flag
          {
            menu++;
            if ((menu == 1)|| (menu == 2) || (menu == 3) || (menu == 4))
            {
                menu = 5 ;               
            };
            if(menu > 14)
            {
               menu = 5;
            };
            printf (" %d\n",menu);
          }
      }
    }
    if((gpio_get(BOT_2)==1) && ((flag_2 ==1)))                  // baixando a flag do deboucing  com flag
    {
        flag_2=0;
    }
///////////////////////////////////////////////////////////////////////////   
    if((gpio_get(BOT_3)==0) && ((flag_3 ==0)))                  // tratamento de deboucing  com flag
    {
       sleep_ms(80);  
      if(gpio_get(BOT_3)==0)                                    // tratamento de deboucing  com flag
      {
          flag_3=1;
          if((gpio_get(BOT_3)==0) && ((flag_3 ==1)))            // tratamento de deboucing  com flag
          {
            if((gpio_get(BOT_3)==0) && (menu == 6)) 
            {
                V_VAR = 220;
                V_fase = 127;
                printf (" %d\n",V_VAR);
                
            }
            if((gpio_get(BOT_3)==0) && (menu == 7)) 
            {
                V_VAR = 380;
                V_fase = 220;
                printf (" %d\n",V_VAR);
            }
            //////////////////////////////////////
            if((gpio_get(BOT_3)==0) && (menu == 8)) 
            {
                BANCO_CAP = 1;
                printf (" %d\n",BANCO_CAP);
            }
            if((gpio_get(BOT_3)==0) && (menu == 9)) 
            {
                BANCO_CAP = 2;
                printf (" %d\n",BANCO_CAP);
            }
            if((gpio_get(BOT_3)==0) && (menu == 10)) 
            {
                BANCO_CAP = 3;
                printf (" %d\n",BANCO_CAP);
            }
            //////////////////////////////////////
            if((gpio_get(BOT_3)==0) && (menu == 11)) 
            {
                TIME_BANCO = 1;
                time_acionamento = 3000;
                printf (" %d\n",TIME_BANCO);
            }
            if((gpio_get(BOT_3)==0) && (menu == 12)) 
            {
                TIME_BANCO = 2;
                time_acionamento = 5000;
                printf (" %d\n",TIME_BANCO);
            }
            if((gpio_get(BOT_3)==0) && (menu == 13)) 
            {
                TIME_BANCO = 3;
                time_acionamento = 10000;
                printf (" %d\n",TIME_BANCO);
            }
          }
      }
    }
    if((gpio_get(BOT_3)==1) && ((flag_3 ==1)))                  // baixando a flag do deboucing  com flag
    {
        flag_3=0;
    }
}
///////////////////////////////////////////////////////////////////////////
void clock_4017 (void)
{
    ADC_TENSAO1 = adc_read();
    sleep_ms(30);
    printf (" %d\n",ADC_TENSAO1);
    gpio_put(CLK_4017,1);
    gpio_put(CLK_4017,0);

    ADC_TENSAO2 = adc_read();
    sleep_ms(30);
    printf (" %d\n",ADC_TENSAO2);
    gpio_put(CLK_4017,1);
    gpio_put(CLK_4017,0);

    ADC_TENSAO3 = adc_read();
    sleep_ms(30);
    printf (" %d\n",ADC_TENSAO3);
    gpio_put(CLK_4017,1);
    gpio_put(CLK_4017,0);

    ADC_TENSAO4 = adc_read();
    sleep_ms(30);
    printf (" %d\n",ADC_TENSAO4);
    gpio_put(CLK_4017,1);
    gpio_put(CLK_4017,0);

    ADC_TENSAO5 = adc_read();
    sleep_ms(30);
    printf (" %d\n",ADC_TENSAO5);
    gpio_put(CLK_4017,1);
    gpio_put(CLK_4017,0);

    ADC_TENSAO6 = adc_read();
    sleep_ms(30);
    printf (" %d\n\n\n",ADC_TENSAO6);
    gpio_put(CLK_4017,1);
    gpio_put(CLK_4017,0);
}
///////////////////////////////////////////////////////////////////////////
void correcao_fp (void)
{
  I_TOTAL = P_APARENTE * (V_VAR * RAIZ_3 );
  P_ATIVA = V_VAR * I_TOTAL;
  P_APARENTE = P_ATIVA / fp_min;
  P_REATIVA = P_APARENTE * fp_min;
  fp_calc = P_ATIVA / P_APARENTE;

  if(ADC_TENSAO1<20)
  {
    I_R = 1 ;
  }
  else if((ADC_TENSAO1>40) && (ADC_TENSAO1 < 90))
  {
     I_R = 2 ;
  }
  else if((ADC_TENSAO1>95) && (ADC_TENSAO1 < 120))
  { 
     I_R = 3 ;
  }
  else if((ADC_TENSAO1>125) && (ADC_TENSAO1 < 145))
  {
    I_R = 4 ;
  }
  else if((ADC_TENSAO1>150) && (ADC_TENSAO1 < 180))
  {
    I_R = 5 ;
  }
  else if((ADC_TENSAO1>185) && (ADC_TENSAO1 < 220))
  {
    I_R = 6 ;
  }


  if(ADC_TENSAO2<20)
  {
    I_S = 1 ;
  }
  else if((ADC_TENSAO2>40) && (ADC_TENSAO2 < 90))
  {
     I_S = 2 ;
  }
  else if((ADC_TENSAO2>95) && (ADC_TENSAO2 < 120))
  { 
     I_S = 3 ;
  }
  else if((ADC_TENSAO2>125) && (ADC_TENSAO2 < 145))
  {
    I_S = 4 ;
  }
  else if((ADC_TENSAO2>150) && (ADC_TENSAO2 < 180))
  {
    I_S = 5 ;
  }
  else if((ADC_TENSAO2>185) && (ADC_TENSAO2 < 220))
  {
    I_S = 6 ;
  }

  if(ADC_TENSAO3<20)
  {
    I_T = 1 ;
  }
  else if((ADC_TENSAO3>40) && (ADC_TENSAO3 < 90))
  {
     I_T = 2 ;
  }
  else if((ADC_TENSAO3>95) && (ADC_TENSAO3 < 120))
  { 
     I_T = 3 ;
  }
  else if((ADC_TENSAO3>125) && (ADC_TENSAO3 < 145))
  {
    I_T = 4 ;
  }
  else if((ADC_TENSAO3>150) && (ADC_TENSAO3 < 180))
  {
    I_T = 5 ;
  }
  else if((ADC_TENSAO3>185) && (ADC_TENSAO3 < 220))
  {
    I_T = 6 ;
  }



  if(ADC_TENSAO4<20)
  {
    V_R = 1 ;
  }
  else if((ADC_TENSAO4>40) && (ADC_TENSAO4 < 90))
  {
    V_R = 2 ;
  }
  else if((ADC_TENSAO4>95) && (ADC_TENSAO4 < 120))
  { 
    V_R = 3 ;
  }
  else if((ADC_TENSAO4>125) && (ADC_TENSAO4 < 145))
  {
    V_R = 4 ;
  }
  else if((ADC_TENSAO4>150) && (ADC_TENSAO4 < 180))
  {
    V_R = 5 ;
  }
  else if((ADC_TENSAO4>185) && (ADC_TENSAO4 < 220))
  {
    V_R = 6 ;
  }


  if(ADC_TENSAO5<20)
  {
    V_S = 1 ;
  }
  else if((ADC_TENSAO5>40) && (ADC_TENSAO5 < 90))
  {
    V_S= 2 ;
  }
  else if((ADC_TENSAO5>95) && (ADC_TENSAO5 < 120))
  { 
    V_S = 3 ;
  }
  else if((ADC_TENSAO5>125) && (ADC_TENSAO5 < 145))
  {
    V_S = 4 ;
  }
  else if((ADC_TENSAO5>150) && (ADC_TENSAO5 < 180))
  {
    V_S = 5 ;
  }
  else if((ADC_TENSAO5>185) && (ADC_TENSAO5 < 220))
  {
    V_S = 6 ;
  }

  if(ADC_TENSAO6<20)
  {
    V_T = 1 ;
  }
  else if((ADC_TENSAO6>40) && (ADC_TENSAO6 < 90))
  {
    V_T = 2 ;
  }
  else if((ADC_TENSAO6>95) && (ADC_TENSAO6 < 120))
  { 
    V_T = 3 ;
  }
  else if((ADC_TENSAO6>125) && (ADC_TENSAO6 < 145))
  {
    V_T = 4 ;
  }
  else if((ADC_TENSAO6>150) && (ADC_TENSAO6 < 180))
  {
    V_T = 5 ;
  }
  else if((ADC_TENSAO6>185) && (ADC_TENSAO6 < 220))
  {
    V_T = 6 ;
  }

  if (fp_calc < 0.92 )
  {
    sleep_ms(3000);
    gpio_put(BANCO1,1);
  }
  if (fp_calc < 0.88 )
  {
    sleep_ms(3000);
    gpio_put(BANCO2,1);
  }
  if (fp_calc < 0.85 )
  {
    sleep_ms(3000);
    gpio_put(BANCO3,1);
  }
  if (fp_calc < 0.83 )
  {
    sleep_ms(3000);
    gpio_put(BANCO4,1);
  }
  if (fp_calc < 0.80 )
  {
    sleep_ms(3000);
    gpio_put(BANCO5,1);
  }
  if(fp_calc >= 0.92)
  { 
    gpio_put(BANCO1,0);
    gpio_put(BANCO2,0);
    gpio_put(BANCO3,0);
    gpio_put(BANCO4,0);
    gpio_put(BANCO5,0);
  }
}
