#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"
#include "fsm.h"

#define LED 2 // Led va asociado al 2
#define PULL_UP_0 0
#define S_INFRA 15
#define freq portTICK_RATE_MS
#define PERIOD_TICK 80/freq
volatile int tiempo=0;
#define SEGUNDO 1000/freq
#define ANTIRREBOTE 100/freq
int flag_armar=0;
volatile int c=0;
int codigo_introducido[3]={0,0,0};
int codigo_correcto[3]={1,2,3}; //Nuestro código es 1,2,3
int indice = 0;
volatile int tiempo_sin_pulsar=0;
volatile int tiempo_reset=0;
int t_antirrebote=0;





/*
Enumeración de los estados de la máquina de Mealy
*/
 enum fsm_state {
APAGADO,
ENCENDIDO,
ARMADO,
NO_ARMADO,
};

void imprime(fsm_t* this){
  printf("%s %d\n",'valor de c:',c);
  printf("%s  %d \n",'valor de indice:', indice);


}

  /*
Definimos la matriz de transicion que tiene como formato
{ESTADO ACTUAL, funcion de comprobacion, ESTADO_SIGUIENTE, funcion de actualizacion}
  */


int pulsacion(fsm_t* this){
  //printf("%s  \n","entró");

  if(!GPIO_INPUT_GET(0)){



    if(xTaskGetTickCount() > t_antirrebote){


      t_antirrebote=xTaskGetTickCount() + ANTIRREBOTE;
      tiempo_sin_pulsar=xTaskGetTickCount() + SEGUNDO;
      tiempo_reset=xTaskGetTickCount() + 3*SEGUNDO;
      c++;
      printf("%s  %d\n","Valor de c:" , c);
      printf("%s  %d\n","Valor de indice:" , indice);
      return 1;

    }
  }return 0;
}

int codigo_c(fsm_t* this){
  int val=0;
  int i;

for(i=0;i<3;i++){
  if(codigo_introducido[i] != codigo_correcto[i]){
    val=0;
    break;
  }val=1;
}
return val;
}





  /*Método que mide si ha pasado más de un segundo entre pulsación
  */

int tiempo_maximo(fsm_t* this){


if(xTaskGetTickCount() > tiempo_sin_pulsar){

    if(xTaskGetTickCount() > tiempo_reset){
      indice=0;
      printf("%s  %d \n",'valor de indice:', indice);

    }

return 1;

}
return 0;

}



void clean_codigo(fsm_t* this){


  if(c > 9){
    codigo_introducido[0]=0; codigo_introducido[1]=0;codigo_introducido[2]=0;
    indice=0;
    c=0;
}
else if (c==0){}
else{


 if(indice<3){

   codigo_introducido[indice]=c;
   c=0;

    if(indice==2){
        indice=0;
      }
   else{
     indice++;
   }

 }
printf("%s  %d\n","Valor de indice:" , indice);

}



}









void armar(fsm_t* this){
flag_armar=1; //activa el flag de alarma
codigo_introducido[0]=0; codigo_introducido[1]=0;codigo_introducido[2]=0;



}

void desarmar(fsm_t* this){
flag_armar=0; //desactiva el flag de alarma
GPIO_OUTPUT_SET(2,1);
codigo_introducido[0]=0; codigo_introducido[1]=0;codigo_introducido[2]=0;



}

int comprobar_presencia_1(fsm_t* this){

if( GPIO_INPUT_GET(S_INFRA) ){

return 1;

}
return 0;

  }

  int comprobar_presencia_2(fsm_t* this){

  if( !GPIO_INPUT_GET(S_INFRA) ){

  return 1;

  }
  return 0;

    }


  void encender(fsm_t* this){

    GPIO_OUTPUT_SET(2,0);
  }

  void apagar(fsm_t* this){

    GPIO_OUTPUT_SET(2,1);
  }






static fsm_trans_t matriz_transicion1[] = {
{NO_ARMADO, pulsacion, NO_ARMADO,NULL},
{NO_ARMADO,tiempo_maximo,NO_ARMADO,clean_codigo},
{NO_ARMADO,codigo_c,ARMADO,armar},
{ARMADO, pulsacion, ARMADO,NULL},
{ARMADO,tiempo_maximo,ARMADO,clean_codigo},
{ARMADO,codigo_c,NO_ARMADO,desarmar},
{-1,NULL,-1,NULL},
};


static fsm_trans_t matriz_transicion2[] = {
{APAGADO,comprobar_presencia_1,ENCENDIDO,encender},
{ENCENDIDO,comprobar_presencia_2,APAGADO,apagar},
{-1,NULL,-1,NULL},
};









uint32 user_rf_cal_sector_set(void)
{
    flash_size_map size_map = system_get_flash_size_map();
    uint32 rf_cal_sec = 0;
    switch (size_map) {
        case FLASH_SIZE_4M_MAP_256_256:
            rf_cal_sec = 128 - 5;
            break;

        case FLASH_SIZE_8M_MAP_512_512:
            rf_cal_sec = 256 - 5;
            break;

        case FLASH_SIZE_16M_MAP_512_512:
        case FLASH_SIZE_16M_MAP_1024_1024:
            rf_cal_sec = 512 - 5;
            break;

        case FLASH_SIZE_32M_MAP_512_512:
        case FLASH_SIZE_32M_MAP_1024_1024:
            rf_cal_sec = 1024 - 5;
            break;

        default:
            rf_cal_sec = 0;
            break;
    }

    return rf_cal_sec;
}






void inter(void* ignore)
{
    fsm_t* fsm1 = fsm_new(matriz_transicion1);

    fsm_t* fsm2 = fsm_new(matriz_transicion2);
    desarmar(fsm1);
    portTickType xLastWakeTime;

    while(true) {
      xLastWakeTime = xTaskGetTickCount (); //Almacenamos el tiempo actual

      fsm_fire(fsm1);

if(flag_armar==1){
        fsm_fire(fsm2);
      }
      vTaskDelayUntil(&xLastWakeTime, PERIOD_TICK);
    }
}

void user_init(void)
{
      PIN_FUNC_SELECT(GPIO_PIN_REG_15,FUNC_GPIO15);
    xTaskCreate (&inter, "startup", 2048, NULL, 1, NULL);
    printf("%s\n","Inicio prueba" );
    //Configurar el PIN D8 (GPIO 15)


}
