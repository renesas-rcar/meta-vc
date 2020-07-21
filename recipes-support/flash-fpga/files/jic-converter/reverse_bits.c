/* 
 * This application reverse the bit order of each byte in the input
 * stream and writes it to the standard output.
 *
 * Two different implementations of the reverse() function are listed
 * below.
 * 
 */

#include <stdio.h>
#include <stdbool.h>

#if 1

/* First the left four bits are swapped with the right four bits. Then
   all adjacent pairs are swapped and then all adjacent single
   bits. */

unsigned char reverse(unsigned char b)
{
  b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
  b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
  b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
  return b;
}

#else

/* Different implementation that uses a lookup table. */

static const unsigned char lookup[16] = {
  0x0, 0x8, 0x4, 0xC, 0x2, 0xA, 0x6, 0xE,
  0x1, 0x9, 0x5, 0xD, 0x3, 0xB, 0x7, 0xF,
};

unsigned char reverse(unsigned char b)
{
  /* Reverse top and bottom nibble then swap them */
  return  (lookup[b & 0xF] << 4) | lookup[b >> 4];
}

#endif



#define BUF_SIZE        256

int main(int argc, char **argv)
{
  size_t n_read;
  size_t n_write;
  unsigned char buf[BUF_SIZE];
  int i;
  FILE *fp_in, *fp_out;

  if (argc > 1)
    {
      fp_in = fopen(argv[1], "rb");
    }
  else
    {
      fp_in = freopen(NULL, "rb", stdin);
    }
  
  if (!fp_in)
    {
      /* error */
      fprintf(stderr, "ERROR: Cannot read input file in binary mode!\n");
      return 1; 
    }
  
  fp_out = freopen(NULL, "wb", stdout);
  if (!fp_out)
    {
      /* error */
      fprintf(stderr, "ERROR: Cannot write output file in binary mode!\n");
      return 1; 
    }
  
  /* Loop until EOF */
  while (true)
    {
      n_read = fread(buf, sizeof(unsigned char), BUF_SIZE, fp_in);

      if (n_read < 0)
        {
          /* error */
          fprintf(stderr, "ERROR: Reading input file failed!\n");
          return 1; 
        }
      
      if (n_read == 0)
        {
          /* end of file */
          return 0;
        }

      for (i=0; i<n_read; ++i)
        {
          buf[i] = reverse(buf[i]);
        }

      n_write = fwrite(buf, sizeof(unsigned char), n_read, fp_out);
      if (n_write != n_read)
        {
          fprintf(stderr, "ERROR: Writing to output file failed!\n");
          return 1;
        }
    }
}
