  #include "rpi3arm.h"

//
//void gost_kuznec_set_key(gost_kuznec_ctx* ctx, const uint8_t* key)
//{
//	
//}
void gost_kuznec_set_key(gost_kuznec_ctx* ctx, const uint8_t* key)
{
	unsigned char test_key[2][16];

	for (int i = 0; i < 16; i++)
		test_key[0][i] = key[i];
	for (int i = 0; i < 16; i++)
		test_key[1][i] = key[16 + i];

	_GOST_Kuz_Expand_Key(test_key[0], test_key[1]);
}

void gost_kuznec_encrypt(const gost_kuznec_ctx* ctx, size_t length, uint8_t* dst, const uint8_t* src)
{

	while (length)
	{
		_GOST_Kuz_Encript(src, dst);
		src += KUZNYECHIK_BLOCK_SIZE;
		dst += KUZNYECHIK_BLOCK_SIZE;
		length -= KUZNYECHIK_BLOCK_SIZE;
	}
	
}

void gost_kuznec_decrypt(const gost_kuznec_ctx* ctx, size_t length, uint8_t* dst, const uint8_t* src)
{
	while (length)
	{
		_GOST_Kuz_Decript(src, dst);
		src += KUZNYECHIK_BLOCK_SIZE;
		dst += KUZNYECHIK_BLOCK_SIZE;
		length -= KUZNYECHIK_BLOCK_SIZE;
	}
}

