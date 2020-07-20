#ifndef KBD_COMMON
#define KBD_COMMON

#define LERP(A,B,L) (A + ((B - A) * L))
#define RLERP(A,B,V) ((V - A) / (B - A))
#define SOCKET_ADDR "/tmp/kbd-lighterd"

extern int usleep (unsigned int __useconds); // this might cause issues.  yolo

typedef struct _file_management
{
    int m_Handle;
} FileManagment;

typedef union _RGB
{
    char raw[6];
    struct _values
    {
        unsigned short R;
        unsigned short G;
        unsigned short B;
    } values;
} RGB;

typedef struct _brightness
{
    unsigned char value;
} Brightness;

typedef struct _led_state
{
    RGB m_Color;
    Brightness m_Brightness;
} LEDState;

typedef struct _keyboard_led_state
{
    RGB m_Color;
    Brightness m_Brightness;
    Brightness m_MaxBrightness;

    FileManagment m_ColorFile;
    FileManagment m_BrightnessFile;
    FileManagment m_MaxBrightnessFile;
} KeyboardLEDState;

#define KBD_LOCK_NUM    0x1
#define KBD_LOCK_CAPS   0x2
#define KBD_LOCK_SCROLL 0x4
typedef struct _keyboard_lock_state
{
    unsigned char m_Values;
} KeyboardLockState;

#define COLOR_MIN 0x3030 // 00
#define COLOR_MAX 0x4646 // FF


void read_led_state(int handle, KeyboardLEDState* kbd);
int read_led_brightness(int handle);
int read_bit_as_bool(int handle);
void write_led_state(int handle, KeyboardLEDState* kbd);
void write_led_brightness(int handle, KeyboardLEDState* kbd);
void print_led_state(KeyboardLEDState* kbd);

#endif // KBD_COMMON