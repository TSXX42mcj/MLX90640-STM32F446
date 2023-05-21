#include <Wire.h>

#include "MLX90640_API.h"
#include "MLX90640_I2C_Driver.h"

#define LED_PIN PA4
#define Button_PIN PB7
#define TA_SHIFT 8 //Default shift for MLX90640 in open air
#define emissivity 0.95

#define MLX90640_address 0x33 //Default 7-bit unshifted address of the MLX90640
float mlx90640To[768], mlx90640Re[24][32];
paramsMLX90640 mlx90640;

float max_range, min_range, max_avg, min_avg, mid_avg;
int count = 0;
float mlx90640Max[8], mlx90640Min[8], mlx90640Mid[8];

void setup() {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  pinMode(Button_PIN, INPUT);
  //Wire.setSDA(PB9);
  //Wire.setSCL(PB8);
  Wire.begin();
  Wire.setClock(400000);//STM32F446RCT6`s max speed is 400khz
  //Wire.setClock(1000000);

  Serial.begin(921600); //Fast serial as possible
  while (!Serial) {  }

  setup_mlx();
}

void range(float max_temp, float min_temp, float mid_temp) {
  float offset;
  mid_avg = mid_avg - (mlx90640Mid[count] - mid_temp) * 0.125;
  max_avg = max_avg - (mlx90640Max[count] - max_temp) * 0.125;
  min_avg = min_avg - (mlx90640Min[count] - min_temp) * 0.125;

  mlx90640Mid[count] = mid_temp;
  mlx90640Max[count] = max_temp;
  mlx90640Min[count] = min_temp;
  offset = abs(mid_avg - mid_temp);
  if (offset > 2 || max_avg - max_temp > 2) {
    max_range = max_avg + offset;
    min_range = min_avg + offset;
    if (max_range - mid_avg < 8) {
      max_range = mid_avg + 8;
    }
    if (mid_avg - min_range < 2) {
      min_range = mid_avg - 2;
    }
  }
  offset = max_range - max_temp;
  if (offset < 0) {
    max_range = max_temp;
  } else if (offset > 2) {
    max_range = max_temp + 2;
  }

  count++;
  if (count == 8)
    count = 0;
}

void setup_mlx() {
  int status;
  uint16_t eeMLX90640[832];

  while (MLX90640_DumpEE(MLX90640_address, eeMLX90640) != 0 || MLX90640_ExtractParameters(eeMLX90640, &mlx90640) != 0) {
    digitalWrite(LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  }

  //MLX90640_SetRefreshRate(MLX90640_address, 0x03); //Set rate to 2Hz
  MLX90640_SetRefreshRate(MLX90640_address, 0x04); //Set rate to 4Hz
  //MLX90640_SetRefreshRate(MLX90640_address, 0x05); //Set rate to 8Hz

  float total_max = 0, total_min = 0;
  digitalWrite(LED_PIN, HIGH);
  float max_temp, min_temp, mid_temp, max_num, min_num, de_temp;
  for (int i = 0; i < 8; i++) {
    read_mlx();
    max_temp = mlx90640To[768];
    min_temp = mlx90640To[768];
    mlx90640Max[i] = max_temp;
    mlx90640Min[i] = min_temp;
    mlx90640Mid[i] = 0;
    total_max = total_max + max_temp;
    total_min = total_min + min_temp;
  }
  max_avg = total_max * 0.125;
  min_avg = total_min * 0.125;
  mid_avg = 0;
  digitalWrite(LED_PIN, LOW);
}

void read_mlx() {
  for (int i = 0 ; i < 2 ; i++) {
    uint16_t mlx90640Frame[834];
    int status = MLX90640_GetFrameData(MLX90640_address, mlx90640Frame);
    float vdd = MLX90640_GetVdd(mlx90640Frame, &mlx90640);
    float Ta = MLX90640_GetTa(mlx90640Frame, &mlx90640);
    //float tr = Ta - TA_SHIFT; //Reflected temperature based on the sensor ambient temperature
    MLX90640_CalculateTo(mlx90640Frame, &mlx90640, emissivity, Ta - TA_SHIFT, mlx90640To);
  }
}

void loop() {
  //if (digitalRead(Button_PIN) == HIGH)
  //  setup_mlx();
  float max_temp = -40, min_temp = 500, max_col = 0, max_rol = 0, min_num = 0, total = 0;
  digitalWrite(LED_PIN, HIGH);
  read_mlx();
  digitalWrite(LED_PIN, LOW);
  String result = "";
  int n = 0, k = 0;
  for (int i = 23; i >= 0; i--) {
    for (int j = 0; j < 32; j++) {
      mlx90640Re[i][j] = mlx90640To[n];
      n++;
      if (mlx90640Re[i][j] > max_temp) {
        max_temp = mlx90640Re[i][j];
        max_col = i;
        max_rol = j;
      }
      if (mlx90640Re[i][j] < min_temp) {
        min_temp = mlx90640Re[i][j];
      }
      Serial.print(String(mlx90640Re[k][j], 2) + ",");
    }
    k++;
  }

  total = mlx90640To[167] + mlx90640To[173] + mlx90640To[179] + mlx90640To[185] + mlx90640To[327] + mlx90640To[345] +
          mlx90640To[487] + mlx90640To[493] + mlx90640To[499] + mlx90640To[505];
  range(max_temp, min_temp, total * 0.125);
  Serial.println(String(max_range, 2) + "," + String(min_range, 2) + "," + String(max_temp, 2) + "," + String(max_col) + "," + String(max_rol));
}

