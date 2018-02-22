#include "esp_common.h"
#include "freertos/task.h"
#include "gpio.h"
#include "fsm.h"

#define LED 2 // Led va asociado al 2
#define PULL_UP_0 0
#define PULL_DOWN_15 15
#define freq portTICK_RATE_MS
#define PERIOD_TICK 20/freq


volatile int tiempo=0;
volatile int t_apagar=0;

#define ANTIREBOTE 300/freq //200 ms para antirrebote
#define MINUTO 1000/freq

/*
Enumeración de los estados de la máquina de Mealy
*/
 enum fsm_state {
APAGADO,
ENCENDIDO,
};


  /*
Definimos la matriz de transicion que tiene como formato
{ESTADO ACTUAL, funcion de comprobacion, ESTADO_SIGUIENTE, funcion de actualizacion}
  */





int pulsado (fsm_t* this){

int condicion= GPIO_INPUT_GET(15) || (!GPIO_INPUT_GET(0));
int tiempo_actual=xTaskGetTickCount ();

  if( condicion ){

      if(tiempo_actual > tiempo){

        tiempo=tiempo_actual + ANTIREBOTE;

        return 1;
      }
      return 0;
  }
  return 0;
}


int no_pulsado (fsm_t* this){

int tiempo_actual=xTaskGetTickCount ();

if(tiempo_actual > t_apagar){

t_apagar=0;
return 1;

}

return 0;



}



void encender(fsm_t* this){
  GPIO_OUTPUT_SET(2,0);
  int tiempo_actual=xTaskGetTickCount ();
  t_apagar=tiempo_actual + MINUTO;

}

void apagar(fsm_t* this){
  GPIO_OUTPUT_SET(2,1);

}


static fsm_trans_t matriz_transicion[] = {
{APAGADO,pulsado,ENCENDIDO,encender},
{ENCENDIDO,pulsado,APAGADO,apagar},
{ENCENDIDO,no_pulsado,APAGADO,apagar},
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
    fsm_t* fsm = fsm_new(matriz_transicion);
    GPIO_OUTPUT_SET(2,1); //Inicialmente arranca el led apagado
    portTickType xLastWakeTime;

    while(true) {
      xLastWakeTime = xTaskGetTickCount (); //Almacenamos el tiempo actual
      fsm_fire(fsm);
      vTaskDelayUntil(&xLastWakeTime, PERIOD_TICK);
    }
}

void user_init(void)
{
      PIN_FUNC_SELECT(GPIO_PIN_REG_15,FUNC_GPIO15);
    xTaskCreate (&inter, "startup", 2048, NULL, 1, NULL);
    //Configurar el PIN D8 (GPIO 15)


}
