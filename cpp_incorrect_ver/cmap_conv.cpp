#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <png.h>
#include "stream.h"

#ifdef __linux__
//#define MAX_PATH 255
#endif

#define CHIP_REGIONS 300

typedef struct
{
    float x1;
    float y1;
    float x2;
    float y2;
    bool used;
} region_t;

typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
} rgba_t;

char inp_cmap[MAX_PATH];
char out_cmap_new[MAX_PATH];
char inp_chip[MAX_PATH];
char out_glmap[MAX_PATH];
char out_cmap[MAX_PATH];

png_bytep png_buffer;
int png_size;
int png_w, png_h;
region_t chip_regions[CHIP_REGIONS];
rgba_t fon;
bool *is_used = NULL;

void prepare_filenames(int argc, char **args)
{
    strcpy(inp_cmap, args[1]);
    strcat(inp_cmap, "_wide.cmap");
    strcpy(out_cmap_new, args[1]);
    strcat(out_cmap_new, "_wide._cmap");
    strcpy(inp_chip, args[1]);
    strcat(inp_chip, "_chip.png");

    strcpy(out_glmap, args[1]);
    strcat(out_glmap, ".glmap");
    strcpy(out_cmap, args[1]);
    strcat(out_cmap, "_chip.glmap");
}

bool convert_cmap()
{
    FILE *f = fopen(inp_cmap, "rb");

    if (!f)
    {
        ERROR_MESSAGE("CMAP not exist\n");
        return false;
    }

    uint32_t width, height;
    fread(&width, 1, sizeof(uint32_t), f);
    fread(&height, 1, sizeof(uint32_t), f);
    printf("Image %ix%i\n", width, height);

    uint8_t *data = new uint8_t[width * height];
    //fseek(f, 0, SEEK_SET);
    fread(data, 1, width * height, f);
    memset(data, 0, sizeof(uint32_t)*2);
    fclose(f);

    region_t regions[100];
    memset(regions, 0, sizeof(region_t)*100);

    int count = 0;
    for (int i=0 ; i<height ; i++)
    {
        for (int j=0 ; j<width ; j++)
        {
            uint8_t idx = data[i*width + j];
            if (idx)
            {
                if (!regions[idx].used)
                {
                    regions[idx].x1 = (j*1.) / width;
                    regions[idx].y1 = (i*1.) / height;
                    regions[idx].used = true;
                    count++;
                }
                else if ((((j+1) == width) || (data[i*width + j + 1] != idx)) && ((i+1 == height) || (data[(i+1)*width + j] != idx)))
                {
                    regions[idx].x2 = (j*1.) / width;
                    regions[idx].y2 = (i*1.) / height;
                    printf("1");
                }
            }
        }
    }

    f = fopen(out_glmap, "wt");
    fprintf(f, "[Regions]=%i\n", count);
    int idx = 0;
    for (int i=0 ; i<100 ; i++)
        if (regions[i].used)
        {
            fprintf(f, "[Region%i]=0 %f %f %f %f\n", idx + 1, regions[i].x1, regions[i].y1, regions[i].x2, regions[i].y2);
            idx++;
        }

    fclose(f);
    delete data;
    return true;
}

bool load_tex(const char *buf, uint32_t sz)
{
    png_image image;
    memset(&image, 0, sizeof(png_image));
    image.version = PNG_IMAGE_VERSION;

    if (png_image_begin_read_from_memory(&image, buf, sz))
    {
        image.format = PNG_FORMAT_RGBA;

        png_buffer = (png_bytep)malloc(PNG_IMAGE_SIZE(image));
        png_size = PNG_IMAGE_SIZE(image);

        if (png_buffer != NULL)
        {
            if (png_image_finish_read(&image, NULL, png_buffer, 0, NULL))
            {
                png_w = image.width;
                png_h = image.height;
                printf("PNG %ix%i\n", png_w, png_h);

                is_used = new bool[png_w * png_h];
                memset(is_used, 0, png_w * png_h);

                return true;
            }
        }
    }
    else
    {
        return false;
    }

    return true;
}

