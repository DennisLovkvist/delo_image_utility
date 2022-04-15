
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
    int target_width = 500;
    int target_height = 500;

    int target_offset_x = 800;
    int target_offset_y = 0;


    

    Image input_image;
    Image resized_image;
    Image cropped_image;

    char input_path[] = "test.png";
    char output_path[] = "output.png";    
    
    input_image.local_buffer = stbi_load(input_path, &input_image.width, &input_image.height, &input_image.bytes_per_pixel, 0);

    resized_image.width = input_image.width / 4;
    resized_image.height = input_image.height / 4;
    resized_image.bytes_per_pixel = input_image.bytes_per_pixel;
    resized_image.local_buffer = (unsigned char *) malloc(resized_image.width * resized_image.height * input_image.bytes_per_pixel);    
    
    stbir_resize(input_image.local_buffer, 
                 input_image.width, 
                 input_image.height, 
                 0, 
                 resized_image.local_buffer, 
                 resized_image.width, 
                 resized_image.height, 
                 0, 
                 STBIR_TYPE_UINT8, 
                 input_image.bytes_per_pixel, 
                 STBIR_ALPHA_CHANNEL_NONE, 
                 0,
                 STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
                 STBIR_FILTER_BOX, STBIR_FILTER_BOX,
                 STBIR_COLORSPACE_SRGB, NULL);  
                 
    stbi_image_free(input_image.local_buffer);    


    target_offset_x = (target_offset_x < 0 ? 0:target_offset_x);
    target_offset_x = (target_offset_x >= resized_image.width ? resized_image.width-1:target_offset_x);

    target_offset_y = (target_offset_y < 0 ? 0:target_offset_y);
    target_offset_y = (target_offset_y >= resized_image.height ? resized_image.height-1:target_offset_y);
    
    target_width = (target_offset_x + target_width > resized_image.width ? resized_image.width - target_offset_x:target_width);
    target_height = (target_offset_y + target_height > resized_image.height ? resized_image.height - target_offset_y:target_height);
    
    cropped_image.width = target_width;
    cropped_image.height = target_height;
    cropped_image.local_buffer = (unsigned char *) malloc(cropped_image.width * cropped_image.height * resized_image.bytes_per_pixel);  

    for (int y = 0; y < cropped_image.height; y++)
    {
        int cropped_image_row = y*cropped_image.width*resized_image.bytes_per_pixel;
        int resized_image_row = (y + target_offset_y)* (resized_image.width *resized_image.bytes_per_pixel) + (target_offset_x*resized_image.bytes_per_pixel);
        memcpy(&cropped_image.local_buffer[cropped_image_row],&resized_image.local_buffer[resized_image_row],cropped_image.width*resized_image.bytes_per_pixel*sizeof(unsigned char));
    }
    
    stbi_image_free(resized_image.local_buffer);    

    stbi_write_png(output_path, cropped_image.width, cropped_image.height, input_image.bytes_per_pixel, cropped_image.local_buffer, 0);   
    
    stbi_image_free(cropped_image.local_buffer);

    return 0;
}