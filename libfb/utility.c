#include <libfb/fb_lib.h>

#ifdef HAVE_TIME_H
# include <time.h>
#endif

#ifdef HAVE_STDLIB_H
# include <stdlib.h>
#endif

/** @file utility.c
 *
 * @brief  Utility functions, such as formatting and print functions
 *
 */

/** Map a FEATURE to the build number where this feature set begins. */
const uint16_t buildnum_featureset[FEATURE_MAX] = { 35, 36 };

/** @brief print a MAC address in a familiar format
 *
 * @param output the stream to print to (i.e. stdout)
 * @param mac the MAC address to print
 */
void
fprint_mac (FILE * output, const volatile unsigned char *mac)
{
  fprintf (output, "%02x:%02x:%02x:%02x:%02x:%02x\n", mac[0], mac[1], mac[2],
	   mac[3], mac[4], mac[5]);
}

/** @brief print a MAC address in a familiar format to stdout
 *
 * @param mac the MAC address to print
 */
void
print_mac (const volatile unsigned char *mac)
{
  fprint_mac (stdout, mac);
}

/** @brief print an IP address in a familiar format
 *
 * @param stream the stream to print to (i.e. stdout)
 * @param ip the IP address to print 
 */
void
fprint_ip (FILE * stream, uint32_t ip)
{
  fprintf (stream, "%d.%d.%d.%d\n", ip & 0xFF, (ip & 0xFF00) >> 8,
	   (ip & 0xFF0000) >> 16, (ip & 0xFF000000) >> 24);
}

/** @brief print an IP address in a familiar format to stdout
 *
 * @param ip the IP address to print 
 */
void
print_ip (uint32_t ip)
{
  fprint_ip (stdout, ip);
}


/** @brief calculate the CRC16 of a buffer
 *
 * @param buf the buffer
 * @param len the length of the buffer
 * @return the calculated CRC16
 */
unsigned short
crc_16 (unsigned char *buf, int len)
{
  /* CRC16-CCITT */
  unsigned short i, crc;
  unsigned char xor, slice, bit, align;

#ifdef DEBUG
  fprintf (stderr, "{crc_16: len(%d)}\n", len);
#endif
  crc = 0xFFFF;
  for (i = 0; i < len; i++)
    {
      slice = buf[i];
      align = 0x80;
      for (bit = 0; bit < 8; bit++)
	{
	  if (crc & 0x8000)
	    xor = 1;
	  else
	    xor = 0;
	  crc = crc << 1;
	  if (slice & align)
	    crc = crc + 1;
	  if (xor)
	    crc = crc ^ CRCPOLY;
	  align = align >> 1;
	}
    }

  for (bit = 0; bit < 16; bit++)
    {
      if (crc & 0x8000)
	xor = 1;
      else
	xor = 0;
      crc = crc << 1;
      if (xor)
	crc = crc ^ CRCPOLY;
    }
  return crc;
}


/* @brief Parse a MAC address into raw 6-byte memory location
 *
 * Takes src_mac in ASCII format (AABBCCDDEEFF) or colon-delimited
 * (AA:BB:CC:DD:EE:FF) and translates to dst_mac
 * 
 * @param src_mac null terminated string representing a MAC address.  
 * @param dst_mac destination memory location, must be at least 6 bytes
 * @return 0 on success and non-zero otherwise.  long.
 */
int
parse_mac (char *src_mac, unsigned char *dst_mac)
{
  char hex[2];
  int x, y;
  int result = 0;
  if (strchr (src_mac, ':') == NULL)
    {
      if (src_mac[0] == '\n')
	return -1;

      for (x = 0, y = 0; x < 6; x++)
	{
	  hex[0] = src_mac[y++];
	  hex[1] = src_mac[y++];
	  dst_mac[x] = strtol (hex, NULL, 16);
	}
    }
  else
    {
      /* Assume we are colon delimited */
      result = sscanf (src_mac, "%2hhX:%2hhX:%2hhX:%2hhX:%2hhX:%2hhX",
		       &dst_mac[0], &dst_mac[1], &dst_mac[2], &dst_mac[3],
		       &dst_mac[4], &dst_mac[5]);
      if (result == 6)		/* 6 would imply success here. */
	result = 0;
    }
  return result;
}


