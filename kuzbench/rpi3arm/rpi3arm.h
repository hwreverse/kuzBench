
  #include <stdint.h>


#define BLOCK_SIZE 16				
#define DEBUG_MODE
typedef uint8_t vect[BLOCK_SIZE]; 

extern "C" void _GOST_Kuz_Expand_Key(const uint8_t *key_1, const uint8_t *key_2);
extern "C" void _GOST_Kuz_Encript(const uint8_t *blk, uint8_t *out_blk);
extern "C" void _GOST_Kuz_Decript(const uint8_t *blk, uint8_t *out_blk);
extern "C" vect _iter_key[10];			


// Nettle interface imitation. It is for using nettle's cbc algorithm with this asm realization.

#include "../nettle/nettle-types.h"

#ifdef __cplusplus
extern "C" {
#endif



#define KUZNYECHIK_KEY_SIZE 32
#define KUZNYECHIK_SUBKEYS_SIZE (16 * 10)
#define KUZNYECHIK_BLOCK_SIZE 16

	struct gost_kuznec_ctx
	{
		uint8_t key[KUZNYECHIK_SUBKEYS_SIZE];
		uint8_t dekey[KUZNYECHIK_SUBKEYS_SIZE];
	};

	void
		gost_kuznec_set_key(struct gost_kuznec_ctx *ctx, const uint8_t *key);


	void
		gost_kuznec_encrypt(const struct gost_kuznec_ctx *ctx,
			size_t length, uint8_t *dst,
			const uint8_t *src);
	void
		gost_kuznec_decrypt(const struct gost_kuznec_ctx *ctx,
			size_t length, uint8_t *dst,
			const uint8_t *src);

#ifdef __cplusplus
}
#endif

