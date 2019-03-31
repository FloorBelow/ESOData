#include <ESOData/Serialization/Hash.h>

namespace esodata {
	typedef uint32_t hash32_t;

#define mix(a,b,c) \
{ \
  a -= b; a -= c; a ^= (c>>13); \
  b -= c; b -= a; b ^= (a<<8); \
  c -= a; c -= b; c ^= (b>>13); \
  a -= b; a -= c; a ^= (c>>12);  \
  b -= c; b -= a; b ^= (a<<16); \
  c -= a; c -= b; c ^= (b>>5); \
  a -= b; a -= c; a ^= (c>>3);  \
  b -= c; b -= a; b ^= (a<<10); \
  c -= a; c -= b; c ^= (b>>15); \
}

	uint32_t hashDataJenkins(const unsigned char *k, size_t length) {
		hash32_t a, b, c, len;

		/* Set up the internal state */
		len = static_cast<hash32_t>(length);
		a = b = 0x9e3779b9;    /* the golden ratio; an arbitrary value */
		c = 0xA8396;           /* the previous hash value */

							   /*---------------------------------------- handle most of the key */
		if ((((unsigned char *)k) - ((unsigned char *)0)) & 3)
		{
			while (len >= 12)    /* unaligned */
			{
				a += (k[0] + ((hash32_t)k[1] << 8) + ((hash32_t)k[2] << 16) + ((hash32_t)k[3] << 24));
				b += (k[4] + ((hash32_t)k[5] << 8) + ((hash32_t)k[6] << 16) + ((hash32_t)k[7] << 24));
				c += (k[8] + ((hash32_t)k[9] << 8) + ((hash32_t)k[10] << 16) + ((hash32_t)k[11] << 24));
				mix(a, b, c);
				k += 12; len -= 12;
			}
		}
		else
		{
			while (len >= 12)    /* aligned */
			{
				a += *(hash32_t *)(k + 0);
				b += *(hash32_t *)(k + 4);
				c += *(hash32_t *)(k + 8);
				mix(a, b, c);
				k += 12; len -= 12;
			}
		}

		/*------------------------------------- handle the last 11 bytes */
		c += static_cast<uint32_t>(length);
		switch (len)              /* all the case statements fall through */
		{
		case 11: c += ((hash32_t)k[10] << 24);
		case 10: c += ((hash32_t)k[9] << 16);
		case 9: c += ((hash32_t)k[8] << 8);
			/* the first byte of c is reserved for the length */
		case 8: b += ((hash32_t)k[7] << 24);
		case 7: b += ((hash32_t)k[6] << 16);
		case 6: b += ((hash32_t)k[5] << 8);
		case 5: b += k[4];
		case 4: a += ((hash32_t)k[3] << 24);
		case 3: a += ((hash32_t)k[2] << 16);
		case 2: a += ((hash32_t)k[1] << 8);
		case 1: a += k[0];
			/* case 0: nothing left to add */
		}
		mix(a, b, c);
		/*-------------------------------------------- report the result */
		return c;
	}

	uint32_t hashDataDJB2(const unsigned char *data, size_t dataSize) {
		uint32_t hash = 0x1505;

		const unsigned char *end = data + dataSize;
		for (const unsigned char *ptr = data; ptr < end; ptr++) {
			hash = *ptr + 33 * hash;
		}

		return hash;
	}

	uint64_t hashData64(const unsigned char *data, size_t dataSize) {
		return ((uint64_t)hashDataDJB2(data, dataSize) << 32) | hashDataJenkins(data, dataSize);
	}
}