/** @brief print the current time to a stream
 *  @param output the stream to print to 
 */
void
print_current_time (FILE * output)
{
  struct tm *time_info;
  time_t calendar_time;

  calendar_time = time (NULL);
  time_info = localtime (&calendar_time);
  fprintf (output, "Time: [%d/%d/%d %d:%d:%d]\n", time_info->tm_mon + 1,
	   time_info->tm_mday, time_info->tm_year + 1900, time_info->tm_hour,
	   time_info->tm_min, time_info->tm_sec);
}

/** @brief pack a 32-bit value into an 8-bit memory
 * @param val the value to pack
 * @param dst the destination memory location
 */
void
store32 (u_int32_t val, u_int8_t * dst)
{
  dst[0] = val & 0xff;
  dst[1] = val >> 8;
  dst[2] = val >> 16;
  dst[3] = val >> 24;
}

/** @brief unpack a 32-bit value from 8-bit memory
 *  @param src the 8-bit memory start location
 *  @return unpacked 32-bit value
 */
u_int32_t
grab32 (const volatile u_int8_t * src)
{
  return ((src[0] & 0xff) | (src[1] << 8) | (src[2] << 16) | (src[3] << 24));
}

/** @brief unpack a 16-bit value from 8-bit memory
 *  @param src the 8-bit memory start location
 *  @return unpacked 16-bit value
 */
u_int16_t
grab16 (const volatile u_int8_t * src)
{
  return ((src[0] & 0xff) | (src[1] << 8));
}

/** @brief pack a 16-bit value into an 8-bit memory
 * @param val the value to pack
 * @param dst the destination memory location
 */
void
store16 (u_int16_t val, u_int8_t * dst)
{
  dst[0] = val & 0xff;
  dst[1] = val >> 8;
}

/*********** Span configuration (informational) functions ************/

/** @brief Print the configuration of a T1/E1/J1 span
 *  @param link the link configuration
 *  @param output the stream to print to 
 */
void
print_span_mode_idtlink (IDT_LINK_CONFIG link, FILE * output)
{
  if (link.E1Mode)
    {
      fprintf (output, "E1");
      if (link.CRCMF)
	fprintf (output, " (CRC4)");
    }
  else
    {
      fprintf (output, "T1");
      if (link.framing)
	fprintf (output, ",ESF");
      else
	fprintf (output, ",SF");
    }

  if (link.encoding)
    fprintf (output, ",AMI");
  else if (link.E1Mode)
    fprintf (output, ",HDB3");
  else
    fprintf (output, ",B8ZS");

  if (link.rbs_en)
    fprintf (output, ",RBS");

  if (link.rlb)
    fprintf (output, ",RLB");

  if (link.EQ)
    fprintf (output, ",EQ");

  fprintf (output, "\n");
}


/** @brief print the value of a span mode mask
 *  @deprecated span mode masked are no longer used
 *
 * @param mode the span mode mask
 * @param output the stream to print to
 */
void
print_span_mode (unsigned char mode, FILE * output)
{
  if (mode & SPAN_MODE_E1)
    {
      fprintf (output, "E1");
      if (mode & SPAN_MODE_CRCMF)
	fprintf (output, " (CRC4)");
    }
  else
    {
      fprintf (output, "T1");
      if (mode & SPAN_MODE_ESF)
	fprintf (output, ",ESF");
      else
	fprintf (output, ",SF");
    }


  if (mode & SPAN_MODE_AMI)
    fprintf (output, ",AMI");
  else if (mode & SPAN_MODE_E1)
    fprintf (output, ",HDB3");
  else
    fprintf (output, ",B8ZS");

  if (mode & SPAN_MODE_RBS)
    fprintf (output, ",RBS");

  if (mode & SPAN_MODE_RLB)
    fprintf (output, ",RLB");

  if (mode & SPAN_MODE_EQ)
    fprintf (output, ",EQ");

  fprintf (output, "\n");
}

