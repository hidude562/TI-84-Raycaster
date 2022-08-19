/* You probably want to keep these headers */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>
//optmize;


/* Here are some standard headers. Take a look at the toolchain for more. */
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <graphx.h>
#include <keypadc.h>
/* Put function prototypes here or in a header (.h) file */

/* Note: uint8_t is an unsigned integer that can range from 0-255. */
/* It performs faster than just an int, so try to use it (or int8_t) when possible */
void printText(const char *text, uint8_t x, uint8_t y);

/* This is the entry point of your program. */
/* argc and argv can be there if you need to use arguments, see the toolchain example. */
int main() {

    //uint16_t r_quality = 1000; //200
    int render_downscale = 10;
    boot_Set48MHzMode();
    uint8_t backc = 0x19;
    int x = 4100;
    int y = 4100;
    float float_i = 0;
    uint16_t roomRes = 10;
    uint16_t total_size = roomRes*roomRes;
    int square_size = 1000;
    bool wall[total_size]; //= {false, false, false, false, false, false, true, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false, false};
    uint16_t i;
    uint16_t i2;
    uint16_t i3;
    uint16_t screenX = 320;
    bool hit;
    uint8_t brightness_level;

    //float rot_step = 60/((float)screenX);//60/screenX;
    //char* int_str;
    int temp_mod;
    int rotX = 1;
    int tmp_rotX = 0;
    int_fast32_t rayX;
    int_fast32_t rayY;
    //int stepX;
    //int stepY;
    int x_dist; //Make unsinged prob
    int y_dist; //same for this

    int rotX_cos_mod_fix;
    int rotX_sin_mod_fix;

    int acc_dist;

    int rayCos; //IDK if it is faster to use one of all the same type or more efficient other types
    int raySin;
    int wall_index;//Should be unsigned but it is fun to raycast all of your calculator's memory
    uint16_t dist; //Should be 8bit but i am currently testing
    int cos_cache[360];

    float_i = 0;
    for(i=0; i < 360; i++){
        cos_cache[i] = (int)(cos(float_i*0.01745)*1000);//*(r_quality)); // all floating point ints are scaled by 1k, also stored in degrees
        float_i++;
    }


    //float rot_step = 60/screenX;

    //Random walls

    i3 = 0;
    for(i=0; i < roomRes; i++) {
        for(i2=0; i2<roomRes; i2++) {
            if(i==0 || i==roomRes-1 || i2==0 || i2==roomRes-1){
                wall[i3] = true;
            } else {
                wall[i3] = rand()%2;;//false
            }
            i3++;
        }
    }


    //Note, eventually add differential height

    os_ClrHomeFull();
    gfx_Begin(gfx_8bpp);
    //gfx_SetDrawBuffer(); Use this in a loop or something
    gfx_FillScreen(backc);
    gfx_SetTextFGColor(0xFE);
    gfx_SetColor(255);



    while (true){
        tmp_rotX=rotX-32; //FOV is locked at 64
        gfx_BlitScreen();
        for(i = 0; i < screenX; i+=render_downscale){
            tmp_rotX+=2;
            rayCos = cos_cache[abs(tmp_rotX%360)];
            raySin = cos_cache[abs((tmp_rotX-90)%360)];


            //Calculations
            dist = 0;
            acc_dist = 0;
            rayX = x;
            rayY = y;

            hit = false;
            while(dist<10) {
                dist+=1;

                temp_mod = (0-rayX)%square_size;
                if(temp_mod == 0){
                    x_dist = square_size;
                } else {
                    if(rayCos>0) {
                        if(temp_mod<0){
                             x_dist = temp_mod+square_size;
                        } else{
                             x_dist = temp_mod;
                        }
                    } else {
                        temp_mod = (0-rayX)%square_size; //Wrong likely
                        if(temp_mod<0){
                             x_dist = abs(temp_mod);
                        } else{
                             x_dist = temp_mod;
                        }
                    }
                }

                temp_mod = rayY%square_size;
                if(temp_mod == 0){
                    y_dist = square_size;
                } else {
                    if(raySin<0) {
                        if(temp_mod<0){
                             y_dist = temp_mod+square_size;
                        } else{
                             y_dist = temp_mod;
                        }
                    } else {
                        temp_mod = (0-rayY)%square_size;
                        if(temp_mod<0){
                             y_dist = temp_mod+square_size;
                        } else{
                             y_dist = temp_mod;
                        }
                    }
                }
                //UNLIKELY TO WORK PROBABLY


                //y_dist = 0;

                x_dist = (x_dist * ((square_size*100)/abs(rayCos)))/100;
                y_dist = (y_dist * ((square_size*100)/abs(raySin)))/100;
                x_dist+=100;
                y_dist+=100;

                //x_dist=201;
                //y_dist=200;
                if(x_dist < y_dist) {
                    rayX+=(rayCos*x_dist)/(square_size); //NOTE it is going a distance of 1000*1000 so maybe divide by 1k?
                    rayY+=(raySin*x_dist)/(square_size);
                    acc_dist+=(x_dist);
                } else { //Maybe can optmize dis
                    rayX+=(rayCos*y_dist)/(square_size);
                    rayY+=(raySin*y_dist)/(square_size);
                    acc_dist+=(y_dist);
                }

                //stepX = rayCos;
                //stepX = raySin;//It would work something like this then test if one is less than the other

                //rayX+=rayCos*stepX;
                //rayY+=raySin*stepX;
                wall_index = ((rayX/1000) + ((rayY/1000)*roomRes)); //slow
                if(wall[wall_index]!=0){ //wall[wall_index]==void// || wall_index<0 || wall_index > total_size-1
                    hit = true;
                    break;
                }
            }
            //cc_dist = (int)(100/sqrt((((rayX-x)/1000)*((rayX-x)/1000)) + (((rayY-y)/1000)*((rayY-y)/1000))));
            //acc_dist=1000/sqrt(abs(rayX-x) + abs(rayY-y));//(int)(1000/sqrt((((rayX-x)/10)*((rayX-x)/10)) + (((rayY-y)/10)*((rayY-y)/10))));


            temp_mod = abs((tmp_rotX - rotX)%360);

            acc_dist = (acc_dist * cos_cache[temp_mod])/1000;
            acc_dist = 5000/sqrt(acc_dist);  //100/dist;

            brightness_level = (acc_dist*acc_dist)/3000;
            if(brightness_level > 7){
                brightness_level = 7;
            }

            //gfx_VertLine_NoClip(i,120-(dist/2),dist);
            gfx_SetColor(0x19);
            gfx_FillRectangle(i,0,render_downscale,320); //Clear up junk, optmize this
            if(hit==true){
                if(y_dist < x_dist){
                    gfx_SetColor((32*(brightness_level))+0); //(acc_dist/20)%256
                } else {
                    gfx_SetColor((1*brightness_level)+0); //((acc_dist/20)+10)%256
                }
                gfx_FillRectangle(i,120-(acc_dist/2),render_downscale,acc_dist);
            }

            if(i == render_downscale*6){
                gfx_PrintStringXY("X:", 0, 0);
                gfx_PrintInt(x/1000, 2);

                gfx_PrintStringXY("Y:", 28, 0);
                gfx_PrintInt(y/1000, 2);
            }

            //gfx_FilL
        }

        if(rotX < 0){
            rotX_cos_mod_fix = ((rotX)%360)+360;
        } else {
            rotX_cos_mod_fix = ((rotX)%360);
        }

        if(rotX-90 < 0){
            rotX_sin_mod_fix = ((rotX-90)%360)+360;
        } else {
            rotX_sin_mod_fix = ((rotX-90)%360);
        }

        if(kb_Data[7] & kb_Down){
            x-=(cos_cache[rotX_cos_mod_fix])/10;
            y-=(cos_cache[rotX_sin_mod_fix])/10;
        }

        if(kb_Data[7] & kb_Left){
            x+=(cos_cache[rotX_sin_mod_fix])/10;
            y-=(cos_cache[rotX_cos_mod_fix])/10;
        }

        if(kb_Data[7] & kb_Right){
            x-=(cos_cache[rotX_sin_mod_fix])/10;
            y+=(cos_cache[rotX_cos_mod_fix])/10;
        }

        if(kb_Data[7] & kb_Up){
            x+=(cos_cache[rotX_cos_mod_fix])/10;
            y+=(cos_cache[rotX_sin_mod_fix])/10;
        }

        if(kb_Data[1] & kb_2nd){
            rotX-=15;
        }

        if(kb_Data[1] & kb_Mode){
            rotX+=15;
        }
        //rotX+=30;

        if(kb_On){
            break; //Stops program
        }

        gfx_SwapDraw();
    }

        //A for loop that copies temp_life to lifez
    gfx_End();
    return 0;
}
