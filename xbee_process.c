
#define START_BYTE  1
#define LENGTH      2
#define FRAME       3
#define DATA        4
#define CHECKSUM    5

#define MAX_LENGTH      2
#define FRAME_LENGTH    5

int length_index;
int frame_index;
int data_index;
int STATE = START_BYTE;

struct received_package{
    unsigned int  length;
    unsigned char frame[6];
    unsigned char data[255];
    unsigned char checksum;
}package;


void clear_package(struct received_package *package){
    length_index = 0;
    frame_index = 0;
    data_index = 0;
    package->checksum = 0;
}

void process(char package_byte){
    
    switch(STATE)
    {
        case START_BYTE:
            if(package_byte == 0x7E)
            {
                clear_package(&package);
                STATE = LENGTH;
            }
            break;
        case LENGTH:
            if(length_index == 0)
            {
                package.length = package_byte << 8;
                length_index += 1;
            }else if(length_index == 1)
            {
                package.length |= package_byte;
                length_index += 1;
            }
            if(length_index == MAX_LENGTH)
            {
                STATE = FRAME;
            }
            break;  
        case FRAME:
            if(frame_index < FRAME_LENGTH)
            {
                package.frame[frame_index] = package_byte;
                package.checksum += package_byte;
                frame_index += 1;
            }
            if(frame_index >= FRAME_LENGTH)
            {
                STATE = DATA;
            }
            
            break;
        case DATA:
            if((data_index + FRAME_LENGTH ) < package.length)
            {
                package.data[data_index] = package_byte;
                package.checksum += package_byte;
                data_index += 1;
            }
           
            if((data_index + FRAME_LENGTH) == package.length)
            {
                STATE = CHECKSUM;
            }
            break;
        case CHECKSUM:
            
            package.checksum = (0xFF-package.checksum);
            if(package.checksum == package_byte)
            {
                package.data[data_index] = '\0';
                printf("\nReceived Data:  %s\n", package.data);
            }else
            {
                printf("\nDet Blev fel\n");
            }
            STATE = START_BYTE;
            break;
        default:
            STATE = START_BYTE;
            break;      
    }

}