/* Randomness and Key Generation Function */

/* We should really use the entire value returned by rand() for larger
 *  key generation. Multiple calls are bad and some platforms do not
 *  have much randomness in the lowest bytes.
 */

/** @brief get a random byte
 *  @return a random byte
 */
uint8_t
fblib_get_random_byte (void)
{
  static int initalized = 0;

  if (!initalized)
    {
      srand (time (NULL));
      initalized = 1;
    }

  return (rand () & 0xFF);
}

/** @brief Write a 32-byte random seed into a buffer 
 *  @param buffer the buffer to write to, must be at least 32 bytes long
 */
void
fblib_write_seed (uint8_t * buffer)
{
  int i;

  if (buffer == NULL)
    return;

  for (i = 0; i < 32; i++)
    buffer[i] = fblib_get_random_byte ();

  return;
}

/** @brief print a key data structure
 *  @param stream the stream to print to
 *  @param key the key to print
 * 
 *  @return the number of bytes printed 
 */
int
libfb_fprint_key (FILE * stream, KEY_ENTRY * key)
{
  int i;
  int len = 0;

  /* Hash Key */
  len += fprintf (stream, "\tHASH_KEY = 0x");

  for (i = 0; i < HASH_KEY_SZ; i++)
    len += fprintf (stream, "%02X", key->hash_key[i]);
  len += fprintf (stream, ";\n");

  /* Customer Key */
  len += fprintf (stream, "\tCUSTOMER_KEY = 0x");

  for (i = 0; i < CUSTOMER_KEY_SZ; i++)
    len += fprintf (stream, "%02X", key->customer_key[i]);
  len += fprintf (stream, ";\n");

  return len;
}

/** @brief Returns the feature set the device being configured supports
 *
 * @return the feature set 
 */
FEATURE
libfb_feature_set (DOOF_STATIC_INFO * dsi)
{
  uint16_t buildnum = dsi->build_num;

  if (IS_FEATURE_2_0 (buildnum))
    return FEATURE_2_0;
  else if (IS_FEATURE_PRE_2_0 (buildnum))
    return FEATURE_PRE_2_0;
  else
    {
      return FEATURE_MAX;
    }
}


/** @brief Selected PMON registers for T1 ESF */
const libfb_PMONRegister libfb_regs_T1ESF[] = {
  {"LCV", "Bipolar Violation/Code Violation", 0x08, 16, 2, NULL},
  {"FER", "Frame Alignment Bit Error", 0x02, 12, 2, NULL},
  {"CRCE", "CRC-6 Error", 0x00, 10, 2, NULL},
  {"OOF", "Out of ESF Synchronization", 0x05, 5, 1, NULL},
  {NULL, NULL, 0, 0, 0, NULL}
};


/** @brief Selected PMON registers for T1 SF */
const libfb_PMONRegister libfb_regs_T1SF[] = {
  {"LCV", "Bipolar Violation/Code Violation", 0x08, 16, 2, NULL},
  {"FER", "F Bit Error", 0x02, 12, 2, NULL},
  {"OOF", "Out of ESF Synchronization", 0x05, 5, 1, NULL},
  {NULL, NULL, 0, 0, 0, NULL}
};

/** @brief Selected PMON registers for E1 */
const libfb_PMONRegister libfb_regs_E1[] = {
  {"LCV", "Bipolar Violation/Code Violation", 0x8, 16, 2, NULL},
  {"FER", "FAS/NFAS Bit/Pattern Error", 0x02, 12, 2, NULL},
  {"CRCE", "CRC-4 Error", 0x00, 10, 2, NULL},
  {"FEBE", "Far End Block Error", 0x0C, 10, 2, NULL},
  {"OOF", "Out of Basic Frame Synchronization", 0x05, 5, 1, NULL},
  {NULL, NULL, 0, 0, 0, NULL}
};
