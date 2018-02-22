#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"
#include "fsm.h"

#define LED 2 // Led va asociado al 2
#define PULL_UP_0 0
#define S_INFRA 15
#define freq portTICK_RATE_MS
#define PERIOD_TICK 20/freq
volatile int tiempo=0;
#define ANTIREBOTE 300/freq //300 ms para antirrebote
int flag_armar=0;




/*
Enumeración de los estados de la máquina de Mealy
*/
 enum fsm_state {
APAGADO,
ENCENDIDO,
ARMADO,
NO_ARMADO,
};



  /*
Definimos la matriz de transicion que tiene como formato
{ESTADO ACTUAL, funcion de comprobacion, ESTADO_SIGUIENTE, funcion de actualizacion}
  */

int impulso_GPIO_up(fsm_t* this){

  if( !GPIO_INPUT_GET(0) ){
    return 1;
   }
   return 0;

}


int impulso_GPIO_down(fsm_t* this){

  if( GPIO_INPUT_GET(0) ){
    return 1;
   }
   return 0;

}


void armar(fsm_t* this){
flag_armar=1; //activa el flag de alarma

}

void desarmar(fsm_t* this){
flag_armar=0; //desactiva el flag de alarma
GPIO_OUTPUT_SET(2,1);


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
{NO_ARMADO,impulso_GPIO_up,ARMADO,armar},
{ARMADO,impulso_GPIO_down,NO_ARMADO,desarmar},
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
    GPIO_OUTPUT_SET(2,1); //Inicialmente arranca el led apagado
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
    //Configurar el PIN D8 (GPIO 15)


}
