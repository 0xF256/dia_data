/*
 * MIT License
 * 
 * Copyright (c) 2025 SmithGoll
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define TAB "    "

#define READ_U8()  get_u8(fp)
#define READ_U16() get_u16(fp)
#define READ_U24() get_u24(fp)
#define READ_U32() get_u32(fp)

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

u64 get_val_impl(FILE *fp, int count)
{
    u64 res = 0;
    for(int i = 0; i < count; i++)
    {
        int t = fgetc(fp);
        if(t < 0)
        {
            fclose(fp);
            fprintf(stderr, "Err: Unexpected EOF!\n");
            exit(-1);
        }

        res |= (u64)t << (i * 8);
    }
    return res;
}

u32 get_u32(FILE *fp)
{
    return (u32)get_val_impl(fp, 4);
}

u32 get_u24(FILE *fp)
{
    return (u32)get_val_impl(fp, 3);
}

u16 get_u16(FILE *fp)
{
    return (u16)get_val_impl(fp, 2);
}

u8 get_u8(FILE *fp)
{
    return (u8)get_val_impl(fp, 1);
}

void show_str(FILE *fp, int strlen)
{
    putchar('\"');

    int c = 0;
    for(int i = 0; i < strlen; i++)
    {
        switch((c = fgetc(fp)))
        {
            case -1:
                fclose(fp);
                fprintf(stderr, "Err: Unexpected EOF!\n");
                exit(-1);
                break;
            case '\n':
                printf("\\n");
                break;
            case '\t':
                printf("\\t");
                break;
            case '\r':
                printf("\\r");
                break;
            default:
                putchar(c);
                break;
        }
    }

    putchar('\"');
}

void depth_printf(int depth, const char *fmt, ...)
{
    if(depth < 0)
    {
        fprintf(stderr, "Err: Wrong depth!\n");
        exit(1);
    }

    do
    {
        printf(TAB);
    }
    while(depth--);

    va_list args;
    va_start(args, fmt);
    vprintf(fmt, args);
    va_end(args);
}

const char *get_direction(int id)
{
    switch(id)
    {
        case 1: return "UP";
        case 2: return "RIGHT";
        case 3: return "DOWN";
        case 4: return "LEFT";
    }
    return "UNK";
}

int main(int argc, const char **argv)
{
    FILE *fp;
    int depth = 0;

    if(argc != 2)
    {
        printf("Usage: demo_parser [FILE]\n");
        return 1;
    }

    fp = fopen(argv[1], "rb");

    if(!fp)
    {
        fprintf(stderr, "Failed to open file: %s\n", strerror(errno));
        return -1;
    }

    if(fseek(fp, 9, SEEK_SET))
    {
        fprintf(stderr, "Failed to seek the file\n");
        fclose(fp);
        return -1;
    }

    int total_demo = READ_U16();

    for(int i = 0; i < total_demo; i++)
    {
        int demo_id = READ_U16();
        int total_instr = READ_U16();
        u32 instr_size = READ_U32();
        int total_spr_id = READ_U16();

        printf("DEMO %d \n{\n", demo_id);

        for(int j = 0; j < total_spr_id; j++)
        {
            READ_U16();
        }
        //if(total_spr_id) putchar('\n');

        int is_in_instr_block = 0;
        int block_instr_count = 0;
        for(int k = 0; k < total_instr; k++)
        {
            int opcode = READ_U8();

            switch(opcode)
            {
                case 0:
                {
                    is_in_instr_block = 1;
                    block_instr_count = READ_U8();

                    if(k != 0) putchar('\n');
                    //depth_printf(depth, "PARALLEL_EXEC_INSTR\n");
                    depth_printf(depth, "{\n");
                    depth++;

                    break;
                }
                case 1:
                {
                    int blockX = READ_U16();
                    int blockY = READ_U16();
                    int frames = READ_U16();

                    depth_printf(depth, "MOVE_CAMERA %d %d %d\n", blockX, blockY, frames);
                    break;
                }
                case 2:
                {
                    int textLines = READ_U8();
                    int offsetY = READ_U16();
                    int strLen = READ_U16();

                    depth_printf(depth, "PAINT_CHAT %d %d ", textLines, offsetY);
                    show_str(fp, strLen);
                    putchar('\n');

                    break;
                }
                case 4:
                {
                    int startX = READ_U16();
                    int startY = READ_U16();

                    // I think this is a mistake made by gameloft
                    // but I will follow the code actually do
                    // int endX = READ_U16();
                    // int endY = READ_U16();
                    int endXY = READ_U16();
                    int UNUSED = READ_U16();

                    int demoSprId = READ_U16();
                    int frameIndex = READ_U16();
                    int frames = READ_U16();

                    depth_printf(depth, "PAINT_SINGLE_DEMO_SPR_WITH_MOVE_ANIM %d %d %d %d %d %d %d\n", startX, startY, endXY, UNUSED, demoSprId, frameIndex, frames);
                    break;
                }
                case 5:
                {
                    int blockX = READ_U16();
                    int blockY = READ_U16();
                    int blockId = READ_U8();

                    depth_printf(depth, "SET_BLOCK_1 %d %d %d\n", blockX, blockY, blockId);
                    break;
                }
                case 6:
                {
                    int frames = READ_U32();

                    depth_printf(depth, "DELAY %d\n", frames);
                    break;
                }
                case 7:
                {
                    depth_printf(depth, "CONFIRM_WITHOUT_TIP\n");
                    break;
                }
                case 9:
                {
                    int blockX = READ_U16();
                    int blockY = READ_U16();
                    int blockId = READ_U16();

                    depth_printf(depth, "SET_BLOCK_2 %d %d %d\n", blockX, blockY, blockId);
                    break;
                }
                case 10:
                {
                    int directionId = READ_U8();

                    depth_printf(depth, "PLAYER_MOVE %s\n", get_direction(directionId));
                    break;
                }
                case 11:
                {
                    int heroSprId = READ_U16();
                    int bgSprAnimId = READ_U16();

                    depth_printf(depth, "SET_DEMO_SPR %d %d\n", heroSprId, bgSprAnimId);
                    break;
                }
                case 12:
                {
                    int posX = READ_U16();
                    int posY = READ_U16();

                    depth_printf(depth, "SET_POS_AND_SHOW_DEMO_SPR %d %d\n", posX, posY);
                    break;
                }
                case 13:
                {
                    int endX = READ_U16();
                    int endY = READ_U16();
                    int frames = READ_U16();

                    // int startX = crtSprOffsetX, startY = crtSprOffsetY;

                    depth_printf(depth, "SET_SPR_MOVE_ANIM %d %d %d\n", endX, endY, frames);
                    break;
                }
                case 14:
                {
                    depth_printf(depth, "SHOW_DEMO_SPR\n");
                    break;
                }
                case 15:
                {
                    depth_printf(depth, "HIDE_DEMO_SPR\n");
                    break;
                }
                case 16:
                {
                    int frameIndex = READ_U16();
                    int blinkCount = READ_U8();

                    depth_printf(depth, "SHOW_DECO_SYMBOL %d %d\n", frameIndex, blinkCount);
                    break;
                }
                case 17:
                {
                    int frameIndex = READ_U16();
                    int needHide = READ_U8();

                    depth_printf(depth, "HIDE_DECO_SYMBOL %d %d\n", frameIndex, needHide);
                    break;
                }
                case 18:
                {
                    int blinkCount = READ_U8();
                    int color = READ_U24();

                    depth_printf(depth, "SCREEN_BLINK %d 0x%06x\n", blinkCount, color);
                    break;
                }
                case 25:
                {
                    int blockX = READ_U16();
                    int blockY = READ_U16();
                    int blockId = READ_U8();
                    int specData = READ_U8();

                    depth_printf(depth, "SET_BLOCK_3 %d %d %d %d\n", blockX, blockY, blockId, specData);
                    break;
                }
                case 26:
                {
                    int blockX = READ_U16();
                    int blockY = READ_U16();
                    int blockId = READ_U32();

                    depth_printf(depth, "SET_BLOCK_4 %d %d %d\n", blockX, blockY, blockId);
                    break;
                }
                case 27:
                {
                    int strLen = READ_U16();

                    depth_printf(depth, "PAINT_HINT ");
                    show_str(fp, strLen);
                    putchar('\n');

                    break;
                }
                default:
                {
                    depth_printf(depth, "UNUSED_%02d\n", opcode);
                    break;
                }
            }

            if(is_in_instr_block && block_instr_count <= 0)
            {
                depth--;
                depth_printf(depth, "}\n");
                if(k != total_instr - 1) putchar('\n');
                is_in_instr_block = 0;
                block_instr_count = 0;
            }
            
            if(is_in_instr_block)
            {
                k--;
                block_instr_count--;
            }
        }

        if(i == total_demo - 1) puts("}");
        else puts("}\n");
    }

    fclose(fp);
    return 0;
}