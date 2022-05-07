
#define START_BYTE  1
#define LENGTH      2
#define FRAME_TYPE  3
#define FRAME_ID    4
#define DELIVERY_STATUS 5
#define CHECKSUM    6

#define MAX_LENGTH      2
#define FRAME_LENGTH    5

int length_index;
int frame_index;
int data_index;
int STATE = START_BYTE;

struct received_package{
    unsigned int  length;
    unsigned char frame[6];
    unsigned int delivery_status;
    unsigned char data[255];
    unsigned char checksum;
}received_status;


void clear_package_1(struct received_package *received_status){
    length_index = 0;
    frame_index = 0;
    data_index = 0;
    received_status->checksum = 0;
}

void process(char package_byte, bool *tx_status_received){
    
    switch(STATE)
    {
        case START_BYTE:
            if(package_byte == 0x7E)
            {
                clear_package_1(&received_status);
                STATE = LENGTH;
            }
            break;
        case LENGTH:
            if(length_index == 0)
            {
                received_status.length = package_byte << 8;
                length_index += 1;
            }else if(length_index == 1)
            {
                received_status.length |= package_byte;
                length_index += 1;
            }
            if(length_index == MAX_LENGTH)
            {
                STATE = FRAME_ID;
            }
            break;  
        case FRAME_ID:
            received_status.frame[0] = package_byte;
            received_status.checksum += package_byte;    
            STATE = FRAME_TYPE;
            break;
        case FRAME_TYPE:
            received_status.frame[1] = package_byte;
            received_status.checksum += package_byte; 
            STATE = DELIVERY_STATUS;
            break;
        case DELIVERY_STATUS:
            received_status.delivery_status = package_byte;
            received_status.checksum += package_byte;
            STATE = CHECKSUM;
            break;
        case CHECKSUM:
            received_status.checksum = (0xFF-received_status.checksum);
            if(received_status.checksum == package_byte)
            {
                //printf("\nDELIVERY STATUS:  %d\n", received_status.delivery_status);
            }else
            {
                printf("\nDet Blev fel\n");
            }
            *tx_status_received = true;
            STATE = START_BYTE;
            break;
        default:
            STATE = START_BYTE;
            break;      
    }

}