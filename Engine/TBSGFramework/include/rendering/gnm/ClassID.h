#pragma once
#ifdef PLATFORM_PS
struct IClass
{
protected:
	static unsigned int regId()
	{
		static unsigned int lastId = -1;
		return ++lastId;
	}
};

template <typename T>
struct ClassID : IClass
{
	static unsigned int id()
	{
		static const unsigned int x = regId();
		return x;
	}
};
#endif