/***************************************************
 * txt2bas, 2026.02. Princz László
 * Convert Mupid Z80 binary file to BTX encoded Mupid program file
 * BPES BASIC text format:
 * Header:
 * - 0x7F (DEL character)
 * - [#_E0-9]
 * Content
 * - line data. [CR|LF]+ replace to one 0x7F (DEL character)
 * Foother
 * - 0x7F
 * - 0x7F
 ****************************************************/
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include "getopt.h"

#define VM 0
#define VS 3
#define VB 'b'

bool verbose = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// Main functions
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void convert( FILE *fin, FILE *fout ) {
    // Header
    fputc( 0x7F, fout );
    fputc( 'E', fout );
    // Content
    for( uint8_t b = fgetc( fin ); !feof( fin ); b = fgetc( fin ) ) {
        if ( ( b == 0x0D ) || ( b == 0x0A ) ) { // EOL
            while( ( !feof( fin ) ) && ( ( b == 0x0D ) || ( b == 0x0A ) ) ) b = fgetc( fin );
            fputc( 0x7F, fout );
            if ( !feof( fin ) ) fputc( b, fout );
        } else {
            fputc( b, fout );
        }
    }
    // Foother
    fputc( 0x7F, fout );
    fputc( 0x7F, fout );
}

void print_usage() {
    printf( "txt2bas v%d.%d%c (build: %s)\n", VM, VS, VB, __DATE__ );
    printf( "PC text file convert Mupid BASIC (BPES) loadable text format.\n");
    printf( "Copyright 2026 by László Princz\n");
    printf( "Usage:\n");
    printf( "txt2bas <input_filename> [<output_filename_without_extension>]\n" );
    printf( "Convert standard text file to Mupid BPES text format for load over BTX.\n" );
    printf( "Command line option:\n");
    printf( "-v           : set verbose mode\n" );
    printf( "-h           : prints this text\n");
    exit(1);
}

int main(int argc, char *argv[]) {
    bool finished = false;
    int argd = argc;

    while ( !finished ) {
        switch ( getopt ( argc, argv, "?hv" ) ) {
            case -1:
            case ':':
                finished = true;
                break;
            case '?':
            case 'h':
                print_usage();
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
        FILE *fin,*fout;
        strcpy( inname, argv[ optind ] );
        strcpy( outname, inname ); // The default output name is input filename + .wav
        if ( argc - optind > 1 ) strcpy( outname, argv[ optind + 1 ] );
        if ( argc - optind > 2 ) print_usage();
        strcat( outname, ".bas" );
        if ( fin = fopen( inname, "rb" ) ) {
            if ( fout = fopen( outname, "wb" ) ) {
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
