#include <stdio.h>
#include <sys/stat.h>
#define STB_IMAGE_IMPLEMENTATION
#include "libs/stb_image.h"
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "libs/stb_image_resize.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "libs/stb_image_write.h"

#define OPERATION_UNDEFINED 0
#define OPERATION_CROP 1
#define OPERATION_RESIZE 2

#define UNITS_UNDEFINED 0
#define UNITS_PIXELS 1
#define UNITS_PERCENTAGES 2

typedef struct Image Image;
struct Image
{
    int width,height,bytes_per_pixel;
    unsigned char* local_buffer;
};
void crop_by_pixels(Image *image_source,Image *image_cropped,int crop_x,int crop_y,int crop_w,int crop_h)
{
    int target_width = crop_w;
    int target_height = crop_h;
    int target_offset_x = crop_x;
    int target_offset_y = crop_y;

    //Prevents cropping outside the image
    target_offset_x = (target_offset_x < 0 ? 0:target_offset_x);
    target_offset_x = (target_offset_x >= image_source->width ? image_source->width-1:target_offset_x);

    target_offset_y = (target_offset_y < 0 ? 0:target_offset_y);
    target_offset_y = (target_offset_y >= image_source->height ? image_source->height-1:target_offset_y);
    
    target_width = (target_offset_x + target_width > image_source->width ? image_source->width - target_offset_x:target_width);
    target_height = (target_offset_y + target_height > image_source->height ? image_source->height - target_offset_y:target_height);

    image_cropped->width = target_width;
    image_cropped->height = target_height;    
    image_cropped->bytes_per_pixel = image_source->bytes_per_pixel;
    
    image_cropped->local_buffer = (unsigned char *) malloc(image_cropped->width * image_cropped->height * image_source->bytes_per_pixel);  

    for (int y = 0; y < image_cropped->height; y++)
    {
        int cropped_image_row = y*image_cropped->width*image_source->bytes_per_pixel;
        int resized_image_row = (y + target_offset_y)* (image_source->width *image_source->bytes_per_pixel) + (target_offset_x*image_source->bytes_per_pixel);
        memcpy(&image_cropped->local_buffer[cropped_image_row],&image_source->local_buffer[resized_image_row],image_cropped->width*image_source->bytes_per_pixel*sizeof(unsigned char));
    }
}
void crop_by_percentage(Image *image_source,Image *image_cropped,float crop_x,float crop_y,float crop_w,float crop_h)
{
    int target_width = image_source->width*crop_w;
    int target_height = image_source->height*crop_h;
    int target_offset_x = image_source->width*crop_x;
    int target_offset_y = image_source->height*crop_y;

    //Prevents cropping outside the image
    target_offset_x = (target_offset_x < 0 ? 0:target_offset_x);
    target_offset_x = (target_offset_x >= image_source->width ? image_source->width-1:target_offset_x);

    target_offset_y = (target_offset_y < 0 ? 0:target_offset_y);
    target_offset_y = (target_offset_y >= image_source->height ? image_source->height-1:target_offset_y);
    
    target_width = (target_offset_x + target_width > image_source->width ? image_source->width - target_offset_x:target_width);
    target_height = (target_offset_y + target_height > image_source->height ? image_source->height - target_offset_y:target_height);
    
    image_cropped->width = target_width;
    image_cropped->height = target_height;
    image_cropped->bytes_per_pixel = image_source->bytes_per_pixel;
    image_cropped->local_buffer = (unsigned char *) malloc(image_cropped->width * image_cropped->height * image_source->bytes_per_pixel);  

    for (int y = 0; y < image_cropped->height; y++)
    {
        int cropped_image_row = y*image_cropped->width*image_source->bytes_per_pixel;
        int resized_image_row = (y + target_offset_y)* (image_source->width *image_source->bytes_per_pixel) + (target_offset_x*image_source->bytes_per_pixel);
        memcpy(&image_cropped->local_buffer[cropped_image_row],&image_source->local_buffer[resized_image_row],image_cropped->width*image_source->bytes_per_pixel*sizeof(unsigned char));
    }
}
void resize(Image *image_source,Image *image_resized,float scale_factor)
{
    image_resized->width = image_source->width * scale_factor;
    image_resized->height = image_source->height * scale_factor;
    image_resized->bytes_per_pixel = image_source->bytes_per_pixel;
    image_resized->local_buffer = (unsigned char *) malloc(image_resized->width * image_resized->height * image_source->bytes_per_pixel);    
    
    stbir_resize(image_source->local_buffer, 
                 image_source->width, 
                 image_source->height, 
                 0, 
                 image_resized->local_buffer, 
                 image_resized->width, 
                 image_resized->height, 
                 0, 
                 STBIR_TYPE_UINT8, 
                 image_source->bytes_per_pixel, 
                 STBIR_ALPHA_CHANNEL_NONE, 
                 0,
                 STBIR_EDGE_CLAMP, STBIR_EDGE_CLAMP,
                 STBIR_FILTER_BOX, STBIR_FILTER_BOX,
                 STBIR_COLORSPACE_SRGB, NULL);                 
    
}
int main(int argc, char *argv[])
{     
    int operation = OPERATION_UNDEFINED;
    int units = UNITS_UNDEFINED;
    char *input_path;
                
    if(argc < 4)
    {
        printf("[error]::not enough arguments \n");
        return 0;
    }
    else
    {             
        if(strcmp(argv[1],"-crop") == 0)
        {   
            operation = OPERATION_CROP;               
        }
        else if(strcmp(argv[1],"-resize") == 0)
        {
            operation = OPERATION_RESIZE;    
        }
        else
        {
            printf("[error]::undefined operation \n");
            return 0;
        }

        input_path = argv[2];        
        struct stat st;
        int result = stat(input_path, &st);       

        if(result == -1)
        {
            printf("[error]::cannot find file \n");
            return 0;
        }

        Image image_source;    
        
        if(operation == OPERATION_CROP)
        {
            if(argc < 8)
            {
                printf("[error]::not enough arguments \n");
                return 0;
            }
            else
            {
                if(strcmp(argv[3],"-px") == 0)
                {
                    units = UNITS_PIXELS;            
                }
                else if(strcmp(argv[3],"-pct") == 0)
                {
                    units = UNITS_PERCENTAGES;
                } 
                else
                {
                    printf("[error]::undefined units \n");
                    return 0;
                }

                printf("%s", "[info]::loading source image "); 
                printf("%s\n", input_path);   
                image_source.local_buffer = stbi_load(input_path, &image_source.width, &image_source.height, &image_source.bytes_per_pixel, 0);

                //Build output path string
                char *output_path;  
                int length = strlen(input_path);
                output_path = malloc(sizeof(char) * (length + 9));
                memcpy(output_path,"cropped_", sizeof(char) * 8);        
                memcpy(&output_path[8],input_path, length);  
                output_path[length + 8] = '\0';

                Image image_cropped; 
                if(units == UNITS_PERCENTAGES)
                {
                    char *ptr;
                    float crop_x = strtof(argv[4], &ptr);  
                    float crop_y = strtof(argv[5], &ptr);  
                    float crop_w = strtof(argv[6], &ptr);  
                    float crop_h = strtof(argv[7], &ptr); 

                    //Clamp values
                    crop_x = (crop_x < 0.0f ? 0.0:crop_x);
                    crop_x = (crop_x >= 1.0f ? 0.9:crop_x);
                    crop_y = (crop_y < 0.0f ? 0.0:crop_y);
                    crop_y = (crop_y >= 1.0f ? 0.9:crop_y);
                    crop_w = (crop_w <= 0.0f ? 0.1:crop_w);
                    crop_w = (crop_w > 1.0f ? 1.0:crop_w);
                    crop_h = (crop_h <= 0.0f ? 0.1:crop_h);
                    crop_h = (crop_h > 1.0f ? 1.0:crop_h);

                    printf("%s\n", "[info]::cropping image...");  
                    crop_by_percentage(&image_source,&image_cropped, crop_x,crop_y,crop_w,crop_h);  
                }
                else if(units == UNITS_PIXELS)
                {
                    char *ptr;
                    int crop_x = strtol(argv[4], &ptr,10);  
                    int crop_y = strtol(argv[5], &ptr,10);  
                    int crop_w = strtol(argv[6], &ptr,10);  
                    int crop_h = strtol(argv[7], &ptr,10); 

                    //Clamp values
                    crop_x = (crop_x < 0 ? 0:crop_x);
                    crop_x = (crop_x >= image_source.width ? image_source.width-16:crop_x);
                    crop_y = (crop_y < 0 ? 0:crop_y);
                    crop_y = (crop_y >= image_source.height ? image_source.height-16:crop_y);
                    crop_w = (crop_w <= 0 ? 16:crop_w);
                    crop_w = (crop_w > image_source.width ? image_source.width:crop_w);
                    crop_h = (crop_h <= 0 ? 16:crop_h);
                    crop_h = (crop_h > image_source.height ? image_source.height:crop_h);

                    printf("%s\n", "[info]::cropping image...");        
                    crop_by_pixels(&image_source,&image_cropped, crop_x,crop_y,crop_w,crop_h);                                       
                }

                printf("%s", "[info]::saving cropped image as "); 
                printf("%s\n", output_path); 

                stbi_write_png(output_path, image_cropped.width, image_cropped.height, image_cropped.bytes_per_pixel, image_cropped.local_buffer, 0); 
                
                stbi_image_free(image_cropped.local_buffer); 
                stbi_image_free(image_source.local_buffer);    
                
                free(output_path); 
            }
        }  
        else
        {
            if(argc < 4)
            {
                printf("[error]::not enough arguments for resizing \n");
                return 0;
            }
            else
            {
                char *ptr;
                float scale_factor = strtof(argv[3], &ptr); 

                char *output_path;  
                int length = strlen(input_path);
                output_path = malloc(sizeof(char) * (length + 9));
                memcpy(output_path,"resized_", sizeof(char) * 8);        
                memcpy(&output_path[8],input_path, length);  
                output_path[length + 8] = '\0';

                printf("%s", "[info]::loading source image "); 
                printf("%s\n", input_path);   
                image_source.local_buffer = stbi_load(input_path, &image_source.width, &image_source.height, &image_source.bytes_per_pixel, 0);
                
                Image image_resized; 
                printf("%s\n", "[info]::resizing image...");  
                resize(&image_source,&image_resized,scale_factor); 

                printf("%s", "[info]::saving resizedped image as "); 
                printf("%s\n", output_path); 
                stbi_write_png(output_path, image_resized.width, image_resized.height, image_resized.bytes_per_pixel, image_resized.local_buffer, 0);   
                
                stbi_image_free(image_resized.local_buffer); 
                stbi_image_free(image_source.local_buffer); 
                
                free(output_path); 
            }
        }          
    }
    return 0;
}