#define uint8_t unsigned char
#define uint16_t unsigned int
#define uint32_t unsigned int

void EncodeNumber(char *str, uint8_t width, uint16_t inputNum);
uint16_t DecodeNumber(char *str);
bool EncodeSN(char *snstr, uint16_t mweek, uint16_t myear, uint16_t eday, uint16_t emon, uint16_t eyear, uint16_t sn);
bool DecodeSN(char *snstr, uint16_t *mweek, uint16_t *myear, uint16_t *eday, uint16_t *emon, uint16_t *eyear, uint16_t *sn);