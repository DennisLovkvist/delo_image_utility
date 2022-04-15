
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

    output_image.width = input_image.width / 4;
    output_image.height = input_image.height / 4;
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
    
    //stbi_write_png(output_path, output_image.width, output_image.height, input_image.bytes_per_pixel, output_image.local_buffer, 0);        
    
    
    Image cropped_image;
    cropped_image.width = 500;
    cropped_image.height = 500;
    cropped_image.local_buffer = (unsigned char *) malloc(cropped_image.width * cropped_image.height * input_image.bytes_per_pixel); 

    


    for (int y = 0; y < cropped_image.height; y++)
    {
        int row = y*cropped_image.width*input_image.bytes_per_pixel;
        int row2 = y*output_image.width*input_image.bytes_per_pixel;
        memcpy(&cropped_image.local_buffer[row],&output_image.local_buffer[row2],cropped_image.width*input_image.bytes_per_pixel*sizeof(unsigned char));
    }
    

    

    stbi_write_png(output_path, cropped_image.width, cropped_image.height, input_image.bytes_per_pixel, cropped_image.local_buffer, 0);
    
    
    stbi_image_free(cropped_image.local_buffer);
    stbi_image_free(input_image.local_buffer);
    stbi_image_free(output_image.local_buffer);

    return 0;
}