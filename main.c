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

#define ARG_OP 0
#define ARG_I 1
#define ARG_U 2
#define ARG_X 3
#define ARG_Y 4
#define ARG_W 5
#define ARG_H 6
#define ARG_S 7
#define ARG_O 8

#define FLAG_COUNT 9

typedef struct Image Image;
struct Image
{
    int width,height,bytes_per_pixel;
    unsigned char* local_buffer;
};

const char* FLAGS[FLAG_COUNT] = { "-op", "-i", "-u", "-x", "-y","-w","-h", "-s","-o"};
char* args_values[FLAG_COUNT]; 
int args_defined[FLAG_COUNT] = {0,0,0,0,0,0,0,0,0}; 

void parse_arguments(int argc,char **argv[])
{
    for (size_t i = 1; i < argc; i+=2)
    {
        char *arg = (*argv)[i];

        for (size_t j = 0; j < FLAG_COUNT; j++)
        {
            if(strcmp(arg,FLAGS[j]) == 0)
            {
                if(i+1 < argc)
                {
                    args_values[j] = (*argv)[i+1];
                    args_defined[j] = 1;
                }
            }

        }
    }
    
}
void print_help()
{
    printf("%s\n", "+-------------------------------------------------------------------------------------------+");
    printf("%s\n", "| [Resizing]                                                                                |");
    printf("%s\n", "|           Example: delo_img -op resize -i test_image.png -s 0.5                           |");
    printf("%s\n", "|           Result: Scales the image by half and saves a copy.                              |");     
    printf("%s\n", "|                                                                                           |");
    printf("%s\n", "| [Cropping]                                                                                |");
    printf("%s\n", "|           Example: delo_img -op crop -i test_image.png -u px -x 100 -y 100 -w 300 -h 300  |");
    printf("%s\n", "|           Result: Crops out an image using pixels as units and saves a copy.              |"); 
    printf("%s\n", "|                                                                                           |");
    printf("%s\n", "|           Example: delo_img -op crop -i test_image.png -u pct -x 0.25 -y 0 -w 0.5 -h 1    |");
    printf("%s\n", "|           Result: Crops out an image using percentagess as units and saves a copy.        |");     
    printf("%s\n", "|                                                                                           |");
    printf("%s\n", "| [Arguments]                                                                               |");
    printf("%s\n", "|           -op     Defines the operation e.g. resize or crop.                              |");
    printf("%s\n", "|           -i      Path to the input image.                                                |");
    printf("%s\n", "|           -u      Units, px = pixels, pct = percentages                                   |");
    printf("%s\n", "|           -s      Scale factor e.g. 0.5 scales image down by half                         |");
    printf("%s\n", "|           -x      X position of the cropping rectangle                                    |");
    printf("%s\n", "|           -y      Y position of the cropping rectangle                                    |");
    printf("%s\n", "|           -w      Weight of the cropping rectangl                                         |");
    printf("%s\n", "|           -h      Height of the cropping rectangle                                        |");
    printf("%s\n", "+-------------------------------------------------------------------------------------------+");
}

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
    if(argc >= 2)
    {
        if(strcmp(argv[1],"-version") == 0)
        {
            printf("%s\n", "delo_img 1.0");
            return 0;
        }
        else if(strcmp(argv[1],"-help") == 0)
        {
            print_help();
            return 0;
        }
    }

    parse_arguments(argc,&argv);    

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
        if(strcmp(args_values[ARG_OP],"crop") == 0)
        {   
            operation = OPERATION_CROP;               
        }
        else if(strcmp(args_values[ARG_OP],"resize") == 0)
        {
            operation = OPERATION_RESIZE;    
        }
        else
        {
            printf("[error]::undefined operation \n");
            return 0;
        }

        input_path = args_values[ARG_I];        
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
            int argument_check = args_defined[ARG_I] * args_defined[ARG_U] * args_defined[ARG_X] * args_defined[ARG_Y] * args_defined[ARG_W] * args_defined[ARG_H];
            
            if(argument_check == 0)
            {
                printf("[error]::not enough arguments \n");
                return 0;
            }
            else
            {
                if(strcmp(args_values[ARG_U],"px") == 0)
                {
                    units = UNITS_PIXELS;            
                }
                else if(strcmp(args_values[ARG_U],"pct") == 0)
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
                if(args_defined[ARG_O])
                {
                    int length = strlen(args_values[ARG_O]);
                    output_path = malloc(sizeof(char) * length+1);
                    memcpy(output_path,args_values[ARG_O], sizeof(char) * length);       
                    output_path[length] = '\0';
                }
                else
                {
                    int length = strlen(input_path);
                    output_path = malloc(sizeof(char) * (length + 9));
                    memcpy(output_path,"cropped_", sizeof(char) * 8);        
                    memcpy(&output_path[8],input_path, sizeof(char) * length);  
                    output_path[length + 8] = '\0';
                }
                 
                

                Image image_cropped; 
                if(units == UNITS_PERCENTAGES)
                {
                    char *ptr;
                    float crop_x = strtof(args_values[ARG_X], &ptr);  
                    float crop_y = strtof(args_values[ARG_Y], &ptr);  
                    float crop_w = strtof(args_values[ARG_W], &ptr);  
                    float crop_h = strtof(args_values[ARG_H], &ptr); 

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
                    int crop_x = strtol(args_values[ARG_X], &ptr,10);  
                    int crop_y = strtol(args_values[ARG_Y], &ptr,10);  
                    int crop_w = strtol(args_values[ARG_W], &ptr,10);  
                    int crop_h = strtol(args_values[ARG_H], &ptr,10); 

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
            int argument_check = args_defined[ARG_I] * args_defined[ARG_S];
            if(argument_check == 0)
            {
                printf("[error]::not enough arguments for resizing \n");
                return 0;
            }
            else
            {
                char *ptr;
                float scale_factor = strtof(args_values[ARG_S], &ptr); 

                char *output_path;  

                if(args_defined[ARG_O])
                {
                    int length = strlen(args_values[ARG_O]);
                    output_path = malloc(sizeof(char) * length+1);
                    memcpy(output_path,args_values[ARG_O], sizeof(char) * length);       
                    output_path[length] = '\0';
                }
                else
                {
                    int length = strlen(input_path);
                    output_path = malloc(sizeof(char) * (length + 9));
                    memcpy(output_path,"resized_", sizeof(char) * 8);        
                    memcpy(&output_path[8],input_path, length);  
                    output_path[length + 8] = '\0';
                }

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