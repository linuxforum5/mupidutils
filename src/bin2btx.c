/***************************************************
 * bin2btx, 2026.02. Princz László
 * Convert Mupid Z80 binary file to BTX encoded Mupid program file
 * Mupid Telesoftware format
 * 0x1F 0x3C ADDR3 PAYLOAD END
 * ADDR3 (3 bytes)
 *   - 0  1 0   b   a15 a14 a13 a12
 *   - 0  1 a11 a10 a9  a8  a7  a6
 *   - 0  1 a5  a4  a3  a2  a1  a0
 *     b = 0 : Load into BANK 2
 *         1 : Load into BANK 3
 *     [ a15 - a0 ] : start address for load
 * DATA bytes
 *   - 0  1 b7 b6 b5 b4 b3 b2
 *   - 0  1 b1 b0 c7 c6 c5 bc
 *   - 0  1 c3 c2 c1 c0 d7 d6
 *     ....
 *     Decoded byte if all bit exists. [ b7-0 ], [ c7-0 ]
 * END one byte
 *   - 0x21 : End teleprogram
 *   - 0x22 : End data block
 *   - 0x28 : End teleprogram, and START ADDR3 + PROGBA (???)
 *   - 0x29 : End teleprogram, and START ADDR3
 *   - 0x2A : End teleprogram, and start BASIC
 ****************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "getopt.h"
#include "math.h"

#define VM 0
#define VS 2
#define VB 'b'

bool verbose = false;
unsigned int load_addr = 0x8100;
unsigned int load_bank = 2;
unsigned char progress_row = 0; // the progress bar row. 0=no progress bar

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Address functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void writeLoadAddr( FILE *fout, unsigned int load_addr ) {
    unsigned char b3 = ( load_addr % 64 ) | 0x40;
    load_addr /= 64;
    unsigned char b2 = ( load_addr % 64 ) | 0x40;
    load_addr /= 64;
    unsigned char b1 = ( load_addr % 64 ) | 0x40;
    if ( load_bank == 3 ) b1 |= 0x10;
    fputc( b1 , fout );
    fputc( b2 , fout );
    fputc( b3 , fout );
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Encode functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned long bits = 0;          // Bits for encoding
unsigned char bit_counter = 0;   // Bit counter in data

unsigned char encodeNextByte() {
    unsigned long bit_mask = 0x3F; // Mask for 6 bits
    unsigned long byte = 0;
    if ( bit_counter >= 6 ) {
        bit_mask = bit_mask << ( bit_counter - 6 );
        byte = bits & bit_mask;
        byte = byte >> ( bit_counter - 6 );
        bit_counter -= 6;
    } else {
        bit_mask = bit_mask >> ( 6 - bit_counter );
        byte = bits & bit_mask;
        byte = byte << ( 6 - bit_counter );
        bit_counter = 0;
    }
    bits = bits % bit_mask;
    return byte | 0x40;
}

void convert_block( FILE *fin, FILE *fout, unsigned int load_addr, long size, unsigned char end_of_block ) {
    fputc( 0x1F, fout );
    fputc( 0x3C, fout );
    writeLoadAddr( fout, load_addr );
    for( long pos = 0; pos < size; pos++ ) {
    // for( unsigned char c = fgetc( fin ); !feof( fin ); c = fgetc( fin ) ) {
        unsigned char c = fgetc( fin );
        bits = bits * 256 + c;
        bit_counter += 8;
        while ( bit_counter > 5 ) fputc( encodeNextByte(), fout );
    }
    while ( bit_counter ) fputc( encodeNextByte(), fout );
    fputc( end_of_block, fout );
}

long get_file_size( FILE* fp ) {
    if ( fp == NULL ) return -1;
    fseek( fp, 0L, SEEK_END ); // Elugrunk a fájl végére
    long size = ftell( fp );    // Lekérdezzük az aktuális pozíciót (ez a méret)
    fseek( fp, 0, SEEK_SET ); // Elugrunk a fájl végére
    return size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Progressbar functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void put_progressbar_start_block( FILE *fout, unsigned char row ) {
    fputc( 0x1F, fout );
    fputc( 64+row, fout );
    fputc( 0x41, fout );

//    fputc( 0x19, fout ); // 
//    fputc( ';', fout ); // # W
    fputc( 0x1D, fout ); // 
    fputc( 'Q', fout ); // # W

    fputc( 0x12, fout ); // repeat
    fputc( 0x40+39, fout ); // repeat
    fputc( 0x1F, fout );
    fputc( 64+row, fout );
    fputc( 0x41, fout );
}

void put_progressbar_step_block( FILE *fout, unsigned char row, unsigned char col ) {
    fputc( 0x1F, fout );
    fputc( 0x40+row, fout );
    fputc( 0x40+col, fout );
    fputc( 0x1D, fout ); // 
    fputc( 0x82, fout ); // # W
    fputc( 127, fout ); // # W
//    fputc( 0x7C, fout ); // White box
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Main functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void convert( FILE *fin, FILE *fout ) {
    long size = get_file_size( fin );
    if ( progress_row ) {
        long block_size = size / 40 + 1;
        long sum_size = 0;
        unsigned int current_load_addr = load_addr;
        put_progressbar_start_block( fout, progress_row );
        for( unsigned char i = 0; i<40; i++ ) {
            if ( i == 39 ) {
                convert_block( fin, fout, current_load_addr, size - 39 * block_size, 0x22 );
                put_progressbar_step_block( fout, progress_row, i+1 );
                convert_block( fin, fout, load_addr, 0, 0x29 );
            } else {
                convert_block( fin, fout, current_load_addr, block_size, 0x22 );
                put_progressbar_step_block( fout, progress_row, i+1 );
            }
            current_load_addr += block_size;
        }
    } else { // All in one
        convert_block( fin, fout, load_addr, size, 0x29 );
    }
}

void print_usage() {
    printf( "bin2btx v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "Z80 binary to Mupid Teleprogram (BTX) format converter.\n");
    printf( "Copyright 2026 by László Princz\n");
    printf( "Usage:\n");
    printf( "bin2btx <input_filename> [<output_filename_without_extension>]\n" );
    printf( "Convert z80 binary code to Mupid Teleprogram format in BTX code.\n" );
    printf( "Command line option:\n");
    printf( "-v           : set verbose mode\n" );
    printf( "-l hexAddr   : load address. Default value is 0x%04X\n", load_addr );
    printf( "-b [2|3]     : load into BANK 2 or 3. Default value is %d\n", load_bank );
    printf( "-B BtxScreen : BTX screen befor load. Default is empty.\n", load_bank );
    printf( "-p row       : progress bar in row ([1-24]). Default is no progress bar.\n", load_bank );
    printf( "-h           : prints this text\n");
    exit(1);
}

void copy( FILE *fbtx, FILE *fout ) {
    for( unsigned char c = fgetc( fbtx ); !feof( fbtx ); c = fgetc( fbtx ) ) {
        fputc( c, fout );
    }
}

int main(int argc, char *argv[]) {
    bool finished = false;
    unsigned char btxname[1024] = { 0 };
    int argd = argc;

    while ( !finished ) {
        switch ( getopt ( argc, argv, "?hvl:b:B:p:" ) ) {
            case -1:
            case ':':
                finished = true;
                break;
            case '?':
            case 'h':
                print_usage();
                break;
            case 'B' :
                sscanf( optarg, "%s", &btxname );
                break;
            case 'l' :
                if ( !sscanf( optarg, "%x", &load_addr ) ) {
                    fprintf( stderr, "Error parsing argument for '-l'.\n");
                    exit(2);
                }
                break;
            case 'p' :
                if ( !sscanf( optarg, "%i", &progress_row ) ) {
                    fprintf( stderr, "Error parsing argument for '-p'.\n");
                    exit(2);
                }
                if ( ( progress_row < 1 ) || ( progress_row > 24 ) ) {
                    fprintf( stderr, "Progress bar is between 1 and 24\n");
                    exit(2);
                }
                break;
            case 'b' :
                if ( !sscanf( optarg, "%i", &load_bank ) ) {
                    fprintf( stderr, "Error parsing argument for '-b'.\n");
                    exit(2);
                }
                if ( ( load_bank < 2 ) || ( load_bank > 3 ) ) {
                    fprintf( stderr, "Load Bank is only 2 or 3\n");
                    exit(2);
                }
                break;
            case 'v':
                verbose = true;
                break;
            default:
                break;
        }
    }

    if ( argc - optind > 0 ) {
        char inname[ 80 ]; // Input filename
        char outname[ 80 ]; // Output filename
        FILE *fin,*fout,*fbtx;
        strcpy( inname, argv[ optind ] );
        strcpy( outname, inname ); // The default output name is input filename + .wav
        if ( argc - optind > 1 ) strcpy( outname, argv[ optind + 1 ] );
        if ( argc - optind > 2 ) print_usage();
        strcat( outname, ".btx" );
        if ( fin = fopen( inname, "rb" ) ) {
            if ( fout = fopen( outname, "wb" ) ) {
                if ( btxname[0] ) {
                    if ( fbtx = fopen( btxname, "rb" ) ) {
                        copy( fbtx, fout );
                        fclose( fbtx );
                    } else {
                        fprintf( stderr, "Error open %s.\n", btxname );
                        exit(4);
                    }
                }
                convert( fin, fout );
                fclose( fout );
                fclose( fin );
            } else {
                fprintf( stderr, "Error creating %s.\n", outname );
                exit(4);
            }
        } else {
            fprintf( stderr, "Error opening %s.\n", inname );
            exit(4);
        }
    } else {
        print_usage();
    }
}
