#ifndef COLOR_CONVERSION
#define COLOR_CONVERSION

typedef struct _rgb_color
{
    float R; // range: [0, 1]
    float G; // range: [0, 1]
    float B; // range: [0, 1]
} RGBColor;

typedef struct _hsv_color
{
    float H; // range: [0, 360]
    float S; // range: [0, 1]
    float V; // range: [0, 1]
} HSVColor;

/*! \brief Convert RGB to HSV color space
  
  Converts a given set of RGB values `r', `g', `b' into HSV
  coordinates. The input RGB values are in the range [0, 1], and the
  output HSV values are in the ranges h = [0, 360], and s, v = [0,
  1], respectively.
  
*/
void RGBtoHSV(const RGBColor* in_rgb, HSVColor* out_hsv);


/*! \brief Convert HSV to RGB color space
  
  Converts a given set of HSV values `h', `s', `v' into RGB
  coordinates. The output RGB values are in the range [0, 1], and
  the input HSV values are in the ranges h = [0, 360], and s, v =
  [0, 1], respectively.

*/
void HSVtoRGB(const HSVColor* in_hsv, RGBColor* out_rgb);

void color_conversion_test();
#endif // COLOR_CONVERSION