#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

size_t encode_varint(uint32_t value, uint8_t* buf) {
	assert(buf != NULL);
	uint8_t* cur = buf;
	while (value >= 0x80) {
		const uint8_t byte = (value & 0x7f) | 0x80;
		*cur = byte;
		value >>= 7;
		++cur;
	}
	*cur = value;
	++cur;
	return cur - buf;
}

uint32_t decode_varint(const uint8_t** bufp) {
	const uint8_t* cur = *bufp;
	uint8_t byte = *cur++;
	uint32_t value = byte & 0x7f;
	size_t shift = 7;
	while (byte >= 0x80) {
		byte = *cur++;
		value += (byte & 0x7f) << shift;
		shift += 7;
	}
	*bufp = cur;
	return value;
}

uint32_t generate_number() {
	const int r = rand();
	const int p = r % 100;
	if (p < 90) {
		return r % 128;
	}
	if (p < 95) {
		return r % 16384;
	}
	if (p < 99) {
		return r % 2097152;
	}
	return r % 268435455;
}

int main() {
	srand(time(NULL));
	FILE* uncompressed = fopen("uncompressed.dat", "wb");
	FILE* compressed = fopen("compressed.dat", "wb");
	uint32_t number = 0;
	uint8_t varint = 0;
	size_t byte_count = 0;
	size_t bytes[100];
	if (uncompressed != NULL && compressed != NULL) {
		for (int i = 0; i < 1000000; i++) {
			number = generate_number();
			byte_count = encode_varint(number, &varint);
			fwrite(&number, sizeof(uint32_t), 1, uncompressed);
			fwrite(&varint, sizeof(uint8_t), byte_count, compressed);
			if (i<100) bytes[i] = byte_count;
		}
	} else {
		printf("Ошибка открытия файла(ов)");
		return EXIT_FAILURE;
	}
	fclose(uncompressed);
	fclose(compressed);

	uncompressed = fopen("uncompressed.dat", "rb");
	compressed = fopen("compressed.dat", "rb");

	if (uncompressed != NULL && compressed != NULL) {
		for (int i = 0; i < 100; i++) {
			fread(&number, sizeof(uint32_t), 1, uncompressed);
			byte_count = encode_varint(number, &varint);
			fread(&varint, sizeof(uint8_t), bytes[i], compressed);
			const uint8_t* bufp = &varint;
			uint32_t decoded = decode_varint(&bufp);
			if (decoded != number)
				printf(
				    "Ошибка, %u(int) != %u(varint), ITERATION "
				    "%d\n",
				    number, decoded, i);
			else printf("%u == %u\n", number, decoded);
		}
	} else {
		printf("Ошибка открытия файла(ов)");
		return EXIT_FAILURE;
	}
	fclose(uncompressed);
	fclose(compressed);
	return 0;
}
