/**
 * Split 1F 3C started blocks from BTX content, and save it into binary format
 */
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

bool verbose = true;

void error( const char* msg ) {
    printf( "ERROR: %s\n", msg );
    exit(1);
}

uint8_t btx_read_b6( FILE *f ) { return fgetc( f ) & 63; }

uint32_t decode_block( FILE *f, char* fn ) {
    uint32_t byte_counter = 0;
    uint32_t b18 = btx_read_b6( f ) * 64 * 64 + btx_read_b6( f ) * 64 + btx_read_b6( f );
    uint32_t b2 = b18 % 256;      // LOW
    b18 = b18 / 256;
    uint32_t b1 = b18 % 256;      // HIGH
    b18 = b18 / 256;
    uint32_t start_address = b1 * 256 + b2; // Start address
    uint32_t bank_id = b18;                 // Bank

    char dstfn[100] = {0};
    sprintf( dstfn, "%s.%02b.%04X.bin", fn, bank_id, start_address );
    FILE *dst = fopen( dstfn, "wb");
    if (!dst) { perror("cannot create file"); return 1; }
    uint8_t b = 0;
    if ( !feof( f ) ) {
        int bit_counter = 0;
        uint32_t bits = 0;
        for( b = fgetc( f ); (!feof( f )) && (b >= 0x40); b = fgetc( f ) ) {
            bits = (bits << 6) + (b & 63);
            bit_counter += 6;
            if ( bit_counter > 7 ) {
                bit_counter -= 8;
                uint32_t curr = bits >> bit_counter;
                bits -= curr << bit_counter;
                fputc( curr, dst );
                byte_counter++;
            }
        }
        printf( "%d.%04X %d bytes end with %02X\n", bank_id, start_address, byte_counter, b );
    }
    fclose( dst );
    char ndstfn[100] = {0};
    sprintf( ndstfn, "%s.%02b.%04X-%02X.bin", fn, bank_id, start_address, b );
    rename( dstfn, ndstfn );
    return byte_counter;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("usage: btx2bin btx_file\n");
        return 1;
    }
    FILE *f = fopen( argv[1], "rb");
    if (!f) { perror("cannot open file"); return 1; }
    uint32_t byte_counter = 0;
    for( uint8_t b=fgetc( f ); !feof( f ); b = fgetc( f ) ) {
        if ( b == 0x1F ) {
            if ( fgetc(f) == 0x3C ) {
                byte_counter = decode_block( f, argv[1] );
            }
        }
    }
    fclose(f);
    return 0;
}