bool load_png(const char *name)
{
    char fn[MAX_PATH];
    strcpy(fn, name);
    if (!strstr(name, ".png"))
    {
        strcat(fn, ".png");
    }

    Stream *str = new Stream(fn, FILE_OPEN_READ_ST);
    if ((str == NULL) || (str->getFileStreamHandle() == INVALID_HANDLE_VALUE))
    {
        ERROR_MESSAGE("PNG: Doesn\'t load texture %s\n", fn);
        return false;
    }
    uint32_t sz = str->getSize();
    char buf[sz];
    str->read(buf, sz);
    delete str;

    return load_tex(buf, sz);
}

bool in_exist_region(float x, float y)
{
    for (int i=0 ; i<CHIP_REGIONS ; i++)
    {
        if ((chip_regions[i].used) &&
            (chip_regions[i].x1 <= x) && (chip_regions[i].x2 >= x) &&
            (chip_regions[i].y1 <= y) && (chip_regions[i].y2 >= y))
        {
            return true;
        }
    }
    return false;
}

int x_min, x_max, y_min, y_max;

#define COMPARE_PIX() (pixel->a > 0)

void scan_pixel(int x, int y)
{
    if (!is_used[y*png_w + x])
    {
        is_used[y*png_w + x] = true;
        rgba_t *pixel = (rgba_t*)png_buffer + (y*png_w + x);
        if (!COMPARE_PIX())
            return;

        if (x < x_min)
            x_min = x;
        else if (x > x_max)
            x_max = x;
        if (y < y_min)
            y_min = y;
        else if (y > y_max)
            y_max = y;

        if (x > 0)
            scan_pixel(x - 1, y);
        if (y > 0)
            scan_pixel(x, y - 1);
        if (x < png_w-1)
            scan_pixel(x + 1, y);
        if (y < png_h-1)
            scan_pixel(x, y + 1);
    }
}

void convert_chip()
{
    memcpy(&fon, png_buffer, sizeof(rgba_t));
    memset(chip_regions, 0, sizeof(region_t)*CHIP_REGIONS);

    uint32_t count = 0;
    for (int i=0 ; i<png_h ; i++)
    {
        for (int j=0 ; j<png_w ; j++)
        {
            rgba_t *pixel = (rgba_t*)png_buffer + (i*png_w + j);
            //memcpy(pixel, &fon, sizeof(rgba_t));
            if (COMPARE_PIX())
            {
                float x = 1.0 * j / png_w;
                float y = 1.0 * i / png_h;
                if (!in_exist_region(x, y))
                {
                    x_min = x_max = j;
                    y_min = y_max = i;
                    scan_pixel(j, i);

                    //printf("Region %i %ix%i-%ix%i\n", count + 1, x_min, y_min, x_max, y_max);
                    chip_regions[count].x1 = x_min * 1. / png_w;
                    chip_regions[count].y1 = y_min * 1. / png_h;
                    chip_regions[count].x2 = x_max * 1. / png_w;
                    chip_regions[count].y2 = y_max * 1. / png_h;
                    chip_regions[count].used = true;
                    count++;
                }
            }
        }
    }

    FILE *f = fopen(out_cmap, "wt");
    fprintf(f, "[Regions]=%i\n", count);
    int idx = 0;
    for (int i=0 ; i<CHIP_REGIONS ; i++)
        if (chip_regions[i].used)
        {
            fprintf(f, "[Region%i]=0 0 %f %f %f %f\n", idx + 1,
                    chip_regions[i].x1, chip_regions[i].y1,
                    chip_regions[i].x2, chip_regions[i].y2);
            idx++;
        }

    fclose(f);
}

int main(int argc, char **args)
{
    if (argc == 1)
    {
        printf("cmap_conv [input_file]\n\tExamle: cmap_conv ./System/Title/Title\n");
        return 0;
    }

    printf("%s\n", args[1]);
    prepare_filenames(argc, args);

    if (convert_cmap())
    {
        //rename(inp_cmap, out_cmap_new);
        if (load_png(inp_chip))
            convert_chip();
    }
}
