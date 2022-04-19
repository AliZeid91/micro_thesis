
#include <inttypes.h>


void init_adc(int i2c_fd, uint8_t *writeBuf, uint8_t *readBuf){
 
      /* Initialize the buffer used to read data from the ADS1115 to 0 */
        readBuf[0]= 0;		
        readBuf[1]= 0;

        writeBuf[0] = 0x03;
        writeBuf[1] = 0x80;
        writeBuf[2] = 0x00;
      /* Write writeBuf to the ADS1015*/

       // Set ALERT/RDY to RDY mode.
        if (write((int)i2c_fd,writeBuf, 3) != 3)
        {
            perror("Write to register 1");
            exit(-1);
        }
          
     /* Wait for the conversion to complete, this requires bit 15 to change from 0->1 */
        do {
            /* Read the config register into readBuf*/
            if (read((int)i2c_fd,readBuf, 2) != 2)
              {
              perror("Read conversion");
              exit(-1);
              }
          } while (!(readBuf[0] & 0x80));
         
       
     /* write to the ADS1015 to set the config register and start a conversion */
        writeBuf[0] = 1;			// set the pointer register so that the following two bytes write to the config register
        writeBuf[1] = 0b11000000;   // set the 8 MSBs of the config register (bits 15-8) to 11000001 (set config register (reg. 1) and start conversion AIN0 and GND, 6.144 V)
        writeBuf[2] = 0b10000101;  		// set the 8 LSBs of the config register (bits 7-0) to 10000101 (128 samples/s) 0x85

     if (write((int)i2c_fd,writeBuf, 3) != 3)
        {
            perror("Write to register 1");
            exit(-1);
        } 
    usleep(1000);
       
}

void ads1015_rx(int i2c_fd, uint8_t *writeBuf, uint8_t *readBuf){
        int16_t adc_val;            // Stores the 16 bit value of ADC conversion
        
    /* Set pointer register to 0 to read from the conversion register */
        writeBuf[0] = 0;		
        	
        if (write(i2c_fd, writeBuf, 1) != 1)
        {
            perror("Write register select");
            exit(-1);
        }

        /* Read the contents of the conversion register into readBuf */
        if(read(i2c_fd,readBuf,2) !=2)
        {
            perror("Read conversion");
            exit(-1);
        }

        #if 1
        /* Combine the two bytes of readBuf into a single 16 bit result */
        adc_val = readBuf[0] << 8 | readBuf[1];	

        /* Print the result to terminal, first convert from binary value to mV */
        printf("ADS1015 Voltage Reading %f V \n", (float)adc_val*6.144/32767.0);	
        //printf("%3d%3d\n",readBuf[0],readBuf[1]);
        //packet.data[0] = readBuf[0];
        //packet.data[1] = readBuf[1];
        #endif
}

#if 0
void ads1015_rx(int i2c_fd, uint8_t *writeBuf, uint8_t *readBuf){
        int16_t adc_val;            // Stores the 16 bit value of ADC conversion
        
    /* Set pointer register to 0 to read from the conversion register */
        writeBuf[0] = 0;		
        	
        if (write(i2c_fd, writeBuf, 1) != 1)
        {
            perror("Write register select");
            exit(-1);
        }

        /* Read the contents of the conversion register into readBuf */
        if(read(i2c_fd,readBuf,2) !=2)
        {
            perror("Read conversion");
            exit(-1);
        }

        /* Combine the two bytes of readBuf into a single 16 bit result */
        adc_val = readBuf[0] << 8 | readBuf[1];	

        /* Print the result to terminal, first convert from binary value to mV */
        printf("ADS1015 Voltage Reading %f V \n", (float)adc_val*6.144/32767.0);	
        //printf("%3d%3d\n",readBuf[0],readBuf[1]);
        packet.data[0] = readBuf[0];
        packet.data[1] = readBuf[1];
}
#endif