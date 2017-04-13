#include <stdio.h>

#define FFT_CAR_IMG_NAME	"D:\\FFT_CAR.bin"
#define ST_CAR_IMG_NAME		"D:\\ST_CAR.bin"
#define SPI_ROM_NAME		"D:\\SPI_MP.bin"
#define NEW_SPI_ROM_NAME	"D:\\SPI_MP_NEW.bin"
#define SPI_BLOCK_SIZE		0x10000

static int _load_file(char *_pPath, unsigned char **_ppBuf, long *_pReadsize)
{
	FILE *pF = NULL;
	int errno = 0;
	int retval = 0;
	long filesize = 0;


	if (!_pPath )
	{
		printf("file input path is unknown \n");
		retval = -1;
		goto return_1;
	}
	errno = fopen_s(&pF, _pPath, "rb");
	if (pF == NULL)
	{
		printf("failed to open file %s\n", _pPath);
		retval = -1;
		goto return_1;
	}
	fseek(pF, 0L, SEEK_END);
	filesize = ftell(pF);
	fseek(pF, 0L, SEEK_SET);

	*_ppBuf = malloc(sizeof(char)*filesize);
	if (*_ppBuf == NULL)
	{
		printf("failed to alloc buffer\n");
		retval = -1;
		goto return_1;
	}

	errno = fread(*_ppBuf, 1, filesize, pF);
	if (errno != filesize)
	{
		printf("failed to read file \n");
		retval = -1;
		goto return_1;
	}
	
	printf("\n ================== %s ==========================\n", _pPath);
	//printf("read size : %d\n", filesize);

	*_pReadsize = filesize;
return_1:
	if (pF != NULL)
		fclose(pF);

	return retval;
}

static int _write(char *_pPath, unsigned char *_pBuff, long _size)
{
	FILE *pF = NULL;
	int errno = 0;
	int retval = 0;
	long writtenSize = 0;

	errno = fopen_s(&pF, _pPath, "wb+");
	if (pF == NULL)
	{
		printf("failed to open file %s\n", _pPath);
		retval = -1;
	}
	writtenSize = fwrite(_pBuff, 1, _size, pF);
	printf("written size  %d\n", writtenSize);

	if(pF != NULL)
		fclose(pF);

	return retval;
}

int main(void)
{
	unsigned char *pBuffBaseSpi = NULL, *pBufFFT = NULL, *pBufST = NULL;
	int errno = 0;
	int retval = 0;
	long spiSize = 0, fftsize=0, fftSizeWithPadding=0, stsize = 0, stSizeWithPadding = 0, totalSize=0;
	int numofblock = 0;
	unsigned char *ptempFFTBuff = NULL, *ptempSTBuff = NULL, *ptempSpiBuff = NULL;

	if (_load_file(SPI_ROM_NAME, &pBuffBaseSpi, &spiSize) < 0)
	{
		retval = -1;
		goto return_1;
	}
	printf("SPI ROM size : %d\n", spiSize);
	if (_load_file(FFT_CAR_IMG_NAME, &pBufFFT, &fftsize) < 0)
	{
		retval = -1;
		goto return_1;
	}
	printf("fft size : %d\n", fftsize);

	if (_load_file(ST_CAR_IMG_NAME, &pBufST, &stsize) < 0)
	{
		retval = -1;
		goto return_1;
	}
	printf("st size : %d\n", stsize);

	numofblock = stsize / SPI_BLOCK_SIZE;
	if ((stsize % SPI_BLOCK_SIZE) != 0)
	{
		numofblock++;
	}
	stSizeWithPadding = numofblock*SPI_BLOCK_SIZE;
	ptempSTBuff = malloc(stSizeWithPadding);
	if (ptempSTBuff == NULL)
	{
		retval = -1;
		goto return_1;
	}
	memset(ptempSTBuff, 0xFF, stSizeWithPadding);
	memcpy(ptempSTBuff, pBufST, stsize);



	numofblock = 0;
	numofblock = fftsize / SPI_BLOCK_SIZE;
	if ((fftsize % SPI_BLOCK_SIZE) != 0)
	{
		numofblock++;
	}
	fftSizeWithPadding = numofblock*SPI_BLOCK_SIZE;

	printf("numofblock : %d, fftSizeWithPadding : %d \n", numofblock, fftSizeWithPadding);

	ptempFFTBuff = malloc(fftSizeWithPadding);
	if (ptempFFTBuff == NULL)
	{
		retval = -1;
		goto return_1;
	}
	
	memset(ptempFFTBuff, 0xFF, fftSizeWithPadding);
	memcpy(ptempFFTBuff, pBufFFT, fftsize);
	_write("D:\\FFT_TEST.bin", ptempFFTBuff, fftSizeWithPadding);
	memcpy(&pBuffBaseSpi[0x400000], ptempFFTBuff, fftSizeWithPadding);

	totalSize = 0x500000 + stSizeWithPadding;
	ptempSpiBuff = malloc(totalSize);
	if (ptempSpiBuff == NULL)
	{
		retval = -1;
		goto return_1;
	}
	memset(ptempSpiBuff, 0xFF, totalSize);
	memcpy(ptempSpiBuff, pBuffBaseSpi, spiSize);
	memcpy(&ptempSpiBuff[0x500000], ptempSTBuff, stSizeWithPadding);
	_write(NEW_SPI_ROM_NAME, ptempSpiBuff, totalSize);

return_1:
	if (pBuffBaseSpi != NULL)
		free(pBuffBaseSpi);
	if (pBufFFT != NULL)
		free(pBufFFT);
	if (ptempFFTBuff != NULL)
		free(ptempFFTBuff);
	if (ptempSTBuff != NULL)
		free(ptempSTBuff);
	if (pBufST != NULL)
		free(pBufST);

	system("pause");
	return retval;
}