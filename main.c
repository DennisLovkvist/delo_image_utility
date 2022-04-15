
#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "libs/stb_image_resize.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "libs/stb_image_write.h"

typedef struct Image Image;
struct Image
{
    int width,height,bytes_per_pixel;
    unsigned char* local_buffer;
};

int main(void)
{          
    Image input_image;
    Image output_image;

    char input_path[] = "test.png";
    char output_path[] = "output.png";    
    
    input_image.local_buffer = stbi_load(input_path, &input_image.width, &input_image.height, &input_image.bytes_per_pixel, 0);

    output_image.width = input_image.width / 2;
    output_image.height = input_image.height / 2;
    output_image.local_buffer = (unsigned char *) malloc(output_image.width * output_image.height * input_image.bytes_per_pixel);    
    
    stbir_resize(input_image.local_buffer, 
                 input_image.width, 
                 input_image.height, 
                 0, 
                 output_image.local_buffer, 
                 output_image.width, 
                 output_image.height, 
                 0, 
                 STBIR_TYPE_UINT8, 
                 input_image.bytes_per_pixel, 
                 STBIR_ALPHA_CHANNEL_NONE, 
                 0,
                 STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
                 STBIR_FILTER_BOX, STBIR_FILTER_BOX,
                 STBIR_COLORSPACE_SRGB, NULL);
    
    stbi_write_png(output_path, output_image.width, output_image.height, input_image.bytes_per_pixel, output_image.local_buffer, 0);

    stbi_image_free(input_image.local_buffer);
    stbi_image_free(output_image.local_buffer);
    
    return 0;
}