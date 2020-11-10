#include "dht11.h"


int get_DHT_data(uint8_t *dataBytes){
	
  int count = 0;
  int byteNum = 0;
  int bitNum = 7;
  int bitCount = 0;

  for (int i = 0; i <6; i++)
  		dataBytes[i] = 0; 

  	gpio_set_direction(DHT_PIN, GPIO_MODE_INPUT_OUTPUT);
	gpio_set_level(DHT_PIN, 0); 
	vTaskDelay(20 / portTICK_PERIOD_MS);
	gpio_set_level(DHT_PIN, 1);

	count = 0;
	while(gpio_get_level(DHT_PIN) != 0){
		gpio_set_direction(DHT_PIN, GPIO_MODE_INPUT);
		if(count > 45){
			return (ESP_ERR_TIMEOUT);
		}
		if(gpio_get_level(DHT_PIN) == 0){
			break;
		}
		count++;
		ets_delay_us(1);
	}
	count = 0;
	while(gpio_get_level(DHT_PIN) == 0)
	{
		if(count > 80)
		{
			return (ESP_ERR_TIMEOUT);
		}
		count++;
		ets_delay_us(1);
	}
	count = 0;
	while(gpio_get_level(DHT_PIN) == 1)
	{
		if(count > 80)
		{	
			 fflush (stdout);
			return (ESP_ERR_TIMEOUT);
		}
		if(gpio_get_level(DHT_PIN) == 0)
			break;
		count++;
		ets_delay_us(1);
	}

	bitCount = 0;
	while(bitCount < 40){
	
		count = 0;
		while(gpio_get_level(DHT_PIN) == 0)
		{
			if (count > 50)
			{
				return (ESP_ERR_TIMEOUT);
			}
			if(gpio_get_level(DHT_PIN) == 1)
				break;
			count++;
			ets_delay_us(1);
		}
		count = 0;
		while(gpio_get_level(DHT_PIN) == 1)
		{
			if (count > 70)
			{
				return (ESP_ERR_TIMEOUT);
			}
			if(gpio_get_level(DHT_PIN) == 0)
				break;
			count++;
			ets_delay_us(1);
		}
  	  
		if (count > 28)
			dataBytes[byteNum] |= (1 << bitNum);
			
		if(bitNum == 0)
		{
			byteNum += 1;
			bitNum = 7;
		}
		else
			bitNum -= 1;
		
		bitCount++;
	}
		int checksum = dataBytes[0] + dataBytes[2] + dataBytes[3];

	if(checksum != dataBytes[4])
		return (ESP_ERR_INVALID_CRC);

	return (ESP_OK);
}