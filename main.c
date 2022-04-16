#include <stdio.h>
#include <sys/stat.h>
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

int main(int argc, char *argv[])
{     
    float scale_factor = 1.0f;
    char *input_path;
    char *output_path;  
    float crop_x,crop_y = 0.0f;
    float crop_w,crop_h = 1.0f;
    int crop = 0;

    if(argc > 1)
    {
        input_path = argv[1];
        
        struct stat st;
        int result = stat(input_path, &st);       

        if(result == -1)
        {
            printf("[error]::cannot find file \n");
            return 0;
        }
        
        //Build output path string
        int length = strlen(input_path);
        output_path = malloc(sizeof(char) * (length + 9));
        memcpy(output_path,"resized_", sizeof(char) * 8);        
        memcpy(&output_path[8],input_path, length);  
        output_path[length + 8] = '\0';
    }

    if(argc > 2)
    {     
        //Convert 2 command line arguments to float
        char *ptr;
        scale_factor = strtof(argv[2], &ptr);  

        if(scale_factor == 0)
        {
            printf("[error]::scale factor cannot be 0 \n");
            return 0;
        }   

        if(argc == 7)
        {
            //Convert crop arguments to float            
            crop_x = strtof(argv[3], &ptr);  
            crop_y = strtof(argv[4], &ptr);  
            crop_w = strtof(argv[5], &ptr);  
            crop_h = strtof(argv[6], &ptr); 

            //Clamp values between 0 and 1
            //Values are "percentages"
            crop_x = (crop_x < 0.0f ? 0.0:crop_x);
            crop_x = (crop_x > 1.0f ? 1.0:crop_x);
            crop_y = (crop_y < 0.0f ? 0.0:crop_y);
            crop_y = (crop_y > 1.0f ? 1.0:crop_y);
            crop_w = (crop_w < 0.0f ? 0.0:crop_w);
            crop_w = (crop_w > 1.0f ? 1.0:crop_w);
            crop_h = (crop_h < 0.0f ? 0.0:crop_h);
            crop_h = (crop_h > 1.0f ? 1.0:crop_h);

            //Tells program to run cropping code
            crop = 1;
        }
    }  

    Image input_image;
    Image resized_image;
    Image cropped_image;  
    
    input_image.local_buffer = stbi_load(input_path, &input_image.width, &input_image.height, &input_image.bytes_per_pixel, 0);

    resized_image.width = input_image.width * scale_factor;
    resized_image.height = input_image.height * scale_factor;
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

    if(!crop)
    {   
        stbi_write_png(output_path, resized_image.width, resized_image.height, resized_image.bytes_per_pixel, resized_image.local_buffer, 0);   
        stbi_image_free(resized_image.local_buffer);  
    }
    else
    {           
        int target_width = resized_image.width*crop_w;
        int target_height = resized_image.height*crop_h;
        int target_offset_x = resized_image.width*crop_x;
        int target_offset_y = resized_image.height*crop_y;

        //Prevents cropping outside the image
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
    }

    free(output_path);

    return 0;
}