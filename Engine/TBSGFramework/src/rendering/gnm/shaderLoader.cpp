#ifdef PLATFORM_PS
#include "rendering/gnm/shaderLoader.h"
#include <allocators.h>

bool readFileContents(void *data, const size_t size, FILE *fp)
{
	if (!fp || !data)
		return false;

	uint8_t *address = static_cast<uint8_t*>(data);

	size_t bytesRead, totalBytesRead = 0;
	while (totalBytesRead < size)
	{
		bytesRead = fread(address, 1, size - totalBytesRead, fp);
		if (!bytesRead)
		{
			return false;
		}

		totalBytesRead += bytesRead;
	}

	return true;
}

bool acquireFileContents(void*& pointer, uint32_t& bytes, const ptl::string& filename)
{
	pointer = NULL;
	bytes = 0;

	bool retValue = false; 

	FILE *fp = fopen(filename.c_str(), "rb");
	if (fp    )
	{ 
		if (!fseek(fp, 0, SEEK_END)) 
		{
			bytes = ftell(fp);  
			if (!fseek(fp, 0, SEEK_SET))
			{
				pointer = malloc(bytes);
				if (pointer)
				{
					if (readFileContents(pointer, bytes, fp))
					{
						retValue = true;  
					}
					else
					{
						free(pointer); 
					}
				}
			}
		}
		 
		fclose(fp);
	}
	else ASSERT_MSG(false, "FAILED TO LOAD SHADER...");   

	if (!retValue)
	{
		pointer = NULL;
		bytes = 0;
	}

	return retValue;
}

#endif
